//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "editor/layer_icon.hpp"

#include <limits>

#include "math/rectf.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

LayerIcon::LayerIcon(LayerObject* layer) :
  ObjectIcon("", layer->get_icon_path()),
  m_layer(layer),
  m_layer_tilemap(dynamic_cast<TileMap*>(layer)),
  m_selection(m_layer_tilemap ?
                Surface::from_file("images/engine/editor/selection.png") : nullptr)
{
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos)
{
  draw(context, pos, 32.f);
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos, float size)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos, size);
  int l = get_zpos();
  if (l != std::numeric_limits<int>::min())
  {
    context.color().draw_text(Resources::small_font, std::to_string(l),
                                pos + Vector(size / 2.f, size / 2.f),
                                ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::default_color);
    if (m_layer_tilemap && m_layer_tilemap->m_editor_active)
      context.color().draw_surface_scaled(m_selection, Rectf(pos, pos + Vector(size, size)), LAYER_GUI - 1);
  }
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos, int pixels_shown)
{
  draw(context, pos, pixels_shown, 32.f);
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos, int pixels_shown, float size)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos, pixels_shown, size);
  int l = get_zpos();
  if (l != std::numeric_limits<int>::min())
  {
    // Don't draw the text if the icon is not 100% visible
    if (m_layer_tilemap && m_layer_tilemap->m_editor_active)
      context.color().draw_surface_scaled(m_selection, Rectf(pos, pos + Vector(size, size)), LAYER_GUI - 1);
  }
}

int
LayerIcon::get_zpos() const
{
  return is_valid() ? m_layer->get_layer() : std::numeric_limits<int>::min();
}

bool
LayerIcon::is_valid() const
{
  return m_layer && m_layer->is_valid();
}
