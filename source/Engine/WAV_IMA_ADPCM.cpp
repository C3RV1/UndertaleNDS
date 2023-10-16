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
        s16* leftBuffer = reinterpret_cast<s16 *>(_leftBuffer.get());
        u8* fileBuffer = _fileBuffer;
        while (samples > 0) {
            u32 remainingFileBuffer = _fileBufferSampleEnd - _fileBufferSamplePos;
            u32 remainingLeftBuffer = kAudioBuffer - _sampleBufferPos % kAudioBuffer;
            u32 max_copy = remainingFileBuffer < remainingLeftBuffer ? remainingFileBuffer : remainingLeftBuffer;
            max_copy = samples < max_copy ? samples : max_copy;

            for (int i = 0; i < max_copy; i++) {
                bool highBits, readHeader;
                u32 samplePos;
                samplePos = ima_adpcm_get_sample_position(_sourceBufferPos + i, 0,
                                                          highBits, readHeader);
                if (readHeader) {
                    u16 new_sample;
                    u8 index;
                    samplePos %= kAudioBuffer * 2;
                    new_sample = fileBuffer[samplePos];
                    new_sample += fileBuffer[samplePos + 1] << 8;
                    index = fileBuffer[samplePos + 2];
                    _leftChannelDecoder.initPredictor((s16)new_sample, index);
                    leftBuffer[(_sampleBufferPos + i) % kAudioBuffer] = (s16)new_sample;
                    continue;
                }

                u8 byte = fileBuffer[samplePos % (kAudioBuffer * 2)];
                u8 nibble = byte;
                if (highBits)
                    nibble >>= 4;
                nibble &= 0xF;

                s16 sample = _leftChannelDecoder.decodeSample(nibble);
                if (sample > 30000) {
                    std::string buffer = "SAMPLE POS " + std::to_string(samplePos) + " SAMPLE " + std::to_string(sample);
                    nocashMessage(buffer.c_str());
                }

                leftBuffer[(_sampleBufferPos + i) % kAudioBuffer] = sample;
            }

            _fileBufferSamplePos += max_copy;
            _sampleBufferPos += max_copy;
            _sourceBufferPos += max_copy;

            if (_fileBufferSamplePos >= _fileBufferSampleEnd) {
                if (renew_ima_adpcm_mono_file_buffer()) {
                    nocashMessage("Completed");
                    // We should stop the audio if we have completed.
                    if (_expectedSampleBufferPos > _sampleBufferPos)
                        stop();
                    break;
                }
            }

            samples -= max_copy;
        }
    }

    bool WAV::renew_ima_adpcm_mono_file_buffer() {
        if (ftell(_stream) >= _dataEnd) {
            if (_loops == 0) {
                return true;
            }
            else if (_loops > 0) {
                _loops--;
            }
            fseek(_stream, _dataStart, SEEK_SET);
            _sourceBufferPos = 0;
        }

        u32 maxReadSize = _dataEnd - ftell(_stream);
        if (maxReadSize >= kAudioBuffer * 2)
            maxReadSize = kAudioBuffer * 2;
        fread(_fileBuffer, maxReadSize, 1, _stream);

        u8 channels = 1;
        u32 samplesPerBlock = (_blockAlign * 2) / channels - 7;

        _fileBufferSamplePos = 0;
        _fileBufferSampleEnd = (maxReadSize * samplesPerBlock) / (u32)_blockAlign;
        nocashMessage(("SAMPLES " + std::to_string(samplesPerBlock) + " BLOCK ALIGN " +
        std::to_string(_blockAlign) + " MAX READ SIZE " + std::to_string(maxReadSize)
        + " FILE END " + std::to_string(_fileBufferSampleEnd)).c_str());
        return false;
    }

    SoundFormat WAV::getAllocFormat() {
        if (_format == SoundFormat_ADPCM)
            return SoundFormat_16Bit;
        return _format;
    }
}