//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_AUDIO_HPP
#define UNDERTALE_AUDIO_HPP

#include <cstdio>
#include <cstring>
#define ARM9
#include <nds.h>
#include <maxmod9.h>

namespace Audio {
    // We do not read a sample at a time, that would take too long. We load in chunks.
    // Size of the buffer:
    const int WAVBuffer = 1000;  // in samples

    class WAV {
    public:
        int loadWAV(const char* name);
        void free_();
        
        char* getFilename() {return filename;}
        bool getLoaded() const { return loaded; }
        void setLoops(int loops_) { loops = loops_; }
        bool getStereo() const { return stereo; }

        bool getActive() const {return active;}
        void play();
        void stop();

        // Sometimes we'll want to start a WAV without
        // having to keep a reference.
        // We want it to free itself once it finishes playing
        // Then we allocate it in the heap using new and
        // set tne following variable to true.
        bool deleteOnStop = false;
    private:
        char* filename = nullptr;
        int loops = 0;
        bool loaded = false;
        u16 sampleRate = 0;
        bool stereo = false;
        u16 bitsPerSample = 8;
        FILE* stream = nullptr;
        u32 dataEnd = 0;
        u32 dataStart = 0;

        u32 co = 44100;  // Used to linearly convert sample rate
        u16 cValueIdx = WAVBuffer;
        u16 maxValueIdx = WAVBuffer;
        u16 values[WAVBuffer * 2] = {0};
        bool active = false;
        WAV* prevWav = nullptr;
        WAV* nextWav = nullptr;
    public:
        friend mm_word fillAudioStream(mm_word, mm_addr, mm_stream_formats);
        friend bool fillAudioStreamWav(WAV*, mm_word, u16*, mm_stream_formats);
    };

    void initAudioStream();
    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format);
    bool fillAudioStreamWav(WAV* wav, mm_word length, u16* dest, mm_stream_formats format);

    void playBGMusic(const char* filename, bool loop);
    void stopBGMusic();

    extern WAV currentBGMusic;

    extern WAV* playingWavHead;
}

#endif //UNDERTALE_AUDIO_HPP
