//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once

using SuperTuxIOSRemoteCommandCallback = bool (*)();
using SuperTuxIOSSeekCommandCallback = bool (*)(double);

void supertux_ios_configure_audio_session();
void supertux_ios_set_remote_command_callbacks(SuperTuxIOSRemoteCommandCallback next_track,
                                               SuperTuxIOSRemoteCommandCallback previous_track,
                                               SuperTuxIOSRemoteCommandCallback play,
                                               SuperTuxIOSRemoteCommandCallback pause,
                                               SuperTuxIOSSeekCommandCallback seek);
void supertux_ios_update_now_playing(const char* title, const char* artist,
                                     double elapsed, double duration,
                                     bool playing, const char* artwork_path);
void supertux_ios_clear_now_playing();
bool supertux_ios_audio_interruption_ended();
