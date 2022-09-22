//
// Created by cervi on 23/08/2022.
//
#include "Engine/BGM.hpp"
#include <errno.h>

namespace BGM {
    WAV currentBGMusic;

    WAV* playingWavs = nullptr;

    int WAV::loadWAV(const char *name) {
        free_();
        loops = 0;
        char buffer[100];
        sprintf(buffer, "nitro:/z_audio/%s", name);
        FILE *f = fopen(buffer, "rb");
        filename = new char[strlen(name) + 1];
        strcpy(filename, name);
        if (f == nullptr) {
            nocashMessage("1");
            return 1;
        }
        stream = f;

        char header[4];

        const char riffHeader[4] = {'R', 'I', 'F', 'F'};
        const char waveHeader[4] = {'W', 'A', 'V', 'E'};
        const char fmtHeader[4] = {'f', 'm', 't', ' '};
        const char dataHeader[4] = {'d', 'a', 't', 'a'};

        fread(header, 4, 1, f);
        if (memcmp(header, riffHeader, 4) != 0) {
            fclose(f);
            sprintf(buffer, "%X %X %X %X", header[0], header[1], header[2], header[3]);
            nocashMessage(buffer);
            return 2;
        }

        fseek(f, ftell(f) + 4, SEEK_SET); // skip chunk size

        fread(header, 4, 1, f);
        if (memcmp(header, waveHeader, 4) != 0) {
            fclose(f);
            nocashMessage("3");
            return 3;
        }

        // fmt header
        fread(header, 4, 1, f);
        if (memcmp(header, fmtHeader, 4) != 0) {
            fclose(f);
            nocashMessage("4");
            return 4;
        }

        fseek(f, ftell(f) + 4, SEEK_SET); // skip chunk size == 0x10

        uint16_t format, channels;
        fread(&format, 2, 1, f);
        fread(&channels, 2, 1, f);
        fread(&sampleRate, 4, 1, f);
        fseek(f, ftell(f) + 4, SEEK_SET); // skip byte rate == self.sample_rate * self.bits_per_sample * self.num_channels // 8
        fseek(f, ftell(f) + 2, SEEK_SET); // skip block align == self.num_channels * self.bits_per_sample // 8
        fread(&bitsPerSample, 2, 1, f);

        if (format != 1) {
            fclose(f);
            nocashMessage("5");
            return 5;
        }

        if (channels > 2) {
            nocashMessage("6");
            return 6;
        }

        stereo = channels == 2;

        // data chunk
        fread(header, 4, 1, f);
        if (memcmp(header, dataHeader, 4) != 0) {
            fclose(f);
            nocashMessage("7");
            return 7;
        }

        uint32_t chunkSize;
        fread(&chunkSize, 4, 1, f);
        dataEnd = ftell(f) + chunkSize;
        dataStart = ftell(f);

        loaded = true;

        return 0;
    }

    void WAV::free_() {
        if (!loaded)
            return;
        delete[] filename;
        filename = nullptr;
        fclose(stream);
        stream = nullptr;
        loaded = false;
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
        if (!loaded) {
            return;
        }
        if (active) {
            stop();
        }
        fseek(stream, dataStart, SEEK_SET);
        active = true;
        co = 44100;
        maxValueIdx = WAVBuffer;
        cValueIdx = WAVBuffer;
        nextWav = playingWavs;
        if (playingWavs != nullptr)
            playingWavs->prevWav = this;
        playingWavs = this;
        char buffer[100];
        sprintf(buffer, "Starting wav: %s stereo %d sample rate %d", getFilename(),
                getStereo(), getSampleRate());
        nocashMessage(buffer);
    }

    void WAV::stop() {
        if (!active)
            return;
        char buffer[100];
        sprintf(buffer, "Stopping wav: %s", getFilename());
        nocashMessage(buffer);
        active = false;
        if (prevWav != nullptr)
            prevWav->nextWav = nextWav;
        else
            playingWavs = nextWav;
        if (nextWav != nullptr)
            nextWav->prevWav = prevWav;
        if (deleteOnStop) {
            free_();
            delete this;
        }
    }

    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format) {
        WAV* current = playingWavs;
        memset(dest, 0, 4 * length);
        while (current != nullptr) {
            WAV* next = current->nextWav;
            if (fillAudioStreamWav(current, length, (uint16_t*)dest, format)) {
                current->stop();
            }
            current = next;
        }
        return length;
    }

    bool fillAudioStreamWav(WAV* wav, mm_word length, uint16_t* dest, mm_stream_formats format) {
        if (wav == nullptr)
            return true;
        if (!wav->active)
            return true;
        if (!wav->getLoaded())
            return true;
        FILE* stream = wav->getStream();
        // do not convert bit depth for now
        if (wav->getBitsPerSample() != 16)
            return true;
        // convert channels & sample rate
        uint32_t dstI = 0;
        int32_t addition;

        while (dstI < length) {
            while (wav->co >= 44100) {
                wav->cValueIdx += 1;
                if (wav->cValueIdx >= wav->maxValueIdx) {
                    if (ftell(stream) >= wav->getDataEnd()) {
                        if (wav->getLoops() != 0) {
                            if (wav->getLoops() > 0)
                                wav->setLoops(wav->getLoops() - 1);
                            fseek(stream, wav->getDataStart(), SEEK_SET);
                            nocashMessage("looping");
                        }
                        else {
                            return true;
                        }
                    }
                    if (!wav->getStereo())
                        wav->maxValueIdx = fread(&wav->values, 2, WAVBuffer, stream) / 4;
                    else
                        wav->maxValueIdx = fread(&wav->values, 2, WAVBuffer * 2, stream) / 2;
                    wav->cValueIdx = 0;
                }
                wav->co -= 44100;
            }
            for (int i = 0; i < 2; i++) {
                if (!wav->getStereo())
                    addition = dest[dstI * 2 + i] + wav->values[wav->cValueIdx];
                else
                    addition = dest[dstI * 2 + i] + wav->values[wav->cValueIdx * 2 + i];
                // clamp to int16_t
                if (addition > 32767)
                    addition = 32767;
                else if (addition < -32768)
                    addition = -32768;
                dest[dstI * 2 + i] = addition;
            }
            wav->co += wav->getSampleRate();
            dstI++;
        }
        return false;
    }

    void playBGMusic(const char* filename, bool loop) {
        stopBGMusic();
        currentBGMusic.loadWAV(filename);
        currentBGMusic.setLoops(loop ? -1 : 0);
        currentBGMusic.deleteOnStop = false;
        if (currentBGMusic.getLoaded())
            currentBGMusic.play();
        else {
            currentBGMusic.free_();
        }
    }

    void stopBGMusic() {
        currentBGMusic.stop();
        currentBGMusic.free_();
    }
}
