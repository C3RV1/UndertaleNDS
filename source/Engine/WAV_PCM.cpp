//
// Created by cervi on 17/10/2023.
//
#include "Engine/WAV.hpp"
#include "Engine/dma.hpp"

namespace Audio2 {
    void WAV::progress_pcm8_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        u8* leftBuffer = _leftBuffer.get();
        u8* fileBuffer = _fileBuffer;
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kAudioBuffer - _sampleBufferPos % kAudioBuffer;
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
        u8* leftBuffer = _leftBuffer.get();
        u8* rightBuffer = _rightBuffer.get();
        u8* fileBuffer = _fileBuffer;
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kAudioBuffer - _sampleBufferPos % kAudioBuffer;
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
        u16* leftBuffer = reinterpret_cast<u16 *>(_leftBuffer.get());
        u16* fileBuffer = reinterpret_cast<u16 *>(_fileBuffer);
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kAudioBuffer - _sampleBufferPos % kAudioBuffer;
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
        u16* leftBuffer = reinterpret_cast<u16*>(_leftBuffer.get());
        u16* rightBuffer = reinterpret_cast<u16*>(_rightBuffer.get());
        u16* fileBuffer = reinterpret_cast<u16*>(_fileBuffer);
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kAudioBuffer - _sampleBufferPos % kAudioBuffer;
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
}
