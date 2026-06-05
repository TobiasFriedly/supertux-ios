//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "supertux/menu/music_menu.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <vector>

#include <fmt/format.h>

#include "gui/menu_item.hpp"
#include "supertux/music_player.hpp"
#include "util/gettext.hpp"

namespace {

constexpr int MNID_STOP = -2;
constexpr int MNID_PREVIOUS_PAGE = -3;
constexpr int MNID_NEXT_PAGE = -4;
constexpr int MNID_PREVIOUS_TRACK = -5;
constexpr int MNID_NEXT_TRACK = -6;
constexpr int MNID_TOGGLE_FAVORITE_CURRENT = -7;
constexpr int MNID_TOGGLE_FAVORITES_ONLY = -8;

constexpr std::size_t TRACKS_PER_PAGE = 10;

}

MusicMenu::MusicMenu() :
  m_page(0)
{
  refresh();
}

void
MusicMenu::refresh()
{
  clear();
  MusicPlayer& player = MusicPlayer::instance();
  const auto& tracks = player.get_tracks();
  const std::vector<std::size_t> playlist = player.get_playlist_indices();

  const std::size_t page_count = playlist.empty() ? 1 :
    (playlist.size() + TRACKS_PER_PAGE - 1) / TRACKS_PER_PAGE;
  if (m_page >= page_count)
    m_page = page_count - 1;

  add_label(_("Music Player"));
  add_hl();

  const std::optional<std::size_t> current_track = player.get_current_track_index();
  if (current_track.has_value())
  {
    add_label(_("Now Playing"));
    add_inactive(tracks[*current_track].label, true);
    add_entry(MNID_PREVIOUS_TRACK, _("Previous Track"));
    add_entry(MNID_NEXT_TRACK, _("Next Track"));
    add_entry(MNID_TOGGLE_FAVORITE_CURRENT,
              player.is_favorite(*current_track) ? _("Remove from Favorites") : _("Add to Favorites"));
    add_entry(MNID_STOP, _("Stop Music"));
  }
  else
  {
    add_inactive(_("No track playing"), true);
  }

  add_hl();
  add_entry(MNID_TOGGLE_FAVORITES_ONLY,
            player.favorites_only() ? _("Show All Tracks") :
            fmt::format(fmt::runtime(_("Favorites Only ({})")), player.get_favorite_count()));

  if (tracks.empty())
  {
    add_inactive(_("No music tracks found"));
  }
  else if (playlist.empty())
  {
    add_inactive(_("No favorite tracks yet"));
  }
  else
  {
    add_inactive(fmt::format(fmt::runtime(_("Page {}/{}")), m_page + 1, page_count), true);

    const std::size_t start = m_page * TRACKS_PER_PAGE;
    const std::size_t end = std::min(start + TRACKS_PER_PAGE, playlist.size());

    for (std::size_t i = start; i < end; ++i)
    {
      const std::size_t track_index = playlist[i];
      const MusicPlayer::Track& track = tracks[track_index];
      const std::string favorite = player.is_favorite(track_index) ? "* " : "";
      const std::string label = (current_track.has_value() && *current_track == track_index) ?
        fmt::format("> {}{}", favorite, track.label) : fmt::format("{}{}", favorite, track.label);
      add_entry(static_cast<int>(track_index), label);
    }

    add_hl();
    if (m_page > 0)
      add_entry(MNID_PREVIOUS_PAGE, _("Previous Page"));
    else
      add_inactive(_("Previous Page"));

    if (m_page + 1 < page_count)
      add_entry(MNID_NEXT_PAGE, _("Next Page"));
    else
      add_inactive(_("Next Page"));
  }

  add_hl();
  add_back(_("Back"));
}

void
MusicMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == MNID_STOP)
  {
    MusicPlayer::instance().stop();
    refresh();
    return;
  }

  if (item.get_id() == MNID_PREVIOUS_PAGE)
  {
    if (m_page > 0)
      --m_page;
    refresh();
    return;
  }

  if (item.get_id() == MNID_NEXT_PAGE)
  {
    if (m_page + 1 < MusicPlayer::instance().get_page_count(TRACKS_PER_PAGE))
      ++m_page;
    refresh();
    return;
  }

  if (item.get_id() == MNID_PREVIOUS_TRACK || item.get_id() == MNID_NEXT_TRACK)
  {
    if (item.get_id() == MNID_PREVIOUS_TRACK)
      MusicPlayer::instance().previous_track();
    else
      MusicPlayer::instance().next_track();

    refresh();
    return;
  }

  if (item.get_id() == MNID_TOGGLE_FAVORITE_CURRENT)
  {
    const auto current_track = MusicPlayer::instance().get_current_track_index();
    if (current_track.has_value())
      MusicPlayer::instance().toggle_favorite(*current_track);
    refresh();
    return;
  }

  if (item.get_id() == MNID_TOGGLE_FAVORITES_ONLY)
  {
    MusicPlayer& player = MusicPlayer::instance();
    player.set_favorites_only(!player.favorites_only());
    m_page = 0;
    refresh();
    return;
  }

  if (item.get_id() < 0)
    return;

  const size_t index = static_cast<size_t>(item.get_id());
  if (index >= MusicPlayer::instance().get_tracks().size())
    return;

  if (MusicPlayer::instance().play_track(index))
  {
    const auto playlist = MusicPlayer::instance().get_playlist_indices();
    const auto it = std::find(playlist.begin(), playlist.end(), index);
    m_page = it == playlist.end() ? 0 :
      static_cast<std::size_t>(std::distance(playlist.begin(), it)) / TRACKS_PER_PAGE;
  }
  refresh();
}
