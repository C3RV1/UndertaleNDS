//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_AUDIO_HPP
#define UNDERTALE_AUDIO_HPP

#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include <memory>
#include <utility>
#include <nds.h>
#include <maxmod9.h>

namespace Audio {
    // We do not read a sample at a time, that would take too long. We load in chunks.
    // Size of the buffer:
    constexpr int kWAVBuffer = 1000;  // in samples

    class WAV {
    public:
        void loadWAV(std::string name);
        ~WAV();
        
        std::string getFilename() {return _filename;}
        bool getLoaded() const { return _loaded; }
        void setLoops(int loops) { _loops = loops; }
        bool getStereo() const { return _stereo; }

        bool getActive() const {return _active;}
        void play();
        void stop();

        // Sometimes we'll want to start a WAV without
        // having to keep a reference.
        // We want it to free itself once it finishes playing
        // Then we allocate it in the heap as a shared_ptr and
        // set the selfFreeingPtr variable, which will be freed once it stops playing.
        inline void freeOnStop(std::shared_ptr<WAV> self) {
            selfFreeingPtr = std::move(self);
        }
    private:
        std::shared_ptr<WAV> selfFreeingPtr = nullptr;
        void free_();
        std::string _filename;
        int _loops = 0;
        bool _loaded = false;
        u16 _sampleRate = 0;
        bool _stereo = false;
        u16 _bitsPerSample = 8;
        FILE* _stream = nullptr;  // TODO: Turn to unique_ptr with delete handler, then free() not needed
        u32 _dataEnd = 0;
        u32 _dataStart = 0;

        u32 _co = 44100;  // Used to linearly convert sample rate
        u16 _cValueIdx = kWAVBuffer;
        u16 _maxValueIdx = kWAVBuffer;
        u16 _values[kWAVBuffer * 2] = {0};
        bool _active = false;
    public:
        friend mm_word fillAudioStream(mm_word, mm_addr, mm_stream_formats);
        friend bool fillAudioStreamWav(WAV*, mm_word, u16*, mm_stream_formats);
    };

    void initAudioStream();
    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format);
    bool fillAudioStreamWav(WAV* wav, mm_word length, u16* dest, mm_stream_formats format);

    void playBGMusic(std::string filename, bool loop);
    void stopBGMusic();

    extern WAV cBGMusic;

    extern std::list<WAV*> wavPlaying;
}

#endif //UNDERTALE_AUDIO_HPP
