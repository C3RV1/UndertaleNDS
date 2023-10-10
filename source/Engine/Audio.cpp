//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "Engine/Engine.hpp"
#include "DEBUG_FLAGS.hpp"
#include <algorithm>
#include <utility>
#include "Engine/dma.hpp"

namespace Audio2 {
    void WAV::loadWAV(const std::string& name) {
        free_();
        if (name.empty())  // Loading an empty WAV is the same as freeing it
            return;
        _loops = 0;
        std::string realPath = "nitro:/z_audio/" + name;
        FILE *f = fopen(realPath.c_str(), "rb");
        _filename = name;
        if (f == nullptr) {
            std::string buffer = "Error opening WAV #r" + name;
            Engine::throw_(buffer);
        }

        char header[4];

        const char riffHeader[4] = {'R', 'I', 'F', 'F'};
        const char waveHeader[4] = {'W', 'A', 'V', 'E'};
        const char fmtHeader[4] = {'f', 'm', 't', ' '};
        const char dataHeader[4] = {'d', 'a', 't', 'a'};

        fread(header, 4, 1, f);
        if (memcmp(header, riffHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid RIFF header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        u32 fileSize;
        fread(&fileSize, 4, 1, f);

        fread(header, 4, 1, f);
        if (memcmp(header, waveHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid WAVE header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        // fmt header
        fread(header, 4, 1, f);
        if (memcmp(header, fmtHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid FMT header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        fseek(f, ftell(f) + 4, SEEK_SET); // skip chunk size == 0x10

        u16 format, channels, bitsPerSample;
        fread(&format, 2, 1, f);
        fread(&channels, 2, 1, f);
        fread(&_sampleRate, 4, 1, f);
        fseek(f, ftell(f) + 4, SEEK_SET); // skip byte rate == self.sample_rate * self.bits_per_sample * self.num_channels // 8
        fseek(f, ftell(f) + 2, SEEK_SET); // skip block align == self.num_channels * self.bits_per_sample // 8
        fread(&bitsPerSample, 2, 1, f);

        if (format == 1) {
            if (bitsPerSample == 8) {
                _format = SoundFormat_8Bit;
            }
            else if (bitsPerSample == 16) {
                _format = SoundFormat_16Bit;
            }
            else {
                std::string buffer = "Error opening WAV #r" + name +
                                     "#x: Invalid bits per sample.";
                fclose(f);
                Engine::throw_(buffer);
            }
        }
        else {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid format.";
            fclose(f);
            Engine::throw_(buffer);
        }

        if (channels > 2) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid channels.";
            fclose(f);
            Engine::throw_(buffer);
        }

        _stereo = channels == 2;

        u32 chunkSize = 0;
        // data chunk
        while (ftell(f) < fileSize + 8) {
            fread(header, 4, 1, f);
            fread(&chunkSize, 4, 1, f);
            if (memcmp(header, dataHeader, 4) == 0)
                break;
            fseek(f, chunkSize, SEEK_CUR);
        }
        if (chunkSize == 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Couldn't find DATA chunk.";
            fclose(f);
            Engine::throw_(buffer);
        }

        _dataEnd = ftell(f) + chunkSize;
        _dataStart = ftell(f);

        u8 bytesPerSample;
        switch (_format) {
            case SoundFormat_8Bit:
                bytesPerSample = 1;
                break;
            case SoundFormat_16Bit:
            case SoundFormat_ADPCM:
                bytesPerSample = 2;
                break;
            default:
                Engine::throw_("Format error 2!");
        }

        _leftBuffer = new u8[bytesPerSample * kWAVBuffer];

        if (_stereo) {
            _rightBuffer = new u8[bytesPerSample * kWAVBuffer];
        }

        _loaded = true;
        _stream = f;
    }

    void WAV::progress_pcm8_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        u8* leftBuffer = static_cast<u8 *>(_leftBuffer);
        u8* fileBuffer = _fileBuffer;
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kWAVBuffer * (_sampleBufferPos / kWAVBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;
            dmaCopy(leftBuffer + _sampleBufferPos, fileBuffer + _fileBufferSamplePos, max_copy);

            _fileBufferSamplePos += max_copy;
            _sampleBufferPos += max_copy;
            if (_fileBufferSamplePos >= _fileBufferSampleEnd) {
                if (renew_pcm8_mono_file_buffer()) {
                    // We should stop the audio if we have completed.
                    if (_expectedSampleBufferPos > _sampleBufferPos)
                        stop();
                    break;
                }
            }

            samples -= max_copy;
        }
    }

    bool WAV::renew_pcm8_mono_file_buffer() {
        if (ftell(_stream) >= _dataEnd) {
            if (_loops == 0) {
                return true;
            }
            else if (_loops > 0) {
                _loops--;
            }
            fseek(_stream, _dataStart, SEEK_SET);
        }

        u32 maxReadSize = _dataEnd - ftell(_stream);
        if (maxReadSize >= kWAVBuffer * 2)
            maxReadSize = kWAVBuffer * 2;
        fread(_fileBuffer, maxReadSize, 1, _stream);
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = maxReadSize;
        return false;
    }

    void WAV::progress_pcm8_stereo(u16 samples) {
        if (_leftBuffer == nullptr || _rightBuffer == nullptr)
            return;
        u8* leftBuffer = static_cast<u8 *>(_leftBuffer);
        u8* rightBuffer = static_cast<u8 *>(_rightBuffer);
        u8* fileBuffer = _fileBuffer;
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kWAVBuffer * (_sampleBufferPos / kWAVBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;

            for (int i = 0; i < max_copy * 2; i++) {
                if (i % 2 == 0) {
                    leftBuffer[_sampleBufferPos + i / 2] = *(fileBuffer + 2*_fileBufferSamplePos);
                }
                else {
                    rightBuffer[_sampleBufferPos + i / 2] = *(fileBuffer + 2*_fileBufferSamplePos + 1);
                    _fileBufferSamplePos++;
                }
            }

            _sampleBufferPos += max_copy;

            if (_fileBufferSamplePos >= _fileBufferSampleEnd) {
                // Renew file buffer
                if (renew_pcm8_stereo_file_buffer()) {
                    // We should stop the audio if we have completed.
                    if (_expectedSampleBufferPos > _sampleBufferPos)
                        stop();
                    break;
                }
            }

            samples -= max_copy;
        }

        DC_FlushRange(leftBuffer, kWAVBuffer);
        DC_FlushRange(rightBuffer, kWAVBuffer);
    }

    bool WAV::renew_pcm8_stereo_file_buffer() {
        if (ftell(_stream) >= _dataEnd) {
            if (_loops == 0) {
                return true;
            }
            else if (_loops > 0) {
                _loops--;
            }
            fseek(_stream, _dataStart, SEEK_SET);
        }

        u32 maxReadSize = _dataEnd - ftell(_stream);
        if (maxReadSize >= kWAVBuffer * 2)
            maxReadSize = kWAVBuffer * 2;
        fread(_fileBuffer, maxReadSize, 1, _stream);
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = maxReadSize / 2;
        return false;
    }

    void WAV::progress_pcm16_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        u16* leftBuffer = reinterpret_cast<u16 *>(_leftBuffer);
        u16* fileBuffer = reinterpret_cast<u16 *>(_fileBuffer);
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kWAVBuffer * (_sampleBufferPos / kWAVBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;
            dmaCopy(leftBuffer + _sampleBufferPos % kWAVBuffer, fileBuffer + _fileBufferSamplePos, max_copy * 2);

            _fileBufferSamplePos += max_copy;

            _sampleBufferPos += max_copy;

            if (_fileBufferSamplePos >= _fileBufferSampleEnd) {
                if (renew_pcm16_mono_file_buffer()) {
                    // We should stop the audio if we have completed.
                    if (_expectedSampleBufferPos > _sampleBufferPos)
                        stop();
                    break;
                }
            }

            samples -= max_copy;
        }
    }

    bool WAV::renew_pcm16_mono_file_buffer() {
        if (ftell(_stream) >= _dataEnd) {
            if (_loops == 0) {
                return true;
            }
            else if (_loops > 0) {
                _loops--;
            }
            fseek(_stream, _dataStart, SEEK_SET);
        }

        u32 maxReadSize = _dataEnd - ftell(_stream);
        if (maxReadSize >= kWAVBuffer * 2)
            maxReadSize = kWAVBuffer * 2;
        fread(_fileBuffer, maxReadSize, 1, _stream);
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = maxReadSize / 4;
        return false;
    }

    void WAV::progress_pcm16_stereo(u16 samples) {
        if (_leftBuffer == nullptr || _rightBuffer == nullptr)
            return;
        u16* leftBuffer = reinterpret_cast<u16*>(_leftBuffer);
        u16* rightBuffer = reinterpret_cast<u16*>(_rightBuffer);
        u16* fileBuffer = reinterpret_cast<u16*>(_fileBuffer);
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kWAVBuffer * (_sampleBufferPos / kWAVBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;

            for (int i = 0; i < max_copy * 2; i++) {
                if (i % 2 == 0) {
                    leftBuffer[_sampleBufferPos % kWAVBuffer + i / 2] = *(fileBuffer + 2*_fileBufferSamplePos);
                }
                else {
                    rightBuffer[_sampleBufferPos % kWAVBuffer + i / 2] = *(fileBuffer + 2*_fileBufferSamplePos + 1);
                    _fileBufferSamplePos++;
                }
            }

            _sampleBufferPos += max_copy;

            if (_fileBufferSamplePos >= _fileBufferSampleEnd) {
                // Renew file buffer
                if (renew_pcm16_stereo_file_buffer()) {
                    // We should stop the audio if we have completed.
                    if (_expectedSampleBufferPos > _sampleBufferPos)
                        stop();
                    break;
                }
            }

            samples -= max_copy;
        }

        DC_FlushRange(leftBuffer, kWAVBuffer * 2);
        DC_FlushRange(rightBuffer, kWAVBuffer * 2);
    }

    bool WAV::renew_pcm16_stereo_file_buffer() {
        if (ftell(_stream) >= _dataEnd) {
            if (_loops == 0) {
                return true;
            }
            else if (_loops > 0) {
                _loops--;
            }
            fseek(_stream, _dataStart, SEEK_SET);
        }

        u32 maxReadSize = _dataEnd - ftell(_stream);
        if (maxReadSize >= kWAVBuffer * 2)
            maxReadSize = kWAVBuffer * 2;
        fread(_fileBuffer, maxReadSize, 1, _stream);
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = maxReadSize / 4;
        return false;
    }

    void WAV::progress(u16 samples) {
        if (!_stereo) {
            switch (_format) {
                case SoundFormat_8Bit:
                    progress_pcm8_mono(samples);
                    break;
                case SoundFormat_16Bit:
                    progress_pcm16_mono(samples);
                    break;
                default:
                    Engine::throw_("Format error 3!");
            }
        }
        else {
            switch (_format) {
                case SoundFormat_8Bit:
                    progress_pcm8_stereo(samples);
                    break;
                case SoundFormat_16Bit:
                    progress_pcm16_stereo(samples);
                    break;
                default:
                    Engine::throw_("Format " + std::to_string(_format) + " error 4!");
            }
        }
    }

    void WAV::update() {
        u16 timerTicks = timerTick(audioManager.getTimerId());
        u16 timerElapsed = timerTicks - _timerLast;
        u32 samples = ((u32)timerElapsed * (u32)_sampleRate) / (BUS_CLOCK / 1024);
        _expectedSampleBufferPos += samples;

        progress(samples);

        _timerLast = timerTicks;
    }

    void WAV::play() {
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

        u8 bytesPerSample;
        switch (_format) {
            case SoundFormat_8Bit:
                bytesPerSample = 1;
                break;
            case SoundFormat_16Bit:
            case SoundFormat_ADPCM:
                bytesPerSample = 2;
                break;
            default:
                Engine::throw_("Format error 1!");
        }

        fseek(_stream, _dataStart, SEEK_SET);
        _sampleBufferPos = 0;
        _expectedSampleBufferPos = 0;
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = 0;

        if (_stereo) {
            _leftChannel = soundPlaySample(_leftBuffer, _format, bytesPerSample * kWAVBuffer, _sampleRate,
                                           127, 0, true, 0);
            _rightChannel = soundPlaySample(_rightBuffer, _format, bytesPerSample * kWAVBuffer, _sampleRate,
                                            127, 127, true, 0);
        }
        else {
            _leftChannel = soundPlaySample(_leftBuffer, _format, bytesPerSample * kWAVBuffer, _sampleRate,
                            127, 64, true, 0);
        }

        _timerLast = timerTick(audioManager.getTimerId());
        audioManager.addPlayingWAV(this);

        progress(kWAVBuffer / 2);
        nocashMessage("End load");
    }

    void WAV::stop() {
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

        audioManager.removePlayingWAV(this);
        selfFreeingPtr = nullptr;
    }

    WAV::~WAV() {
        free_();
    }

    void WAV::free_() {
        if (_active) {
            stop();
        }

        if (_stream != nullptr) {
            fclose(_stream);
            _stream = nullptr;
        }

        _loaded = false;

        delete[] _leftBuffer;
        _leftBuffer = nullptr;

        delete[] _rightBuffer;
        _rightBuffer = nullptr;
    }

    AudioManager::AudioManager(int timerId) {
        for (int i = 0; i < 16; i++) {
            _enabledChannels[i] = true;
            _activeChannels[i] = false;
        }
        soundEnable();
        _timerId = timerId;
        timerStart(timerId, ClockDivider_1024, 0, nullptr);
    }

    int AudioManager::getFreeChannel() {
        for (int i = 0; i < 16; i++) {
            if (!_activeChannels[i] && _enabledChannels[i]) {
                _activeChannels[i] = true;
                return i;
            }
        }
        Engine::throw_("Couldn't get free channel!");
    }

    void AudioManager::freeChannel(int channel) {
        if (0 <= channel && channel <= 15)
            _activeChannels[channel] = false;
    }

    void AudioManager::addPlayingWAV(Audio2::WAV *wav) {
        _wavPlaying.push_front(wav);
    }

    void AudioManager::removePlayingWAV(Audio2::WAV *wav) {
        auto idx = std::find(_wavPlaying.begin(), _wavPlaying.end(), wav);
        if (idx != _wavPlaying.end())
            _wavPlaying.erase(idx);
    }

    void AudioManager::update() {
        auto current = _wavPlaying.begin();
        while (current != _wavPlaying.end()) {
            WAV* currentWAV = *(current++);
            currentWAV->update();
        }
    }

    AudioManager audioManager(0);

    void playBGMusic(const std::string& filename, bool loop) {
        stopBGMusic();
        cBGMusic.loadWAV(filename);
        cBGMusic.setLoops(loop ? -1 : 0);
        if (cBGMusic.getLoaded())
            cBGMusic.play();
    }

    void stopBGMusic() {
        cBGMusic.stop();
    }

    WAV cBGMusic;
}
