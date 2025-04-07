//
// Created by cervi on 16/10/2023.
//

#include "Engine/WAV.hpp"

namespace Audio2 {
    void ADPCM_Decoder::initPredictor(int new_sample, int index) {
        _new_sample = new_sample;
        _index = index;
    }

    s16 ADPCM_Decoder::decodeSample(u8 inputNibble) {
        int step = kStepSizeTable[_index];
        int difference = step >> 3;

        if (inputNibble & 4)
            difference += step;
        if (inputNibble & 2)
            difference += step >> 1;
        if (inputNibble & 1)
            difference += step >> 2;

        if (inputNibble & 8)
            difference = -difference;

        _new_sample += difference;
        if (_new_sample > 32767)
            _new_sample = 32767;
        else if (_new_sample < -32768)
            _new_sample = -32768;

        _index += kIndexTable[inputNibble];
        if (_index < 0)
            _index = 0;
        else if (_index > 88)
            _index = 88;
        return _new_sample;
    }

    u32 WAV::ima_adpcm_get_sample_position(u32 sampleIdx, u8 channelIdx, bool& highBits,
                                           bool& readHeader) {
        u8 channels = _stereo ? 2 : 1;
        // Remove the 7 extra samples in the header
        u32 samplesPerBlock = _blockAlign * 2 / channels - 7;

        u32 block = sampleIdx / samplesPerBlock;
        u32 absOffset = block * _blockAlign;
        sampleIdx %= samplesPerBlock;

        if (sampleIdx == 0) {
            readHeader = true;
            return absOffset + channelIdx * 4;
        }
        else {
            readHeader = false;
        }
        sampleIdx -= 1;

        u32 channelBlock = sampleIdx / 8;
        sampleIdx %= 8;

        absOffset += ((channelBlock + 1) * channels + channelIdx) * 4;

        u8 sampleByte = sampleIdx / 2;
        highBits = sampleIdx & 1;

        absOffset += sampleByte;

        return absOffset;
    }

    void WAV::progress_ima_adpcm_mono(u16 samples) {
        if (_leftBuffer == nullptr)
            return;
        s16 *leftBuffer = reinterpret_cast<s16 *>(_leftBuffer.get());
        u8 *fileBuffer = _fileBuffer;
        for (int i = 0; i < samples; i++) {
            bool highBits, readHeader;
            u32 samplePos;
            samplePos = ima_adpcm_get_sample_position(
                _sourceBufferPos + i, 0,
                highBits, readHeader
            );
            if (readHeader) {
                u16 new_sample;
                u8 index;
                samplePos %= kAudioBuffer * 2;
                new_sample = fileBuffer[samplePos];
                new_sample += fileBuffer[samplePos + 1] << 8;
                index = fileBuffer[samplePos + 2];
                _leftChannelDecoder.initPredictor((s16) new_sample, index);
                leftBuffer[(_sampleBufferPos + i) % kAudioBuffer] = (s16) new_sample;
                continue;
            }

            u8 byte = fileBuffer[samplePos % (kAudioBuffer * 2)];
            u8 nibble = byte;
            if (highBits)
                nibble >>= 4;
            nibble &= 0xF;

            s16 sample = _leftChannelDecoder.decodeSample(nibble);
            leftBuffer[(_sampleBufferPos + i) % kAudioBuffer] = sample;
        }

        DC_FlushRange(leftBuffer + _sampleBufferPos % kAudioBuffer, samples * 2);
    }

    void WAV::progress_ima_adpcm_stereo(u16 samples) {
        if (_leftBuffer == nullptr || _rightBuffer == nullptr)
            return;
        s16 *leftBuffer = reinterpret_cast<s16 *>(_leftBuffer.get());
        s16 *rightBuffer = reinterpret_cast<s16 *>(_rightBuffer.get());
        u8 *fileBuffer = _fileBuffer;
        for (int i = 0; i < samples * 2; i++) {
            u8 channelIdx = i % 2;
            bool highBits, readHeader;
            u32 samplePos;
            samplePos = ima_adpcm_get_sample_position(
                _sourceBufferPos + i / 2, channelIdx,
                highBits, readHeader
            );
            if (readHeader) {
                u16 new_sample;
                u8 index;
                samplePos %= kAudioBuffer * 2;
                new_sample = fileBuffer[samplePos];
                new_sample += fileBuffer[samplePos + 1] << 8;
                index = fileBuffer[samplePos + 2];
                if (channelIdx == 0) {
                    _leftChannelDecoder.initPredictor((s16) new_sample, index);
                    leftBuffer[(_sampleBufferPos + i / 2) % kAudioBuffer] = (s16) new_sample;
                } else {
                    _rightChannelDecoder.initPredictor((s16) new_sample, index);
                    rightBuffer[(_sampleBufferPos + i / 2) % kAudioBuffer] = (s16) new_sample;
                }
                continue;
            }

            u8 byte = fileBuffer[samplePos % (kAudioBuffer * 2)];
            u8 nibble = byte;
            if (highBits)
                nibble >>= 4;
            nibble &= 0xF;

            if (channelIdx == 0) {
                s16 sample = _leftChannelDecoder.decodeSample(nibble);
                leftBuffer[(_sampleBufferPos + i / 2) % kAudioBuffer] = sample;
            } else {
                s16 sample = _rightChannelDecoder.decodeSample(nibble);
                rightBuffer[(_sampleBufferPos + i / 2) % kAudioBuffer] = sample;
            }
        }

        DC_FlushRange(leftBuffer + _sampleBufferPos % kAudioBuffer, samples * 2);
        DC_FlushRange(rightBuffer + _sampleBufferPos % kAudioBuffer, samples * 2);
    }
}