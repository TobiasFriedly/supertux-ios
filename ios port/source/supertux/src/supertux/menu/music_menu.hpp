//  SuperTux
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once

#include "gui/menu.hpp"

#include <cstddef>

class MusicMenu final : public Menu
{
public:
  MusicMenu();

  void refresh() override;
  void menu_action(MenuItem& item) override;

private:
  std::size_t m_page;

private:
  MusicMenu(const MusicMenu&) = delete;
  MusicMenu& operator=(const MusicMenu&) = delete;
};
