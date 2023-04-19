//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "DEBUG_FLAGS.hpp"

namespace Audio {
    WAV cBGMusic;

    WAV* playingWavHead = nullptr;

    int WAV::loadWAV(const char *name) {
        free_();
        _loops = 0;
        char buffer[100];
        sprintf(buffer, "nitro:/z_audio/%s", name);
        FILE *f = fopen(buffer, "rb");
        _filename = new char[strlen(name) + 1];
        strcpy(_filename, name);
        if (f == nullptr)
            return 1;
        _stream = f;

        char header[4];

        const char riffHeader[4] = {'R', 'I', 'F', 'F'};
        const char waveHeader[4] = {'W', 'A', 'V', 'E'};
        const char fmtHeader[4] = {'f', 'm', 't', ' '};
        const char dataHeader[4] = {'d', 'a', 't', 'a'};

        fread(header, 4, 1, f);
        if (memcmp(header, riffHeader, 4) != 0) {
            fclose(f);
            return 2;
        }

        fseek(f, ftell(f) + 4, SEEK_SET); // skip chunk size

        fread(header, 4, 1, f);
        if (memcmp(header, waveHeader, 4) != 0) {
            fclose(f);
            return 3;
        }

        // fmt header
        fread(header, 4, 1, f);
        if (memcmp(header, fmtHeader, 4) != 0) {
            fclose(f);
            return 4;
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
            fclose(f);
            return 5;
        }

        if (channels > 2) {
            return 6;
        }

        _stereo = channels == 2;

        // data chunk
        fread(header, 4, 1, f);
        if (memcmp(header, dataHeader, 4) != 0) {
            fclose(f);
            return 7;
        }

        u32 chunkSize;
        fread(&chunkSize, 4, 1, f);
        _dataEnd = ftell(f) + chunkSize;
        _dataStart = ftell(f);

        _loaded = true;

        return 0;
    }

    void WAV::free_() {
        if (!_loaded)
            return;
        _loaded = false;
        stop();
        delete[] _filename;
        _filename = nullptr;
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
        _next = playingWavHead;
        if (playingWavHead != nullptr)
            playingWavHead->_prev = this;
        playingWavHead = this;
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Starting wav: %s stereo %d sample rate %d", getFilename(),
                getStereo(), getSampleRate());
        nocashMessage(buffer);
#endif
    }

    void WAV::stop() {
        if (!_active)
            return;
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Stopping wav: %s", getFilename());
        nocashMessage(buffer);
#endif
        _active = false;
        if (_prev != nullptr)
            _prev->_next = _next;
        else
            playingWavHead = _next;
        if (_next != nullptr)
            _next->_prev = _prev;
        if (deleteOnStop) {
            delete this;
        }
    }

    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format) {
        WAV* current = playingWavHead;
        memset(dest, 0, 4 * length);
        while (current != nullptr) {
            WAV* next = current->_next;
            if (fillAudioStreamWav(current, length, (u16*)dest, format)) {
                current->stop();
            }
            current = next;
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

    void playBGMusic(const char* filename, bool loop) {
        stopBGMusic();
        cBGMusic.loadWAV(filename);
        cBGMusic.setLoops(loop ? -1 : 0);
        cBGMusic.deleteOnStop = false;
        if (cBGMusic.getLoaded())
            cBGMusic.play();
    }

    void stopBGMusic() {
        cBGMusic.stop();
    }
}
