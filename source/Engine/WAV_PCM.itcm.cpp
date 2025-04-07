//
// Created by cervi on 17/10/2023.
//
#include "Engine/WAV.hpp"
#include "Engine/dma.hpp"

namespace Audio2 {
    void WAV::progress_pcm8_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        u8 *leftBuffer = _leftBuffer.get();
        u8 *fileBuffer = _fileBuffer;
        dmaCopySafe(
            3,
            fileBuffer + _fileBufferSamplePos,
            leftBuffer + _sampleBufferPos,
            samples
        );
    }

    void WAV::progress_pcm8_stereo(u16 samples) {
        if (_leftBuffer == nullptr || _rightBuffer == nullptr)
            return;
        u8 *leftBuffer = _leftBuffer.get();
        u8 *rightBuffer = _rightBuffer.get();
        u8 *fileBuffer = _fileBuffer;
        for (int i = 0; i < samples * 2; i++) {
            u32 audioBufferPos = _sampleBufferPos + i / 2;
            audioBufferPos %= kAudioBuffer;
            if (i % 2 == 0) {
                leftBuffer[audioBufferPos] = *(fileBuffer + 2 * _fileBufferSamplePos + i);
            } else {
                rightBuffer[audioBufferPos] = *(fileBuffer + 2 * _fileBufferSamplePos + i);
            }
        }

        DC_FlushRange(leftBuffer + _sampleBufferPos, samples);
        DC_FlushRange(rightBuffer + _sampleBufferPos, samples);
    }

    void WAV::progress_pcm16_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        u16 *leftBuffer = reinterpret_cast<u16 *>(_leftBuffer.get());
        u16 *fileBuffer = reinterpret_cast<u16 *>(_fileBuffer);
        dmaCopySafe(
            3,
            fileBuffer + _fileBufferSamplePos,
            leftBuffer + _sampleBufferPos % kAudioBuffer,
            samples * 2
        );
    }

    void WAV::progress_pcm16_stereo(u16 samples) {
        if (_leftBuffer == nullptr || _rightBuffer == nullptr)
            return;
        u16 *leftBuffer = reinterpret_cast<u16 *>(_leftBuffer.get());
        u16 *rightBuffer = reinterpret_cast<u16 *>(_rightBuffer.get());
        u16 *fileBuffer = reinterpret_cast<u16 *>(_fileBuffer);
        for (int i = 0; i < samples * 2; i++) {
            u32 audioBufferPos = _sampleBufferPos + i / 2;
            audioBufferPos %= kAudioBuffer;
            if (i % 2 == 0) {
                leftBuffer[audioBufferPos] = *(fileBuffer + 2 * _fileBufferSamplePos + i);
            } else {
                rightBuffer[audioBufferPos] = *(fileBuffer + 2 * _fileBufferSamplePos + i);
            }
        }

        DC_FlushRange(leftBuffer + _sampleBufferPos % kAudioBuffer, samples * 2);
        DC_FlushRange(rightBuffer + _sampleBufferPos % kAudioBuffer, samples * 2);
    }
}
