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

namespace Audio2 {
    class WAV;

    class AudioManager {
    public:
        explicit AudioManager(int timerId);

        // void setChannelLocked(int channel, bool locked);
        // bool isLocked(int channel);

        void update();

        u8 getTimerId() const {return _timerId;}
    private:
        int getFreeChannel();
        void freeChannel(int channel);

        void addPlayingWAV(WAV* wav);
        void removePlayingWAV(WAV* wav);

        bool _enabledChannels[16];
        bool _activeChannels[16];
        std::list<WAV*> _wavPlaying;
        u8 _timerId;

        friend class WAV;
    };

    extern AudioManager audioManager;

    constexpr int kWAVBuffer = 2000;  // in samples

    class WAV {
    public:
        void loadWAV(const std::string& name);
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
        void update();

        void progress(u16 samples);

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

        std::string _filename;

        int _loops = 0;
        bool _loaded = false;

        FILE* _stream = nullptr;  // TODO: Turn to unique_ptr with delete handler, then free() not needed
        SoundFormat _format;
        u16 _sampleRate = 0;
        bool _stereo = false;

        u32 _dataEnd = 0;
        u32 _dataStart = 0;
        u8 _fileBuffer[kWAVBuffer * 2];
        u32 _fileBufferSamplePos;
        u32 _fileBufferSampleEnd;

        int _leftChannel = -1;
        u8* _leftBuffer = nullptr;  // When allocated has kWAVBuffer buffer size, type depends on format

        int _rightChannel = -1;
        u8* _rightBuffer = nullptr;  // When allocated has kWAVBuffer buffer size, type depends on format
        u32 _sampleBufferPos;
        u32 _expectedSampleBufferPos;

        u16 _timerLast;

        bool _active = false;

        friend class AudioManager;
    };

    void playBGMusic(const std::string& filename, bool loop);
    void stopBGMusic();

    extern WAV cBGMusic;
}

#endif //UNDERTALE_AUDIO_HPP
