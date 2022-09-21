//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_BGM_HPP
#define UNDERTALE_BGM_HPP

#include <stdio.h>
#include <string.h>
#define ARM9
#include <nds.h>
#include <maxmod9.h>

namespace BGM {
    const int WAVBuffer = 1000;

    class WAV {
    public:
        int loadWAV(const char* name);
        char* getFilename() {return filename;}
        uint16_t getSampleRate() const { return sampleRate; }
        bool getLoaded() const { return loaded; }
        int getLoops() const { return loops; }
        void setLoops(int loops_) { loops = loops_; }
        FILE* getStream() const { return stream; }
        bool getStereo() const { return stereo; }
        uint16_t getBitsPerSample() const { return bitsPerSample; }
        uint32_t getDataEnd() const { return dataEnd; }
        uint32_t getDataStart() const { return dataStart; }
        void free_();

        bool getActive() const {return active;}
        void play();
        void stop();
        bool deleteOnStop = false;
    private:
        char* filename = nullptr;
        int loops = 0;
        bool loaded = false;
        uint16_t sampleRate = 0;
        bool stereo = false;
        uint16_t bitsPerSample = 8;
        FILE* stream = nullptr;
        uint32_t dataEnd = 0;
        uint32_t dataStart = 0;

        uint32_t co = 44100;  // Used to linearly adjust sample rate
        uint16_t cValueIdx = WAVBuffer;
        uint16_t maxValueIdx = WAVBuffer;
        uint16_t values[WAVBuffer * 2] = {0};
        bool active = false;
        WAV* prevWav = nullptr;
        WAV* nextWav = nullptr;
    public:
        friend mm_word fillAudioStream(mm_word, mm_addr, mm_stream_formats);
        friend bool fillAudioStreamWav(WAV*, mm_word, uint16_t*, mm_stream_formats);
    };

    void initAudioStream();
    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format);
    bool fillAudioStreamWav(WAV* wav, mm_word length, uint16_t* dest, mm_stream_formats format);

    void playBGMusic(const char* filename, bool loop);
    void stopBGMusic();

    extern WAV currentBGMusic;

    extern WAV* playingWavs;
}

#endif //UNDERTALE_BGM_HPP
