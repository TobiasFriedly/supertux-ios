//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "audio/ios_audio_session.hpp"

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <UIKit/UIKit.h>

#include <atomic>

#include "util/log.hpp"

namespace {

SuperTuxIOSRemoteCommandCallback s_next_track_callback = nullptr;
SuperTuxIOSRemoteCommandCallback s_previous_track_callback = nullptr;
SuperTuxIOSRemoteCommandCallback s_play_callback = nullptr;
SuperTuxIOSRemoteCommandCallback s_pause_callback = nullptr;
SuperTuxIOSSeekCommandCallback s_seek_callback = nullptr;
bool s_remote_commands_registered = false;
bool s_audio_notifications_registered = false;
std::atomic_bool s_audio_interruption_ended(false);

MPRemoteCommandHandlerStatus
run_remote_command(SuperTuxIOSRemoteCommandCallback callback)
{
  if (callback && callback())
    return MPRemoteCommandHandlerStatusSuccess;

  return MPRemoteCommandHandlerStatusCommandFailed;
}

MPRemoteCommandHandlerStatus
run_seek_command(SuperTuxIOSSeekCommandCallback callback, double position)
{
  if (callback && callback(position))
    return MPRemoteCommandHandlerStatusSuccess;

  return MPRemoteCommandHandlerStatusCommandFailed;
}

NSString*
string_from_utf8(const char* text)
{
  if (!text)
    return @"";

  NSString* string = [NSString stringWithUTF8String:text];
  return string ? string : @"";
}

MPMediaItemArtwork*
artwork_from_path(const char* artwork_path)
{
  NSString* path = nil;
  if (artwork_path && artwork_path[0] != '\0')
    path = string_from_utf8(artwork_path);
  else
    path = [[NSBundle mainBundle] pathForResource:@"supertux-ios-artwork" ofType:@"png"];

  if (!path)
    return nil;

  UIImage* image = [UIImage imageWithContentsOfFile:path];
  if (!image)
    return nil;

  if (@available(iOS 10.0, *))
  {
    return [[MPMediaItemArtwork alloc] initWithBoundsSize:image.size requestHandler:^UIImage* (CGSize size) {
      (void)size;
      return image;
    }];
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  return [[MPMediaItemArtwork alloc] initWithImage:image];
#pragma clang diagnostic pop
}

void
register_audio_notifications()
{
  if (s_audio_notifications_registered)
    return;

  NSNotificationCenter* notification_center = [NSNotificationCenter defaultCenter];
  [notification_center addObserverForName:AVAudioSessionInterruptionNotification
                                   object:[AVAudioSession sharedInstance]
                                    queue:nil
                               usingBlock:^(NSNotification* notification) {
    NSNumber* type_value = notification.userInfo[AVAudioSessionInterruptionTypeKey];
    if (!type_value)
      return;

    const AVAudioSessionInterruptionType type =
      static_cast<AVAudioSessionInterruptionType>([type_value unsignedIntegerValue]);
    if (type == AVAudioSessionInterruptionTypeEnded)
    {
      NSError* error = nil;
      [[AVAudioSession sharedInstance] setActive:YES error:&error];
      if (error)
      {
        log_warning << "Failed to reactivate iOS audio session after interruption: "
                    << [[error localizedDescription] UTF8String] << std::endl;
      }
      s_audio_interruption_ended.store(true);
    }
  }];

  [notification_center addObserverForName:AVAudioSessionMediaServicesWereResetNotification
                                   object:[AVAudioSession sharedInstance]
                                    queue:nil
                               usingBlock:^(NSNotification* notification) {
    (void)notification;
    s_audio_interruption_ended.store(true);
  }];

  s_audio_notifications_registered = true;
}

}

void
supertux_ios_configure_audio_session()
{
  @autoreleasepool
  {
    AVAudioSession* session = [AVAudioSession sharedInstance];
    NSError* error = nil;

    if (![session setCategory:AVAudioSessionCategoryPlayback error:&error])
    {
      log_warning << "Failed to set iOS audio session category: "
                  << [[error localizedDescription] UTF8String] << std::endl;
      return;
    }

    register_audio_notifications();

    error = nil;
    if (![session setActive:YES error:&error])
    {
      log_warning << "Failed to activate iOS audio session: "
                  << [[error localizedDescription] UTF8String] << std::endl;
    }
  }
}

void
supertux_ios_set_remote_command_callbacks(SuperTuxIOSRemoteCommandCallback next_track,
                                          SuperTuxIOSRemoteCommandCallback previous_track,
                                          SuperTuxIOSRemoteCommandCallback play,
                                          SuperTuxIOSRemoteCommandCallback pause,
                                          SuperTuxIOSSeekCommandCallback seek)
{
  @autoreleasepool
  {
    s_next_track_callback = next_track;
    s_previous_track_callback = previous_track;
    s_play_callback = play;
    s_pause_callback = pause;
    s_seek_callback = seek;

    MPRemoteCommandCenter* command_center = [MPRemoteCommandCenter sharedCommandCenter];
    command_center.nextTrackCommand.enabled = YES;
    command_center.previousTrackCommand.enabled = YES;
    command_center.playCommand.enabled = YES;
    command_center.pauseCommand.enabled = YES;
    command_center.changePlaybackPositionCommand.enabled = YES;

    if (s_remote_commands_registered)
      return;

    [command_center.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
      (void)event;
      return run_remote_command(s_next_track_callback);
    }];
    [command_center.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
      (void)event;
      return run_remote_command(s_previous_track_callback);
    }];
    [command_center.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
      (void)event;
      return run_remote_command(s_play_callback);
    }];
    [command_center.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
      (void)event;
      return run_remote_command(s_pause_callback);
    }];
    [command_center.changePlaybackPositionCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
      MPChangePlaybackPositionCommandEvent* position_event = (MPChangePlaybackPositionCommandEvent*)event;
      return run_seek_command(s_seek_callback, position_event.positionTime);
    }];

    s_remote_commands_registered = true;
  }
}

void
supertux_ios_update_now_playing(const char* title, const char* artist,
                                double elapsed, double duration,
                                bool playing, const char* artwork_path)
{
  @autoreleasepool
  {
    NSMutableDictionary* now_playing = [NSMutableDictionary dictionary];
    now_playing[MPMediaItemPropertyTitle] = string_from_utf8(title);
    now_playing[MPMediaItemPropertyArtist] = string_from_utf8(artist);
    now_playing[MPNowPlayingInfoPropertyPlaybackRate] = playing ? @1.0 : @0.0;
    now_playing[MPNowPlayingInfoPropertyElapsedPlaybackTime] = @(elapsed);
    if (duration > 0.0)
      now_playing[MPMediaItemPropertyPlaybackDuration] = @(duration);

    MPMediaItemArtwork* artwork = artwork_from_path(artwork_path);
    if (artwork)
      now_playing[MPMediaItemPropertyArtwork] = artwork;

    [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = now_playing;
  }
}

void
supertux_ios_clear_now_playing()
{
  @autoreleasepool
  {
    [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
  }
}

bool
supertux_ios_audio_interruption_ended()
{
  return s_audio_interruption_ended.exchange(false);
}
