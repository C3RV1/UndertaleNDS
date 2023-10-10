//
// Created by cervi on 10/10/2023.
//

#ifndef UNDERTALE_WAV_HPP
#define UNDERTALE_WAV_HPP

#include "Engine/Audio.hpp"

namespace Audio2 {
    class WAV : public AudioFile {
    public:
        void load(const std::string& name) override;
        ~WAV() override;
    protected:
        void resetPlaying() override;
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
        // void progress_ima_adpcm_mono(u16 samples);
        // void progress_ima_adpcm_stereo(u16 samples);

        std::shared_ptr<WAV> selfFreeingPtr = nullptr;
        void free_();

        FILE* _stream = nullptr;  // TODO: Turn to unique_ptr with delete handler, then free() not needed

        u32 _dataEnd = 0;
        u32 _dataStart = 0;

        u8 _fileBuffer[kAudioBuffer * 2];
        u32 _fileBufferSamplePos;
        u32 _fileBufferSampleEnd;
    };

    void playBGMusic(const std::string& filename, bool loop);
    void stopBGMusic();

    extern WAV cBGMusic;
}

#endif //UNDERTALE_WAV_HPP
