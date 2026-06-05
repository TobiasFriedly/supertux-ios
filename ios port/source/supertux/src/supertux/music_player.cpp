//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "supertux/music_player.hpp"

#include <algorithm>
#include <iterator>

#include "audio/sound_manager.hpp"
#ifdef SUPERTUX_IOS
#include "audio/ios_audio_session.hpp"
#endif
#include "physfs/util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/file_system.hpp"
#include "util/string_util.hpp"

namespace {

std::string
make_track_label(const std::string& path)
{
  std::string label = FileSystem::strip_extension(FileSystem::basename(path));
  label = StringUtil::replace_all(label, "_", " ");
  label = StringUtil::replace_all(label, "-", " ");

  if (!path.empty())
  {
    const std::string group = FileSystem::basename(FileSystem::dirname(path));
    if (!group.empty() && group != "music")
      label = group + " / " + label;
  }

  return label;
}

#ifdef SUPERTUX_IOS
bool
ios_remote_next_track()
{
  return MusicPlayer::instance().next_track();
}

bool
ios_remote_previous_track()
{
  return MusicPlayer::instance().previous_track();
}

bool
ios_remote_play()
{
  return MusicPlayer::instance().resume();
}

bool
ios_remote_pause()
{
  return MusicPlayer::instance().pause();
}

bool
ios_remote_seek(double seconds)
{
  return MusicPlayer::instance().seek(seconds);
}
#endif

}

MusicPlayer&
MusicPlayer::instance()
{
  static MusicPlayer music_player;
  return music_player;
}

MusicPlayer::MusicPlayer() :
  m_tracks(),
  m_tracks_loaded(false),
  m_player_active(false),
  m_paused(false),
  m_favorites_only(false)
{
  register_ios_remote_commands();
}

const std::vector<MusicPlayer::Track>&
MusicPlayer::get_tracks()
{
  load_tracks();
  return m_tracks;
}

std::size_t
MusicPlayer::get_page_count(std::size_t tracks_per_page)
{
  load_tracks();

  if (m_tracks.empty() || tracks_per_page == 0)
    return 1;

  return (m_tracks.size() + tracks_per_page - 1) / tracks_per_page;
}

std::optional<std::size_t>
MusicPlayer::get_current_track_index()
{
  load_tracks();

  const std::string& current_music = SoundManager::current()->get_current_music();
  for (size_t i = 0; i < m_tracks.size(); ++i)
  {
    if (m_tracks[i].path == current_music)
      return i;
  }

  return std::nullopt;
}

std::vector<std::size_t>
MusicPlayer::get_playlist_indices() const
{
  const_cast<MusicPlayer*>(this)->load_tracks();

  std::vector<std::size_t> indices;
  indices.reserve(m_tracks.size());

  for (std::size_t i = 0; i < m_tracks.size(); ++i)
  {
    if (!m_favorites_only || is_favorite(i))
      indices.push_back(i);
  }

  return indices;
}

bool
MusicPlayer::favorites_only() const
{
  return m_favorites_only;
}

void
MusicPlayer::set_favorites_only(bool enabled)
{
  m_favorites_only = enabled;
}

bool
MusicPlayer::is_favorite(std::size_t index) const
{
  const_cast<MusicPlayer*>(this)->load_tracks();

  if (index >= m_tracks.size() || !g_config)
    return false;

  const auto& favorites = g_config->music_favorites;
  return std::find(favorites.begin(), favorites.end(), m_tracks[index].path) != favorites.end();
}

void
MusicPlayer::toggle_favorite(std::size_t index)
{
  load_tracks();

  if (index >= m_tracks.size() || !g_config)
    return;

  auto& favorites = g_config->music_favorites;
  const auto it = std::find(favorites.begin(), favorites.end(), m_tracks[index].path);
  if (it == favorites.end())
    favorites.push_back(m_tracks[index].path);
  else
    favorites.erase(it);

  g_config->save();
}

std::size_t
MusicPlayer::get_favorite_count() const
{
  const_cast<MusicPlayer*>(this)->load_tracks();

  std::size_t count = 0;
  for (std::size_t i = 0; i < m_tracks.size(); ++i)
  {
    if (is_favorite(i))
      ++count;
  }
  return count;
}

bool
MusicPlayer::play_track(std::size_t index)
{
  load_tracks();

  if (index >= m_tracks.size())
    return false;

  SoundManager::current()->enable_music(true);
  SoundManager::current()->play_music(m_tracks[index].path, 0.5f, false);
  m_player_active = true;
  m_paused = false;
  refresh_now_playing(true);
  return true;
}

bool
MusicPlayer::next_track()
{
  load_tracks();

  if (m_tracks.empty())
    return false;

  const auto playlist = get_playlist_indices();
  if (playlist.empty())
    return false;

  const std::size_t current = get_current_track_index().value_or(playlist.front());
  const auto it = std::find(playlist.begin(), playlist.end(), current);
  if (it == playlist.end())
    return play_track(playlist.front());

  const auto next = std::next(it);
  return play_track(next == playlist.end() ? playlist.front() : *next);
}

bool
MusicPlayer::previous_track()
{
  load_tracks();

  if (m_tracks.empty())
    return false;

  const auto playlist = get_playlist_indices();
  if (playlist.empty())
    return false;

  const std::size_t current = get_current_track_index().value_or(playlist.front());
  const auto it = std::find(playlist.begin(), playlist.end(), current);
  if (it == playlist.end() || it == playlist.begin())
    return play_track(playlist.back());

  return play_track(*std::prev(it));
}

bool
MusicPlayer::resume()
{
  load_tracks();

  if (m_tracks.empty())
    return false;

  const auto current_track = get_current_track_index();
  if (!current_track.has_value())
    return play_track(0);

  SoundManager::current()->resume_music(0.2f);
  m_player_active = true;
  m_paused = false;
  refresh_now_playing(true);
  return true;
}

bool
MusicPlayer::pause()
{
  if (!get_current_track_index().has_value())
    return false;

  SoundManager::current()->pause_music(0.2f);
  m_paused = true;
  refresh_now_playing(false);
  return true;
}

bool
MusicPlayer::seek(double seconds)
{
  if (!get_current_track_index().has_value())
    return false;

  if (!SoundManager::current()->seek_music(seconds))
    return false;

  refresh_now_playing(!m_paused);
  return true;
}

void
MusicPlayer::stop()
{
  SoundManager::current()->stop_music();
  m_player_active = false;
  m_paused = false;

#ifdef SUPERTUX_IOS
  supertux_ios_clear_now_playing();
#endif
}

void
MusicPlayer::update()
{
  if (!m_player_active || m_paused)
    return;

  if (SoundManager::current()->is_music_finished())
  {
    next_track();
  }
}

void
MusicPlayer::refresh_now_playing(bool playing)
{
#ifdef SUPERTUX_IOS
  const auto current_track = get_current_track_index();
  if (current_track.has_value())
  {
    supertux_ios_update_now_playing(m_tracks[*current_track].label.c_str(), "SuperTux",
                                    SoundManager::current()->get_music_position(),
                                    SoundManager::current()->get_music_duration(),
                                    playing, nullptr);
  }
  else
  {
    supertux_ios_clear_now_playing();
  }
#else
  (void)playing;
#endif
}

void
MusicPlayer::load_tracks()
{
  if (m_tracks_loaded)
    return;

  m_tracks.clear();

  physfsutil::enumerate_files_recurse("/music", [this](const std::string& path) {
    if (StringUtil::has_suffix(path, ".music"))
      m_tracks.push_back({path, make_track_label(path)});
    return false;
  });

  std::sort(m_tracks.begin(), m_tracks.end(), [](const Track& lhs, const Track& rhs) {
    return lhs.label < rhs.label;
  });

  m_tracks_loaded = true;
}

void
MusicPlayer::register_ios_remote_commands()
{
#ifdef SUPERTUX_IOS
  supertux_ios_set_remote_command_callbacks(
    &ios_remote_next_track,
    &ios_remote_previous_track,
    &ios_remote_play,
    &ios_remote_pause,
    &ios_remote_seek);
#endif
}
