//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "audio/sound_file.hpp"
#include "audio/sound_manager.hpp"
#include "audio/stream_sound_source.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

#include <algorithm>
#include <iterator>

StreamSoundSource::StreamSoundSource() :
  m_file(),
  m_buffers(),
  m_buffer_durations(),
  m_fade_state(NoFading),
  m_fade_start_time(),
  m_fade_time(),
  m_looping(false),
  m_reached_end(false),
  m_finished(false),
  m_playback_base_seconds(0.0)
{
  alGenBuffers(STREAMFRAGMENTS, m_buffers);
  try
  {
    SoundManager::check_al_error("Couldn't allocate audio buffers: ");
  }
  catch(std::exception& e)
  {
    log_warning << e.what() << std::endl;
  }
  //add me to update list
  SoundManager::current()->register_for_update( this );
}

StreamSoundSource::~StreamSoundSource()
{
  //don't update me any longer
  SoundManager::current()->remove_from_update( this );
  m_file.reset();
  stop();
  alDeleteBuffers(STREAMFRAGMENTS, m_buffers);
  try
  {
    SoundManager::check_al_error("Couldn't delete audio buffers: ");
  }
  catch(std::exception& e)
  {
    // Am I bovvered?
    log_warning << e.what() << std::endl;
  }
}

void
StreamSoundSource::set_sound_file(std::unique_ptr<SoundFile> newfile)
{
  m_file = std::move(newfile);
  std::fill(std::begin(m_buffer_durations), std::end(m_buffer_durations), 0.0);
  m_reached_end = false;
  m_finished = false;
  m_playback_base_seconds = 0.0;

  ALint queued;
  alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
  for (size_t i = 0; i < STREAMFRAGMENTS - queued; ++i) {
    if (fillBufferAndQueue(m_buffers[i]) == false)
      break;
  }
}

void
StreamSoundSource::set_looping(bool looping_)
{
  m_looping = looping_;
  if (m_looping)
    m_finished = false;
}

void
StreamSoundSource::resume()
{
  if (m_finished)
    seek_time(0.0);

  OpenALSoundSource::resume();
  set_gain(1.0);
  m_fade_state = NoFading;
}

void
StreamSoundSource::update()
{
  ALint processed = 0;
  alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
  for (ALint i = 0; i < processed; ++i) {
    ALuint buffer;
    alSourceUnqueueBuffers(m_source, 1, &buffer);
    m_playback_base_seconds += get_buffer_duration(buffer);
    record_buffer_duration(buffer, 0.0);
    try
    {
      SoundManager::check_al_error("Couldn't unqueue audio buffer: ");
    }
    catch(std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }

    if (fillBufferAndQueue(buffer) == false)
      break;
  }

  if (!playing() && !paused()) {
    if (!m_looping)
    {
      if (m_reached_end)
        m_finished = true;
      return;
    }

    if (processed == 0)
      return;

    // we might have to restart the source if we had a buffer underrun
    log_info << "Restarting audio source because of buffer underrun" << std::endl;
    play();
  }

  if (m_fade_state == FadingOn || m_fade_state == FadingResume) {
    float time = g_real_time - m_fade_start_time;
    if (time >= m_fade_time) {
      set_gain(1.0);
      m_fade_state = NoFading;
    } else {
      set_gain(time / m_fade_time);
    }
  } else if (m_fade_state == FadingOff || m_fade_state == FadingPause) {
    float time = g_real_time - m_fade_start_time;
    if (time >= m_fade_time) {
      if (m_fade_state == FadingOff)
        stop();
      else
        pause();
      m_fade_state = NoFading;
    } else {
      set_gain( (m_fade_time - time) / m_fade_time);
    }
  }
}

void
StreamSoundSource::set_fading(FadeState state, float fade_time_)
{
  m_fade_state = state;
  m_fade_time = fade_time_;
  m_fade_start_time = g_real_time;
}

bool
StreamSoundSource::seek_time(double seconds)
{
  if (!m_file)
    return false;

  const bool was_playing = playing();
  const bool was_paused = paused();

  stop();
  clear_queued_buffers();

  if (!m_file->seek_time(seconds))
    return false;

  m_playback_base_seconds = std::max(0.0, seconds);
  m_reached_end = false;
  m_finished = false;
  std::fill(std::begin(m_buffer_durations), std::end(m_buffer_durations), 0.0);

  for (size_t i = 0; i < STREAMFRAGMENTS; ++i)
  {
    if (fillBufferAndQueue(m_buffers[i]) == false)
      break;
  }

  if (was_playing)
  {
    play();
  }
  else if (was_paused)
  {
    play();
    pause();
  }

  return true;
}

double
StreamSoundSource::get_position() const
{
  if (!m_file)
    return 0.0;

  ALfloat offset = 0.f;
  alGetSourcef(m_source, AL_SEC_OFFSET, &offset);
  const double position = m_playback_base_seconds + static_cast<double>(offset);
  const double duration = get_duration();
  return duration > 0.0 ? std::min(position, duration) : position;
}

double
StreamSoundSource::get_duration() const
{
  return m_file ? m_file->get_duration() : 0.0;
}

bool
StreamSoundSource::finished() const
{
  return m_finished;
}

void
StreamSoundSource::clear_queued_buffers()
{
  ALint queued = 0;
  alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
  while (queued > 0)
  {
    ALuint buffer = 0;
    alSourceUnqueueBuffers(m_source, 1, &buffer);
    record_buffer_duration(buffer, 0.0);
    --queued;
  }
}

void
StreamSoundSource::record_buffer_duration(ALuint buffer, double duration)
{
  for (size_t i = 0; i < STREAMFRAGMENTS; ++i)
  {
    if (m_buffers[i] == buffer)
    {
      m_buffer_durations[i] = duration;
      return;
    }
  }
}

double
StreamSoundSource::get_buffer_duration(ALuint buffer) const
{
  for (size_t i = 0; i < STREAMFRAGMENTS; ++i)
  {
    if (m_buffers[i] == buffer)
      return m_buffer_durations[i];
  }

  return 0.0;
}

bool
StreamSoundSource::fillBufferAndQueue(ALuint buffer)
{
  if (!m_file)
    return false;

  // fill buffer
  std::unique_ptr<char[]> bufferdata(new char[STREAMFRAGMENTSIZE]);
  size_t bytesread = 0;
  do {
    bytesread += m_file->read(static_cast<char *>(bufferdata.get()) + bytesread,
      STREAMFRAGMENTSIZE - bytesread);
    // end of sound file
    if (bytesread < STREAMFRAGMENTSIZE) {
      if (m_looping)
        m_file->reset();
      else
      {
        m_reached_end = true;
        break;
      }
    }
  } while(bytesread < STREAMFRAGMENTSIZE);

  if (bytesread > 0) {
    ALenum format = SoundManager::get_sample_format(*m_file);
    const int bytes_per_second = m_file->m_rate * m_file->m_channels * (m_file->m_bits_per_sample / 8);
    const double duration = bytes_per_second > 0 ?
      static_cast<double>(bytesread) / static_cast<double>(bytes_per_second) : 0.0;
    try
    {
      alBufferData(buffer, format, bufferdata.get(), static_cast<ALsizei>(bytesread), m_file->m_rate);
      SoundManager::check_al_error("Couldn't refill audio buffer: ");

      alSourceQueueBuffers(m_source, 1, &buffer);
      SoundManager::check_al_error("Couldn't queue audio buffer: ");
      record_buffer_duration(buffer, duration);
    }
    catch(std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }
  }

  // return false if there aren't more buffers to fill
  return bytesread >= STREAMFRAGMENTSIZE;
}
