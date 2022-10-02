//
// Created by Cervi on 23/08/2022.
//
#include "Engine/Audio.hpp"
#include "DEBUG_FLAGS.hpp"

namespace Audio {
    WAV currentBGMusic;

    WAV* playingWavHead = nullptr;

    int WAV::loadWAV(const char *name) {
        free_();
        loops = 0;
        char buffer[100];
        sprintf(buffer, "nitro:/z_audio/%s", name);
        FILE *f = fopen(buffer, "rb");
        filename = new char[strlen(name) + 1];
        strcpy(filename, name);
        if (f == nullptr)
            return 1;
        stream = f;

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
        fread(&sampleRate, 4, 1, f);
        fseek(f, ftell(f) + 4, SEEK_SET); // skip byte rate == self.sample_rate * self.bits_per_sample * self.num_channels // 8
        fseek(f, ftell(f) + 2, SEEK_SET); // skip block align == self.num_channels * self.bits_per_sample // 8
        fread(&bitsPerSample, 2, 1, f);

        if (format != 1) {
            fclose(f);
            return 5;
        }

        if (channels > 2) {
            return 6;
        }

        stereo = channels == 2;

        // data chunk
        fread(header, 4, 1, f);
        if (memcmp(header, dataHeader, 4) != 0) {
            fclose(f);
            return 7;
        }

        u32 chunkSize;
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
        nextWav = playingWavHead;
        if (playingWavHead != nullptr)
            playingWavHead->prevWav = this;
        playingWavHead = this;
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Starting wav: %s stereo %d sample rate %d", getFilename(),
                getStereo(), getSampleRate());
        nocashMessage(buffer);
#endif
    }

    void WAV::stop() {
        if (!active)
            return;
#ifdef DEBUG_AUDIO
        char buffer[100];
        sprintf(buffer, "Stopping wav: %s", getFilename());
        nocashMessage(buffer);
#endif
        active = false;
        if (prevWav != nullptr)
            prevWav->nextWav = nextWav;
        else
            playingWavHead = nextWav;
        if (nextWav != nullptr)
            nextWav->prevWav = prevWav;
        if (deleteOnStop) {
            free_();
            delete this;
        }
    }

    mm_word fillAudioStream(mm_word length, mm_addr dest, mm_stream_formats format) {
        WAV* current = playingWavHead;
        memset(dest, 0, 4 * length);
        while (current != nullptr) {
            WAV* next = current->nextWav;
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
        if (!wav->active)
            return true;
        if (!wav->getLoaded())
            return true;
        FILE* stream = wav->getStream();
        // TODO: convert bit depth
        if (wav->getBitsPerSample() != 16)
            return true;
        // convert channels & sample rate
        u32 dstI = 0;
        // TODO: s32 addition; to clip

        while (dstI < length) {
            while (wav->co >= 44100) {
                wav->cValueIdx += 1;
                if (wav->cValueIdx >= wav->maxValueIdx) {
                    if ((u32)ftell(stream) >= wav->getDataEnd()) {
                        if (wav->getLoops() != 0) {
                            if (wav->getLoops() > 0)
                                wav->setLoops(wav->getLoops() - 1);
                            fseek(stream, wav->getDataStart(), SEEK_SET);
#ifdef DEBUG_AUDIO
                            nocashMessage("looping");
#endif
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
                    dest[dstI * 2 + i] += wav->values[wav->cValueIdx];
                else
                    dest[dstI * 2 + i] += wav->values[wav->cValueIdx * 2 + i];
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
