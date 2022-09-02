//
// Created by cervi on 23/08/2022.
//
#include "Engine/BGM.hpp"

namespace BGM {
    WAV* currentlyPlayingWav = nullptr;
    FILE* currentStream = nullptr;
    uint32_t currentDataEnd = 0;
    uint32_t currentDataStart = 0;
    bool currentLoop = false;
    bool shouldClose = false;
    WAV globalWAV;

    int WAV::loadWAV(const char *name) {
        free_();
        loop = false;
        char buffer[100];
        sprintf(buffer, "nitro:/z_audio/%s", name);
        FILE *f = fopen(buffer, "rb");
        filename = new char[strlen(name) + 1];
        strcpy(filename, name);
        if (!f)
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

        uint16_t format, channels;
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
        if (stream == currentStream)
            stopWAV();
        delete[] filename;
        filename = nullptr;
        fclose(stream);
        stream = nullptr;
        loaded = false;
    }

    void playWAV(WAV& wav) {
        stopWAV();
        if (!wav.getLoaded())
            return;

        currentlyPlayingWav = &wav;
        currentStream = wav.getStream();
        currentDataEnd = wav.getDataEnd();
        currentDataStart = wav.getDataStart();
        currentLoop = wav.getLoop();
        shouldClose = false;

        mm_stream stream;

        stream.sampling_rate = wav.getSampleRate();
        stream.buffer_length = 8000;  // 1 seconds buffer
        stream.callback = fillWAV;   // give wav filling routine
        uint16_t format;
        if (wav.getBitsPerSample() == 8) {
            format = MM_STREAM_8BIT_MONO;
        } else {
            format = MM_STREAM_16BIT_MONO;
        }
        if (wav.getStereo()) {
            format |= 1;
        }
        stream.format = format; // select format
        stream.timer = MM_TIMER0;             // use timer0
        stream.manual = 1;                    // auto filling

        // open the stream
        mmStreamOpen(&stream);
    }

    void stopWAV() {
        if (currentlyPlayingWav == nullptr)
            return;
        mmStreamClose();
        currentlyPlayingWav = nullptr;
    }

    mm_word fillWAV(mm_word length, mm_addr dest, mm_stream_formats format) {
        if (currentlyPlayingWav == nullptr)
            return 0;
        FILE* stream = currentStream;
        int readLength = 1;
        if (format & 2)
            readLength = 2;
        if (format & 1)
            readLength *= 2;
        uint32_t maxLength = (currentDataEnd - ftell(stream)) / readLength;
        if (maxLength == 0) {
            if (!currentLoop) {
                shouldClose = true;
                return 0;
            }
            else {
                fseek(currentStream, currentDataStart, SEEK_SET);
                return fillWAV(length, dest, format);
            }
        }
        if (length > maxLength)
            length = maxLength;
        fread(dest, readLength, length, stream);
        return length;
    }
}
