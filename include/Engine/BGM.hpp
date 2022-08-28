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
        int loadWAV(const char* name);
        char* getFilename() {return filename;}
        uint16_t getSampleRate() const { return sampleRate; }
        bool getLoaded() const { return loaded; }
        bool getLoop() const { return loop; }
        void setLoop(bool loop_) { loop = loop_; }
        FILE* getStream() const { return stream; }
        bool getStereo() const { return stereo; }
        uint16_t getBitsPerSample() const { return bitsPerSample; }
        uint32_t getDataEnd() const { return dataEnd; }
        uint32_t getDataStart() const { return dataStart; }
        void free_();
    private:
        char* filename = nullptr;
        bool loop = false;
        bool loaded = false;
        uint16_t sampleRate = 0;
        bool stereo = false;
        uint16_t bitsPerSample = 8;
        FILE* stream = nullptr;
        uint32_t dataEnd = 0;
        uint32_t dataStart = 0;
    };

    // TODO: loop (data start, fseek and parameter?)
    void playWAV(WAV& wav);
    void stopWAV();
    mm_word fillWAV(mm_word length, mm_addr dest, mm_stream_formats format);

    extern WAV* currentlyPlayingWav;
    extern FILE* currentStream;
    extern uint32_t currentDataEnd;
    extern uint32_t currentDataStart;
    extern bool currentLoop;
    extern bool shouldClose;

    extern WAV globalWAV;
}

#endif //LAYTON_BGM_HPP
