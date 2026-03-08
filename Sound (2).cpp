#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include "wavfile.h"
using namespace std;
// I set the working directory at the start of main() so I can type just
// the filename instead of the full path every time — this saved a lot of trouble.
// --- Memory Management ---
void allocateArray(unsigned char*& data, int size) {
    data = new unsigned char[size];
}
void deallocateArray(unsigned char*& data) {
    delete[] data;
    data = nullptr;} // avoids dangling pointer after free
// --- Display ---
void printArray(const unsigned char* data, int len) {
    cout << "{ ";
    for (int i = 0; i < len; i++)
        cout << (int)data[i] << (i < len - 1 ? " -> " : "");
    cout << " }" << endl;
}
// --- Up Sampling: each sample is duplicated {1,2,3} -> {1,1,2,2,3,3} ---
// Returns a new array — caller is responsible for freeing it
unsigned char* doubleArray(unsigned char* src, int n, int& outN) {
    outN = n * 2;
    unsigned char* out = new unsigned char[outN];
    for (int i = 0; i < n; i++)
        out[i * 2] = out[i * 2 + 1] = src[i];
    return out;
}
// --- Down Sampling: every alternate sample is kept {1,2,3,4} -> {1,3} ---
// Returns a new array — caller is responsible for freeing it
unsigned char* shrinkArray(unsigned char* src, int n, int& outN) {
    outN = (n + 1) / 2;
    unsigned char* out = new unsigned char[outN];
    for (int i = 0; i < outN; i++)
        out[i] = src[i * 2];
    return out;
}
// --- Moving Average Filter ---
// out[i] = mean of N neighbors on each side of index i
// Boundaries are handled by using only available neighbors
void fillWithMean(const unsigned char* in, unsigned char* out, int len, int n) {
    for (int i = 0; i < len; i++) {
        int lb = (i - n >= 0) ? i - n : 0;
        int rb = (i + n <= len - 1) ? i + n : len - 1;
        int sum = 0, cnt = 0;
        for (int j = lb; j <= rb; j++) { sum += in[j]; cnt++; }
        out[i] = (unsigned char)(sum / cnt);
    }
}
// --- File Loading ---
// I allocate a 70MB buffer so any file fits — readWavFile updates
// the length to the actual sample count after reading
bool readFromFile(const char* name, unsigned char*& data, int& bytes, int& hz) {
    allocateArray(data, 70000000);
    bytes = 70000000;
    bool ok = readWavFile(const_cast<char*>(name), data, bytes, hz);
    if (!ok) {
        deallocateArray(data);
        cout << "  >> ERROR: Could not open: " << name << endl;
    }
    return ok;
}
// --- Up Sample Audio ---
// Doubles samples then saves with 2x Hz so pitch goes higher
void upSampleAudio(unsigned char* arr, int size, int samplingRate) {
    int newSize = 0;
    unsigned char* upSampledArr = doubleArray(arr, size, newSize);
    char newFileName[] = "upsampled.wav";
    bool success = writeWavFile(newFileName, upSampledArr, newSize, samplingRate * 2);
    if (success) {
        cout << "  >> Upsampled file created: " << newFileName << endl;
        cout << "  >> Playing UP-sampled (higher pitch)..." << endl;
        playWavFile(newFileName);
    }
    else cout << "  >> Error writing upsampled file!" << endl;
    delete[] upSampledArr;
}
// --- Down Sample Audio ---
// Shrinks samples then saves with half Hz so pitch goes lower
void downSampleAudio(unsigned char* arr, int size, int samplingRate) {
    int newSize = 0;
    unsigned char* downSampledArr = shrinkArray(arr, size, newSize);
    char newFileName[] = "downsampled.wav";
    bool success = writeWavFile(newFileName, downSampledArr, newSize, samplingRate / 2);
    if (success) {
        cout << "  >> Downsampled file created: " << newFileName << endl;
        cout << "  >> Playing DOWN-sampled (lower pitch)..." << endl;
        playWavFile(newFileName);
    }
    else cout << "  >> Error writing downsampled file!" << endl;
    delete[] downSampledArr;
}
// --- Moving Average Filter ---
// Smooths the audio then saves and plays the result
void movingAverageFilter(const unsigned char* data, int bytes, int hz, int n, const char* name) {
    unsigned char* cl = nullptr;
    allocateArray(cl, bytes);
    fillWithMean(data, cl, bytes, n);
    writeWavFile(const_cast<char*>(name), cl, bytes, hz);
    cout << "  >> Saved: " << name << endl;
    cout << "  >> Playing filtered audio (N=" << n << ")..." << endl;
    playWavFile(const_cast<char*>(name));
    deallocateArray(cl);
}
// --- Merge Array ---
// Interleaves two arrays as per assignment: {1,2,3}+{10,20} = {1,10,2,20,3}
// Remaining elements are appended after the shorter array runs out
void mergeArray(const unsigned char* a, int la, const unsigned char* b, int lb,
    unsigned char*& out, int& outLen) {
    outLen = la + lb;
    allocateArray(out, outLen);
    int pA = 0, pB = 0, pM = 0;
    while (pA < la && pB < lb) { out[pM++] = a[pA++]; out[pM++] = b[pB++]; }
    while (pA < la) out[pM++] = a[pA++];
    while (pB < lb) out[pM++] = b[pB++];
}
// --- UI ---
void showWelcome() {
    cout << "\n  ==========================================\n";
    cout << "         Welcome to DJ Hub                   \n";
    cout << "       SANA FATIMA(2501033)           \n";
    cout << "  ==========================================\n\n";
}
void showMenu() {
    cout << "\n  ------------------------------------------\n";
    cout << "         ~~ PLEASE ENTER YOUR CHOICE ~~                 \n";
    cout << "  You have 3 songs. Please enter as it is.              \n";
    cout << "  [1] dhani.wav                                         \n";
    cout << "  [2] sallimono.wav                                     \n";
    cout << "  [3] song.wav                                          \n";
    cout << "  ------------------------------------------\n";
    cout << "   Option 1  Load an Audio File                     \n";
    cout << "   Option 2  View Audio Samples                  \n";
    cout << "   Option 3  Up-Sample it (Higher Pitch)          \n";
    cout << "   Option 4  Down-Sample it (Lower  Pitch)          \n";
    cout << "   Option 5  Moving Average Filter               \n";
    cout << "   Option 6  Mix Two Tracks (Merging)                     \n";
    cout << "   Otpion 7  Play a Track of your Choice                        \n";
    cout << "   Option 0  Exit DJ Hub                         \n";
    cout << "  ------------------------------------------\n";
    cout << "  Select: ";
}
int main() {
    SetCurrentDirectoryA("C:\\Users\\Mr Laptop point\\Desktop\\MUSIC DJJ\\x64\\Debug");
    showWelcome();

    unsigned char* c1 = nullptr, * c2 = nullptr;
    int  c1B = 0, c2B = 0;
    int  c1Hz = 0, c2Hz = 0;
    bool c1R = false, c2R = false;
    int  pick = -1;

    while (pick != 0) {
        showMenu();
        cin >> pick;

        if (pick == 1) {
            char name[300];
            cout << "  Filename (e.g. sallimono.wav): "; cin >> name;
            if (c1R) { deallocateArray(c1); c1R = false; }
            c1B = 0;
            if (readFromFile(name, c1, c1B, c1Hz)) {
                cout << "  >> Loaded! Samples=" << c1B << "  Hz=" << c1Hz << endl;
                c1R = true;
            }
        }
        else if (pick == 2) {
            if (!c1R) cout << "  >> Load a file first! (Option 1)\n";
            else {
                int sh = (c1B < 30) ? c1B : 30;
                cout << "  First " << sh << " samples:\n";
                printArray(c1, sh);
            }
        }
        else if (pick == 3) {
            if (!c1R) cout << "  >> Load a file first! (Option 1)\n";
            else upSampleAudio(c1, c1B, c1Hz);
        }
        else if (pick == 4) {
            if (!c1R) cout << "  >> Load a file first! (Option 1)\n";
            else downSampleAudio(c1, c1B, c1Hz);
        }
        else if (pick == 5) {
            if (!c1R) cout << "  >> Load a file first! (Option 1)\n";
            else {
                int nv; char name[300];
                cout << "  Enter N value (1, 3, 5, 15, 100): "; cin >> nv;
                cout << "  Save filtered file as: "; cin >> name;
                movingAverageFilter(c1, c1B, c1Hz, nv, name);
            }
        }
        else if (pick == 6) {
            char n1[300], n2[300];
            cout << "  First track :     "; cin >> n1;
            cout << "  Second track : "; cin >> n2;

            if (c1R) { deallocateArray(c1); c1R = false; }
            if (c2R) { deallocateArray(c2); c2R = false; }
            c1B = 0; c2B = 0;

            bool r1 = readFromFile(n1, c1, c1B, c1Hz);
            bool r2 = readFromFile(n2, c2, c2B, c2Hz);

            if (r1 && r2) {
                c1R = c2R = true;
                unsigned char* mix = nullptr; int mixLen = 0;
                mergeArray(c1, c1B, c2, c2B, mix, mixLen);

                // Save with both sampling rates so we can observe the difference
                char out1[300] = "mixed_with_salli_hz.wav";
                char out2[300] = "mixed_with_dhani_hz.wav";
                writeWavFile(out1, mix, mixLen, c2Hz);
                cout << "  >> Saved: " << out1 << "  (Hz=" << c2Hz << ")" << endl;
                writeWavFile(out2, mix, mixLen, c1Hz);
                cout << "  >> Saved: " << out2 << "  (Hz=" << c1Hz << ")" << endl;
                deallocateArray(mix);

                cout << "  >> Playing with SALLI Hz (" << c2Hz << ")...\n";
                playWavFile(out1);
                cout << "  >> Playing with DHANI Hz (" << c1Hz << ")...\n";
                playWavFile(out2);
            }
            else {
                cout << "  >> ERROR: Could not load one or both files. Check again if you enter the same name .\n";
                if (c1R) { deallocateArray(c1); c1R = false; }
                if (c2R) { deallocateArray(c2); c2R = false; }
            }
        }
        else if (pick == 7) {
            char name[300];
            cout << "  Filename to play: "; cin >> name;
            if (!playWavFile(name)) cout << "  >> ERROR:This File is not found! Check if you wrote correct name\n";
        }
        else if (pick == 0) {
            cout << "\n  ==========================================\n";
            cout << "       Thanks for using DJ Hub!             \n";
            cout << "            See you next time!              \n";
            cout << "  ==========================================\n\n";
        }
        else cout << "  >> Invalid option. Try again.\n";
    }
    if (c1R) deallocateArray(c1);
    if (c2R) deallocateArray(c2);
    return 0;
}