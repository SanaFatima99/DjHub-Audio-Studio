#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cstring>
#pragma comment(lib, "Winmm.lib")
using namespace std;

// WAV file header structure — matches the standard WAV format layout
typedef struct {
    char           RIFF[4];
    unsigned long  ChunkSize;
    char           WAVE[4];
    char           fmt[4];
    unsigned long  Subchunk1Size;
    unsigned short AudioFormat;   // 1=PCM, 3=Float
    unsigned short NumOfChan;
    unsigned long  SamplesPerSec;
    unsigned long  bytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char           Subchunk2ID[4];
    unsigned long  Subchunk2Size;
} wav_hdr;

// Builds an 8-bit mono WAV header for writing output files. 
void constructHeader(wav_hdr& h, int len, int rate = 8000) {
    h.RIFF[0] = 'R'; h.RIFF[1] = 'I'; h.RIFF[2] = 'F'; h.RIFF[3] = 'F';
    h.ChunkSize = len + sizeof(h) - 8;
    h.WAVE[0] = 'W'; h.WAVE[1] = 'A'; h.WAVE[2] = 'V'; h.WAVE[3] = 'E';
    h.fmt[0] = 'f';  h.fmt[1] = 'm';  h.fmt[2] = 't';  h.fmt[3] = ' ';
    h.Subchunk1Size = 16;
    h.AudioFormat = 1;
    h.NumOfChan = 1;
    h.SamplesPerSec = rate;
    h.bytesPerSec = rate;
    h.blockAlign = 1;
    h.bitsPerSample = 8;
    h.Subchunk2ID[0] = 'd'; h.Subchunk2ID[1] = 'a';
    h.Subchunk2ID[2] = 't'; h.Subchunk2ID[3] = 'a';
    h.Subchunk2Size = len;
}

// Reads any WAV file and converts it to 8-bit mono automatically.
// I added support for 8, 16, 24, and 32-bit formats because song.wav
// and just because of this mistake in song.wav file ,I tried millionth times to fit it in 8-bit moto 
// and now 32-bit float stereo and was loading as 0 samples without this.
bool readWavFile(char* fileName, unsigned char data[], int& length, int& samplingRate) {
    FILE* f = fopen(fileName, "rb"); // binary mode — must use "rb" not "r"
    if (!f) return false;

    wav_hdr h;
    fread(&h, sizeof(h), 1, f);

    int channels = h.NumOfChan;
    int bits = h.bitsPerSample;
    int rawSize = (int)h.Subchunk2Size;
    int bytesPerSample = bits / 8;

    cout << "  File: " << fileName << endl;
    cout << "  Bits=" << bits << "  Channels=" << channels << "  Hz=" << h.SamplesPerSec << endl;

    unsigned char* raw = new unsigned char[rawSize];
    fread(raw, 1, rawSize, f);
    fclose(f);

    samplingRate = h.SamplesPerSec;
    int totalSamples = rawSize / (bytesPerSample * channels);
    length = totalSamples;

    for (int i = 0; i < totalSamples; i++) {
        int mixed = 0;
        for (int ch = 0; ch < channels; ch++) {
            int idx = (i * channels + ch) * bytesPerSample;

            if (bits == 8) {
                mixed += (int)raw[idx];
            }
            else if (bits == 16) {
                short s16 = (short)(raw[idx] | (raw[idx + 1] << 8));
                mixed += (int)(s16 / 256) + 128;
            }
            else if (bits == 24) {
                int s24 = raw[idx] | (raw[idx + 1] << 8) | (raw[idx + 2] << 16);
                if (s24 & 0x800000) s24 |= ~0xFFFFFF;
                mixed += (s24 / 65536) + 128;
            }
            else if (bits == 32) {
                // song.wav is 32-bit float stereo — memcpy safely reads the float bytes
                float fval;
                memcpy(&fval, &raw[idx], 4);
                if (fval > 1.0f) fval = 1.0f;
                if (fval < -1.0f) fval = -1.0f;
                int val = (int)(fval * 127.0f) + 128;
                val = (val > 255) ? 255 : (val < 0) ? 0 : val;
                mixed += val;
            }
        }
        // Average channels to convert stereo to mono
        data[i] = (unsigned char)(mixed / channels);
    }

    delete[] raw;
    cout << "  >> Loaded " << totalSamples << " samples!" << endl;
    return true;
}

// Writes processed audio data back as an 8-bit mono WAV file
bool writeWavFile(char* fileName, unsigned char data[], int length, int samplingRate) {
    FILE* f = fopen(fileName, "wb"); // binary write mode
    if (!f) return false;
    wav_hdr h;
    constructHeader(h, length, samplingRate);
    fwrite(&h, sizeof(h), 1, f);
    fwrite(data, 1, length, f);
    fclose(f);
    return true;
}

// Plays a WAV file using full path — I use PlaySoundA with the full directory
// path because relative paths were not being picked up correctly by Windows API
bool playWavFile(char* fileName) {
    FILE* f = fopen(fileName, "rb");
    if (!f) return false;
    fclose(f);

    char fullPath[500];
    GetCurrentDirectoryA(500, fullPath);
    strcat(fullPath, "\\");
    strcat(fullPath, fileName);

    bool stop = false;
    cout << "  Now Playing: " << fullPath << endl;
    PlaySoundA(fullPath, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    cout << "  Enter 1 to stop: ";
    while (!stop) cin >> stop;
    PlaySoundA(NULL, 0, 0);
    cout << "  Stopped!" << endl;
    return true;
}