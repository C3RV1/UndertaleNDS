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
    const int kWAVBuffer = 1000;  // in samples

    class WAV {
    public:
        int loadWAV(const char* name);
        void free_();
        
        char* getFilename() {return _filename;}
        bool getLoaded() const { return _loaded; }
        void setLoops(int loops) { _loops = loops; }
        bool getStereo() const { return _stereo; }

        bool getActive() const {return _active;}
        void play();
        void stop();

        // Sometimes we'll want to start a WAV without
        // having to keep a reference.
        // We want it to free itself once it finishes playing
        // Then we allocate it in the heap using new and
        // set tne following variable to true.
        bool deleteOnStop = false;
    private:
        char* _filename = nullptr;
        int _loops = 0;
        bool _loaded = false;
        u16 _sampleRate = 0;
        bool _stereo = false;
        u16 _bitsPerSample = 8;
        FILE* _stream = nullptr;
        u32 _dataEnd = 0;
        u32 _dataStart = 0;

        u32 _co = 44100;  // Used to linearly convert sample rate
        u16 _cValueIdx = kWAVBuffer;
        u16 _maxValueIdx = kWAVBuffer;
        u16 _values[kWAVBuffer * 2] = {0};
        bool _active = false;
        WAV* _prev = nullptr;
        WAV* _next = nullptr;
    public:
        friend mm_word fillAudioStream(mm_word, mm_addr, mm_stream_formats);
        friend bool fillAudioStreamWav(WAV*, mm_word, u16*, mm_stream_formats);
    };

    void initAudioStream();
    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format);
    bool fillAudioStreamWav(WAV* wav, mm_word length, u16* dest, mm_stream_formats format);

    void playBGMusic(const char* filename, bool loop);
    void stopBGMusic();

    extern WAV cBGMusic;

    extern WAV* playingWavHead;
}

#endif //UNDERTALE_AUDIO_HPP
