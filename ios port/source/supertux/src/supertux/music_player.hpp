//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

class MusicPlayer final
{
public:
  struct Track
  {
    std::string path;
    std::string label;
  };

public:
  static MusicPlayer& instance();

  const std::vector<Track>& get_tracks();
  std::size_t get_page_count(std::size_t tracks_per_page);
  std::optional<std::size_t> get_current_track_index();
  std::vector<std::size_t> get_playlist_indices() const;
  bool favorites_only() const;
  void set_favorites_only(bool enabled);
  bool is_favorite(std::size_t index) const;
  void toggle_favorite(std::size_t index);
  std::size_t get_favorite_count() const;

  bool play_track(std::size_t index);
  bool next_track();
  bool previous_track();
  bool resume();
  bool pause();
  bool seek(double seconds);
  void stop();
  void update();

  void refresh_now_playing(bool playing);

private:
  MusicPlayer();

  void load_tracks();
  void register_ios_remote_commands();

private:
  std::vector<Track> m_tracks;
  bool m_tracks_loaded;
  bool m_player_active;
  bool m_paused;
  bool m_favorites_only;

private:
  MusicPlayer(const MusicPlayer&) = delete;
  MusicPlayer& operator=(const MusicPlayer&) = delete;
};
