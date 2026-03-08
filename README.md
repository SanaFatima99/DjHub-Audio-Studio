~~ DJ Hub — Audio Processing Studio
> A C++ console application for real-time WAV audio manipulation.

~~About the Project

DJ Hub is a menu-driven audio processing program that lets you load WAV files and apply various signal processing operations on them — including up-sampling, down-sampling, moving average filtering, and mixing two tracks together. All output files are saved and played back automatically so you can hear the effect instantly.

This project was built entirely in C++ using low-level memory management (no STL containers) and the Windows Multimedia API for audio playback.

~~Features

| 1 | Load Audio File:
Loads any WAV file (8, 16, 24, or 32-bit) and converts it to 8-bit mono |
| 2 | View Audio Samples |
Prints the first 30 sample values of the loaded file |
| 3 | Up-Sample:
Duplicates each sample → doubles Hz → higher pitch playback |
| 4 | Down-Sample:
Keeps every alternate sample → halves Hz → lower pitch playback |
| 5 | Moving Average Filter: 
Smooths audio using configurable N-neighbor averaging |
| 6 | Mix Two Tracks:
Interleaves two audio arrays and saves at both sampling rates |
| 7 | Play a Track:
Plays any WAV file from the working directory |
| 0 | Exit:
Exits the program cleanly, freeing all memory |

---

Elaboration:

### Up Sampling
Each sample is duplicated so the array doubles in size. The sampling rate is also doubled, which causes the audio player to play it back at a higher pitch.
```
{1, 2, 3} → {1, 1, 2, 2, 3, 3}
```

### Down Sampling
Every alternate sample is discarded and the sampling rate is halved, resulting in lower pitch playback.
```
{1, 2, 3, 4} → {1, 3}
```

### Moving Average Filter
Each output sample is replaced by the average of its N left and N right neighbors. Boundaries are handled gracefully by using only available neighbors.
```
N=1, input[i] = mean of (i-1, i, i+1)
```

### Mixing (Merge Array)
Two audio arrays are interleaved element by element. When one array runs out, the remaining elements of the longer array are appended.
```
{1,2,3} + {10,20} = {1,10,2,20,3}
```
The mixed file is saved twice — once at each file's original sampling rate — so you can compare how pitch changes with the same data.

---

## 🔧 Technical Highlights

- **Supports all WAV formats** — 8-bit, 16-bit, 24-bit, and 32-bit float (including stereo files like `song.wav` which is 32-bit float stereo at 44100 Hz)
- **Dynamic memory allocation** — no fixed buffer sizes; each file gets exactly the memory it needs
- **Manual memory management** — uses `new` and `delete[]` throughout, no STL
- **Windows Multimedia API** — uses `PlaySoundA` with full directory path for reliable playback
- **Auto working directory** — `SetCurrentDirectoryA` is set at startup so filenames can be typed without full paths

---

## Project Structure

```
MUSIC DJJ/
├── Sound (2).cpp       # Main program — all audio processing functions + menu
├── wavfile.h           # Header — function declarations for WAV I/O
├── wavfile.cpp         # WAV file read/write/play implementation
├── dhani.wav           # Sample audio file (8-bit mono)
├── sallimono.wav       # Sample audio file (8-bit mono)
└── song.wav            # Sample audio file (32-bit float stereo)
```

---

##  How to Run

1. Clone the repository
2. Open `MUSIC DJJ.sln` in **Visual Studio**
3. Make sure all three source files are added under **Source Files** in Solution Explorer
4. Update the path in `main()` to match your system:
cpp
SetCurrentDirectoryA("C:\\Your\\Path\\To\\MUSIC DJJ\\x64\\Debug");
5. Place your `.wav` files in the same `Debug` folder
6. Press **Ctrl+Shift+B** to build, then run

##  Requirements

- Windows OS
- Visual Studio (any recent version)
- WAV audio files to process

##  Author

**Sana Fatima**  
Department of Creative Technologies 
