//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#pragma once

#include <SDL.h>
#include <map>
#include <bitset>

#include "config.h"

#include "controller.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/surface_ptr.hpp"

class Controller;
class DrawingContext;

class MobileController final
{
public:
  MobileController();
  void draw(DrawingContext& context, bool movement_controls_enabled = true, bool editor_pause_only = false);
  void apply(Controller& controller) const;
  void update(bool movement_controls_enabled = true, bool editor_pause_only = false);
  void reset();

  /** returns true if the finger event was inside the screen button area */
  bool process_finger_down_event(const SDL_TouchFingerEvent& event, bool movement_controls_enabled = true, bool editor_pause_only = false);
  /** returns true if the finger event was inside the screen button area */
  bool process_finger_up_event(const SDL_TouchFingerEvent& event, bool movement_controls_enabled = true, bool editor_pause_only = false);
  /** returns true if the finger event was inside the screen button area */
  bool process_finger_motion_event(const SDL_TouchFingerEvent& event, bool movement_controls_enabled = true, bool editor_pause_only = false);

  void buzz();

private:
  void update_layout(float width, float height);
  bool contains_control_at_pos(const Vector& pos, bool movement_controls_enabled = true, bool editor_pause_only = false) const;
  bool contains_movement_control_at_pos(const Vector& pos, bool movement_controls_enabled = true) const;
  void activate_widget_at_pos(float x, float y, bool movement_controls_enabled = true, bool editor_pause_only = false);
  void activate_floating_joystick_at_pos(const Vector& pos);
  Vector finger_event_to_logical(const SDL_TouchFingerEvent& event) const;

private:
  std::bitset<(size_t)Control::CONTROLCOUNT> m_input, m_input_last;

  std::map<SDL_FingerID, Vector> m_fingers;

  Rectf m_rect_directions, m_rect_jump, m_rect_action, m_rect_cheats,
        m_rect_debug, m_rect_escape, m_rect_item, m_rect_joystick_zone;
  Rectf m_draw_directions, m_draw_jump, m_draw_action, m_draw_cheats,
        m_draw_debug, m_draw_escape;
  bool m_joystick_active, m_mouse_joystick_active;
  SDL_FingerID m_joystick_finger;
  Vector m_joystick_center, m_joystick_pos;
  const SurfacePtr m_tex_dirs, m_tex_btn, m_tex_btn_press, m_tex_pause,
                   m_tex_up, m_tex_dwn, m_tex_lft, m_tex_rgt,
                   m_tex_jump, m_tex_action, m_tex_cheats, m_tex_debug;

  int m_screen_width, m_screen_height;
  float m_mobile_controls_scale;

  // We need the timer to be away from the game loop to stop vibration
  SDL_TimerID m_haptic_timer;
  std::unique_ptr<SDL_Haptic, decltype(&SDL_HapticClose)> m_haptic;

private:
  MobileController(const MobileController&) = delete;
  MobileController& operator=(const MobileController&) = delete;
};
