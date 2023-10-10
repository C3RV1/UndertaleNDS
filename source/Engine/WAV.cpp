//
// Created by cervi on 10/10/2023.
//
#include "Engine/Engine.hpp"
#include "Engine/WAV.hpp"
#include "Engine/dma.hpp"

namespace Audio2 {
    void WAV::load(const std::string& name) {
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
        u32 sampleRate;
        fread(&format, 2, 1, f);
        fread(&channels, 2, 1, f);
        fread(&sampleRate, 4, 1, f);
        _sampleRate = sampleRate;
        fseek(f, ftell(f) + 4, SEEK_SET); // skip byte rate == self.sample_rate * self.bits_per_sample * self.num_channels // 8
        fseek(f, ftell(f) + 2, SEEK_SET); // skip block align == self.num_channels * self.bits_per_sample // 8
        fread(&bitsPerSample, 2, 1, f);
        _bitsPerSample = bitsPerSample;

        if (format == 1) {
            if (_bitsPerSample == 8) {
                _format = SoundFormat_8Bit;
            }
            else if (_bitsPerSample == 16) {
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

        allocateBuffers();

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
            u32 remainingLeftBuffer = kAudioBuffer * (_sampleBufferPos / kAudioBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;
            dmaCopySafe(3,
                        fileBuffer + _fileBufferSamplePos,
                        leftBuffer + _sampleBufferPos,
                        max_copy);

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
        if (maxReadSize >= kAudioBuffer * 2)
            maxReadSize = kAudioBuffer * 2;
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
            u32 remainingLeftBuffer = kAudioBuffer * (_sampleBufferPos / kAudioBuffer + 1) - _sampleBufferPos;
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

        DC_FlushRange(leftBuffer, kAudioBuffer);
        DC_FlushRange(rightBuffer, kAudioBuffer);
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
        if (maxReadSize >= kAudioBuffer * 2)
            maxReadSize = kAudioBuffer * 2;
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
            u32 remainingLeftBuffer = kAudioBuffer * (_sampleBufferPos / kAudioBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;
            dmaCopySafe(3,
                        fileBuffer + _fileBufferSamplePos,
                        leftBuffer + _sampleBufferPos % kAudioBuffer,
                        max_copy * 2);

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
        if (maxReadSize >= kAudioBuffer * 2)
            maxReadSize = kAudioBuffer * 2;
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
            u32 remainingLeftBuffer = kAudioBuffer * (_sampleBufferPos / kAudioBuffer + 1) - _sampleBufferPos;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;

            for (int i = 0; i < max_copy * 2; i++) {
                if (i % 2 == 0) {
                    leftBuffer[_sampleBufferPos % kAudioBuffer + i / 2] = *(fileBuffer + 2 * _fileBufferSamplePos);
                }
                else {
                    rightBuffer[_sampleBufferPos % kAudioBuffer + i / 2] = *(fileBuffer + 2 * _fileBufferSamplePos + 1);
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

        DC_FlushRange(leftBuffer, kAudioBuffer * 2);
        DC_FlushRange(rightBuffer, kAudioBuffer * 2);
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
        if (maxReadSize >= kAudioBuffer * 2)
            maxReadSize = kAudioBuffer * 2;
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
                    Engine::throw_("WAV: Format " + std::to_string(_format) + " not implemented");
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
                    Engine::throw_("WAV: Format " + std::to_string(_format) + " not implemented");
            }
        }
    }

    void WAV::resetPlaying() {
        fseek(_stream, _dataStart, SEEK_SET);
        _sampleBufferPos = 0;
        _expectedSampleBufferPos = 0;
        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = 0;
    }

    WAV::~WAV() {
        WAV::free_();
    }

    void WAV::free_() {
        if (!_loaded)
            return;

        if (_stream != nullptr) {
            fclose(_stream);
            _stream = nullptr;
        }

        AudioFile::free_();
    }

    void playBGMusic(const std::string& filename, bool loop) {
        stopBGMusic();
        cBGMusic.load(filename);
        cBGMusic.setLoops(loop ? -1 : 0);
        if (cBGMusic.getLoaded())
            cBGMusic.play();
    }

    void stopBGMusic() {
        cBGMusic.stop();
    }

    WAV cBGMusic;
}
