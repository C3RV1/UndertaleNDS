//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "DEBUG_FLAGS.hpp"
#include <algorithm>
#include "Engine/dma.hpp"

namespace Audio2 {
    void AudioFile::allocateBuffers() {
        _leftBuffer = nullptr;
        _rightBuffer = nullptr;

        _leftBuffer = std::unique_ptr<u8[]>(new u8[(getBitsPerSample() * kAudioBuffer) / 8]);

        if (_stereo) {
            _rightBuffer = std::unique_ptr<u8[]>(new u8[(getBitsPerSample() * kAudioBuffer) / 8]);
        }
    }

    void AudioFile::play() {
        if (!_loaded)
            return;
        if (_active) {
            stop();
        }
        _active = true;

#ifdef DEBUG_AUDIO
        std::string buffer = "Starting wav: " + getFilename() + " stereo " + std::to_string(getStereo()) +
            " sample rate " + std::to_string(_sampleRate) + " format " + std::to_string(_format);
        nocashMessage(buffer.c_str());
#endif
        resetPlaying();

        if (_stereo) {
            _leftChannel = soundPlaySample(_leftBuffer.get(), getAllocFormat(),
                                           (getBitsPerSample() * kAudioBuffer) / 8,
                                           _sampleRate, _volume, 0, true, 0);
            _rightChannel = soundPlaySample(_rightBuffer.get(), getAllocFormat(),
                                            (getBitsPerSample() * kAudioBuffer) / 8, _sampleRate,
                                            _volume, 127, true, 0);
        }
        else {
            _leftChannel = soundPlaySample(_leftBuffer.get(), getAllocFormat(),
                                           (getBitsPerSample() * kAudioBuffer) / 8, _sampleRate,
                                           _volume, 64, true, 0);
        }

        _timerLast = timerTick(audioManager.getTimerId());
        audioManager.addPlaying(this);

        progress(kAudioBuffer / 2);
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

        audioManager.removePlaying(this);
        selfFreeingPtr = nullptr;
    }

    void AudioFile::update() {
        if (!_active)
            return;
        u16 timerTicks = timerTick(audioManager.getTimerId());
        u16 timerElapsed = timerTicks - _timerLast;
        u32 samples = ((u32)timerElapsed * (u32)_sampleRate + _ticksRemain) / (BUS_CLOCK / 1024);
        _ticksRemain = ((u32)timerElapsed * (u32)_sampleRate + _ticksRemain) % (BUS_CLOCK / 1024);
        _expectedSampleBufferPos += samples;

        progress(samples);

        _timerLast = timerTicks;
    }

    void AudioManager::addPlaying(Audio2::AudioFile *wav) {
        _playing.push_front(wav);
    }

    void AudioManager::removePlaying(Audio2::AudioFile *wav) {
        auto idx = std::find(_playing.begin(), _playing.end(), wav);
        if (idx != _playing.end())
            _playing.erase(idx);
    }

    void AudioManager::update() {
        auto current = _playing.begin();
        while (current != _playing.end()) {
            AudioFile* current_audio_file = *(current++);
            current_audio_file->update();
        }
    }

    AudioManager::AudioManager(int timerId) {
        soundEnable();
        _timerId = timerId;
        timerStart(timerId, ClockDivider_1024, 0, nullptr);
    }

    AudioManager audioManager(0);
}
