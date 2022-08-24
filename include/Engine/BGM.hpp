//
// Created by cervi on 23/08/2022.
//

#ifndef LAYTON_BGM_HPP
#define LAYTON_BGM_HPP

#include <stdio.h>
#include <string.h>
#define ARM9
#include <nds.h>
#include <maxmod9.h>

namespace BGM {
    class WAV {
    public:
        int loadWAV(FILE* f);
        uint16_t getSampleRate() const { return sampleRate; }
        bool getLoaded() const { return loaded; }
        FILE* getStream() const { return stream; }
        bool getStereo() const { return stereo; }
        uint16_t getBitsPerSample() const { return bitsPerSample; }
        uint32_t getDataEnd() const { return dataEnd; }
        void free_();
    private:
        bool loaded = false;
        uint16_t sampleRate = 0;
        bool stereo = false;
        uint16_t bitsPerSample = 8;
        FILE* stream = nullptr;
        uint32_t dataEnd = 0;
    };

    void playWAV(WAV& wav);
    void stopWAV();
    mm_word fillWAV(mm_word length, mm_addr dest, mm_stream_formats format);

    extern WAV* currentlyPlayingWav;
    extern FILE* currentStream;
    extern uint32_t currentDataEnd;
    extern bool currentStereo;
    extern bool shouldClose;
}

#endif //LAYTON_BGM_HPP
