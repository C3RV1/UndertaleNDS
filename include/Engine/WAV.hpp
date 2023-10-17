//
// Created by cervi on 10/10/2023.
//

#ifndef UNDERTALE_WAV_HPP
#define UNDERTALE_WAV_HPP

#include "Engine/Audio.hpp"

namespace Audio2 {
    class ADPCM_Decoder {
    public:
        void initPredictor(int new_sample, int index);
        s16 decodeSample(u8 inputNibble);

    private:
        const int kIndexTable[16] = {
            -1, -1, -1, -1, 2, 4, 6, 8,
            -1, -1, -1, -1, 2, 4, 6, 8
        };
        const int kStepSizeTable[89] = {
            7, 8, 9, 10, 11, 12, 13, 14,
            16, 17, 19, 21, 23, 25, 28,
            31, 34, 37, 41, 45, 50, 55,
            60, 66, 73, 80, 88, 97, 107,
            118, 130, 143, 157, 173, 190, 209,
            230, 253, 279, 307, 337, 371, 408,
            449, 494, 544, 598, 658, 724, 796,
            876, 963, 1060, 1166, 1282, 1411, 1552,
            1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
            4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
            9493, 10442, 11487, 12635, 13899, 15289, 16818,
            18500, 20350, 22385, 24623, 27086, 29794, 32767
        };

        int _index = 0;
        int _new_sample = 0;
    };

    class WAV : public AudioFile {
    public:
        void load(const std::string& name) override;
        ~WAV() override;
    protected:
        void resetPlaying() override;
        SoundFormat getAllocFormat() override;
        u8 getBitsPerSample() override;
    private:
        void progress(u16 samples) override;

        void progress_pcm8_mono(u16 samples);
        bool renew_pcm8_mono_file_buffer();

        void progress_pcm8_stereo(u16 samples);
        bool renew_pcm8_stereo_file_buffer();

        void progress_pcm16_mono(u16 samples);
        bool renew_pcm16_mono_file_buffer();

        void progress_pcm16_stereo(u16 samples);
        bool renew_pcm16_stereo_file_buffer();

        // TODO: Implement IMA_ADPCM (and other codecs?)
        void progress_ima_adpcm_mono(u16 samples);
        bool renew_ima_adpcm_mono_file_buffer();

        void progress_ima_adpcm_stereo(u16 samples);
        bool renew_ima_adpcm_stereo_file_buffer();

        u32 ima_adpcm_get_sample_position(u32 sampleIdx, u8 channelIdx,
                                          bool& highBits, bool& readHeader);

        /**
         * Frees the playing _stream. Called on the destructor and when loading,
         * to ensure that no file is left open.
         */
        void free_();

        FILE* _stream = nullptr;  // TODO: Turn to unique_ptr with delete handler, then free() not needed

        u32 _dataEnd = 0;
        u32 _dataStart = 0;
        u16 _blockAlign = 0;

        u8 _fileBuffer[kAudioBuffer * 2];
        u32 _sourceBufferPos = 0;  // TODO: Cleanup.
        u32 _fileBufferSamplePos;
        u32 _fileBufferSampleEnd;

        SoundFormat _format;
        u8 _bitsPerSample;

        // TODO: Implement channels as variable length?
        ADPCM_Decoder _leftChannelDecoder;
        ADPCM_Decoder _rightChannelDecoder;
    };

    void playBGMusic(const std::string& filename, bool loop);
    void stopBGMusic();

    extern WAV cBGMusic;
}

#endif //UNDERTALE_WAV_HPP
