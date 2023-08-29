//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "Engine/Engine.hpp"
#include "DEBUG_FLAGS.hpp"
#include <algorithm>
#include <utility>

namespace Audio {
    WAV cBGMusic;

    std::list<WAV*> wavPlaying;

    void WAV::loadWAV(const std::string& name) {
        free_();
        if (name.empty())  // Loading an empty WAV is the same as freeing it
            return;
        _loops = 0;
        std::string realPath = "nitro:/z_audio/" + name;
        FILE *f = fopen(realPath.c_str(), "rb");
        _filename = name;
        if (f == nullptr) {
            std::string buffer = "Error opening WAV #r" + name;
            Engine::throw_(buffer);
        }

        char header[4];

        const char riffHeader[4] = {'R', 'I', 'F', 'F'};
        const char waveHeader[4] = {'W', 'A', 'V', 'E'};
        const char fmtHeader[4] = {'f', 'm', 't', ' '};
        const char dataHeader[4] = {'d', 'a', 't', 'a'};

        fread(header, 4, 1, f);
        if (memcmp(header, riffHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                    "#x: Invalid RIFF header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        u32 fileSize;
        fread(&fileSize, 4, 1, f);

        fread(header, 4, 1, f);
        if (memcmp(header, waveHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid WAVE header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        // fmt header
        fread(header, 4, 1, f);
        if (memcmp(header, fmtHeader, 4) != 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid FMT header.";
            fclose(f);
            Engine::throw_(buffer);
        }

        fseek(f, ftell(f) + 4, SEEK_SET); // skip chunk size == 0x10

        u16 format, channels;
        fread(&format, 2, 1, f);
        fread(&channels, 2, 1, f);
        fread(&_sampleRate, 4, 1, f);
        fseek(f, ftell(f) + 4, SEEK_SET); // skip byte rate == self.sample_rate * self.bits_per_sample * self.num_channels // 8
        fseek(f, ftell(f) + 2, SEEK_SET); // skip block align == self.num_channels * self.bits_per_sample // 8
        fread(&_bitsPerSample, 2, 1, f);

        if (format != 1) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid format.";
            fclose(f);
            Engine::throw_(buffer);
        }

        if (channels > 2) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Invalid channels.";
            fclose(f);
            Engine::throw_(buffer);
        }

        _stereo = channels == 2;

        u32 chunkSize = 0;
        // data chunk
        while (ftell(f) < fileSize + 8) {
            fread(header, 4, 1, f);
            fread(&chunkSize, 4, 1, f);
            if (memcmp(header, dataHeader, 4) == 0)
                break;
            fseek(f, chunkSize, SEEK_CUR);
        }
        if (chunkSize == 0) {
            std::string buffer = "Error opening WAV #r" + name +
                                 "#x: Couldn't find DATA chunk.";
            fclose(f);
            Engine::throw_(buffer);
        }

        _dataEnd = ftell(f) + chunkSize;
        _dataStart = ftell(f);

        _loaded = true;
        _stream = f;
    }

    void WAV::free_() {
        if (!_loaded)
            return;
        _loaded = false;
        stop();
#ifdef DEBUG_AUDIO
        std::string buffer = "Freeing WAV stream " + _filename;
        nocashMessage(buffer.c_str());
#endif
        fclose(_stream);
        _stream = nullptr;
    }

    void initAudioStream() {
        mm_stream stream;

        stream.sampling_rate = 44100;
        stream.buffer_length = 8000;
        stream.callback = fillAudioStream;
        stream.format = MM_STREAM_16BIT_STEREO;
        stream.timer = MM_TIMER0;
        stream.manual = 1;

        mmStreamOpen(&stream);
    }

    void WAV::play() {
        if (!_loaded) {
            return;
        }
        if (_active) {
            stop();
        }
        fseek(_stream, _dataStart, SEEK_SET);
        _active = true;
        _co = 44100;
        _maxValueIdx = kWAVBuffer;
        _cValueIdx = kWAVBuffer;
        wavPlaying.push_front(this);
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Starting wav: %s stereo %d sample rate %d", getFilename().c_str(),
                getStereo(), _sampleRate);
        nocashMessage(buffer);
#endif
    }

    void WAV::stop() {
        if (!_active)
            return;
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Stopping wav: %s", getFilename().c_str());
        nocashMessage(buffer);
#endif
        _active = false;
        auto idx = std::find(wavPlaying.begin(), wavPlaying.end(), this);
        if (idx != wavPlaying.end())
            wavPlaying.erase(idx);
        // If the reference is no longer held by any shared ptr
        // this will delete the object
        selfFreeingPtr = nullptr;
    }

    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format) {
        auto current = wavPlaying.begin();
        dmaFillWords(0, dest, 4 * length);
        while (current != wavPlaying.end()) {
            WAV* currentWAV = *(current++);
            if (fillAudioStreamWav(currentWAV, length, (u16*)dest, format)) {
                currentWAV->stop();
            }
        }
        return length;
    }

    bool fillAudioStreamWav(WAV* wav, mm_word length, u16* dest, mm_stream_formats) {
        if (wav == nullptr)
            return true;
        if (!wav->_active)
            return true;
        if (!wav->_loaded)
            return true;
        FILE* stream = wav->_stream;
        // TODO: convert bit depth
        if (wav->_bitsPerSample != 16)
            return true;
        // TODO: Document how sample rate change works
        u32 dstI = 0;
        // TODO: s32 addition; to clip

        while (dstI < length) {
            while (wav->_co >= 44100) {
                wav->_cValueIdx += 1;
                if (wav->_cValueIdx >= wav->_maxValueIdx) {
                    if ((u32)ftell(stream) >= wav->_dataEnd) {
                        if (wav->_loops != 0) {
                            if (wav->_loops > 0)
                                wav->_loops--;
                            fseek(stream, wav->_dataStart, SEEK_SET);
#ifdef DEBUG_AUDIO
                            nocashMessage("looping");
#endif
                        }
                        else {
                            return true;
                        }
                    }
                    long readElements = (wav->_dataEnd - ftell(stream)) / 2;
                    if (wav->_stereo)
                        readElements /= 2;
                    if (readElements > kWAVBuffer)
                        readElements = kWAVBuffer;
                    if (!wav->_stereo)
                        wav->_maxValueIdx = fread(&wav->_values, 2, readElements, stream);
                    else
                        wav->_maxValueIdx = fread(&wav->_values, 4, readElements, stream);
                    wav->_cValueIdx = 0;
                }
                wav->_co -= 44100;
            }
            for (int i = 0; i < 2; i++) {
                if (!wav->_stereo)
                    dest[dstI * 2 + i] += wav->_values[wav->_cValueIdx];
                else
                    dest[dstI * 2 + i] += wav->_values[wav->_cValueIdx * 2 + i];
            }
            wav->_co += wav->_sampleRate;
            dstI++;
        }
        return false;
    }

    void playBGMusic(std::string filename, bool loop) {
        stopBGMusic();
        cBGMusic.loadWAV(std::move(filename));
        cBGMusic.setLoops(loop ? -1 : 0);
        if (cBGMusic.getLoaded())
            cBGMusic.play();
    }

    void stopBGMusic() {
        cBGMusic.stop();
    }

    WAV::~WAV() noexcept {
#ifdef DEBUG_AUDIO
        std::string buffer = "Called destructor of WAV " + _filename;
        nocashMessage(buffer.c_str());
#endif
        free_();
    }
}
