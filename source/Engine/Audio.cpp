//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "DEBUG_FLAGS.hpp"
#include <memory>

namespace Audio2 {
void AudioFile::allocateBuffers() {
  if (!_leftBuffer)
    _leftBuffer =
        std::unique_ptr<u8[]>(new u8[(getBitsPerSample() * kAudioBuffer) / 8]);

  if (_stereo && !_rightBuffer)
    _rightBuffer =
        std::unique_ptr<u8[]>(new u8[(getBitsPerSample() * kAudioBuffer) / 8]);
}

void AudioManager::play(std::shared_ptr<AudioFile> audio_file) {
  if (!audio_file) {
    nocashMessage("Tried to play nullptr audio_file!");
    return;
  }

  if (audio_file->play()) {
    for (const auto &current : _playing) {
      if (current.get() == audio_file.get())
        return;
    }
    _playing.push_back(std::move(audio_file));
  }
}

bool AudioFile::play() {
  if (!_loaded)
    return false;
  if (_active) {
    stop();
  }
  _active = true;

#ifdef DEBUG_AUDIO
  std::string buffer = "Starting wav: " + getFilename() + " stereo " +
                       std::to_string(getStereo()) + " sample rate " +
                       std::to_string(_sampleRate) + " format " +
                       std::to_string(_format);
  nocashMessage(buffer.c_str());
#endif

  resetPlaying();

  if (_stereo) {
    _leftChannel = soundPlaySample(_leftBuffer.get(), getAllocFormat(),
                                   (getBitsPerSample() * kAudioBuffer) / 8,
                                   _sampleRate, _volume, 0, true, 0);
    _rightChannel = soundPlaySample(_rightBuffer.get(), getAllocFormat(),
                                    (getBitsPerSample() * kAudioBuffer) / 8,
                                    _sampleRate, _volume, 127, true, 0);
  } else {
    _leftChannel = soundPlaySample(_leftBuffer.get(), getAllocFormat(),
                                   (getBitsPerSample() * kAudioBuffer) / 8,
                                   _sampleRate, _volume, 64, true, 0);
  }

  _timerLast = timerTick(audioManager.getTimerId());

  progress(kAudioBuffer / 2);
  return true;
}

void AudioFile::setVolume(u8 volume) {
  if (volume == _volume)
    return;
  if (_leftChannel != -1)
    soundSetVolume(_leftChannel, volume);
  if (_rightChannel != -1)
    soundSetVolume(_rightChannel, volume);
  _volume = volume;
}

void AudioManager::stop(const std::shared_ptr<AudioFile> &audio_file) {
  if (!audio_file) {
    nocashMessage("Tried to stop nullptr audio_file!");
    return;
  }

  if (!audio_file->getPlaying())
    return;
  audio_file->stop();
  for (auto current = _playing.begin(); current != _playing.end(); ++current) {
    if (audio_file.get() == current->get()) {
      _playing.erase(current);
      break;
    }
  }
}

void AudioFile::stop() {
  if (!_active)
    return;

#ifdef DEBUG_AUDIO
  char buffer[100];
  sprintf(buffer, "Stopping wav: %s", getFilename().c_str());
  nocashMessage(buffer);
#endif

  _active = false;

  soundKill(_leftChannel);
  if (_stereo)
    soundKill(_rightChannel);

  _leftChannel = -1;
  _rightChannel = -1;
}

ITCM_CODE
void AudioFile::update() {
  if (!_active)
    return;
  u16 timerTicks = timerTick(audioManager.getTimerId());
  u16 timerElapsed = timerTicks - _timerLast;
  u32 samples = ((u32)timerElapsed * (u32)_sampleRate + _ticksRemain) /
                (BUS_CLOCK / 1024);
  _ticksRemain = ((u32)timerElapsed * (u32)_sampleRate + _ticksRemain) %
                 (BUS_CLOCK / 1024);
  _expectedSampleBufferPos += samples;

  progress(samples);

  _timerLast = timerTicks;
}

ITCM_CODE
void AudioManager::update() {
  for (const auto &current : _playing) {
    current->update();
  }
  for (auto current = _playing.begin(); current != _playing.end();) {
    if (!(*current)->getPlaying()) {
      current = _playing.erase(current);
    } else
      ++current;
  }
}

AudioManager::AudioManager(int timerId) {
  soundEnable();
  _timerId = timerId;
  timerStart(timerId, ClockDivider_1024, 0, nullptr);
}

AudioManager audioManager(0);
} // namespace Audio2
