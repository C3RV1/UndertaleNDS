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
    class AudioManager;

    constexpr int kAudioBuffer = 2000;  // in samples

    class AudioFile {
    public:
        virtual void load(const std::string& name) = 0;
        void play();
        void stop();
        virtual ~AudioFile() = default;

        std::string getFilename() {return _filename;}
        void setLoops(int loops) {_loops = loops;}
        bool getStereo() const {return _stereo;}
        bool getPlaying() const {return _active;}
        bool getLoaded() const {return _loaded;}

        // Sometimes we'll want to start a WAV without
        // having to keep a reference.
        // We want it to free itself once it finishes playing
        // Then we allocate it in the heap as a shared_ptr and
        // set the selfFreeingPtr variable, which will be freed once it stops playing.
        inline void freeOnStop(std::shared_ptr<AudioFile> self) {
            selfFreeingPtr = std::move(self);
        }
    protected:
        virtual void allocateBuffers();
        virtual void resetPlaying() = 0;

        SoundFormat _format;

        int _leftChannel = -1;
        u8* _leftBuffer = nullptr;  // When allocated has kWAVBuffer buffer size, type depends on format

        int _rightChannel = -1;
        u8* _rightBuffer = nullptr;  // When allocated has kWAVBuffer buffer size, type depends on format
        u32 _sampleBufferPos;

        void update();
        virtual void progress(u16 samples) = 0;

        std::string _filename;
        bool _active = false;
        bool _stereo = false;
        bool _loaded = false;
        int _loops = 0;
        u16 _timerLast;
        u32 _expectedSampleBufferPos;
        u16 _sampleRate = 1;
        u8 _bitsPerSample;

        std::shared_ptr<AudioFile> selfFreeingPtr = nullptr;

        friend class AudioManager;
    };

    class AudioManager {
    public:
        explicit AudioManager(int timerId);

        void update();

        u8 getTimerId() const {return _timerId;}
    private:
        void addPlaying(AudioFile* wav);
        void removePlaying(AudioFile* wav);

        std::list<AudioFile*> _playing;
        u8 _timerId;

        friend class AudioFile;
    };

    extern AudioManager audioManager;

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

#endif //UNDERTALE_AUDIO_HPP
