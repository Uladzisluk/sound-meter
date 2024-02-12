#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#define BUFFER_SIZE 4096
#define SAMPLE_RATE 44100
#define MAX_CALIBRATION_ATTEMPTS 50

double max_rms = 0;
double max_db = 0;
double sum_db = 0;
double k = 0.0;
int calibration_attempt = 0;
boolean calibration = 1;

UINT chooseMicrophone(int numDevices) {
    printf("Choose microphone to use: ");
    UINT deviceId;
    while (1) {
        scanf_s("%d", &deviceId);
        if (deviceId < numDevices) {
            return deviceId;
        }
        printf("There is no this microphone, choose another: ");
    }
}

int listMicrophones() {
    UINT numDevices = waveInGetNumDevs();
    if (numDevices == 0) {
        printf("There are no microphones available.\n");
        return numDevices;
    }

    printf("Available microphones:\n");

    for (UINT deviceId = 0; deviceId < numDevices; ++deviceId) {
        WAVEINCAPS wic;
        MMRESULT result = waveInGetDevCaps(deviceId, &wic, sizeof(WAVEINCAPS));
        if (result == MMSYSERR_NOERROR) {
            printf("Device %d: %ls \n", deviceId, wic.szPname);
        }
    }
    return numDevices;
}

void checkError(MMRESULT result, const char* message) {
    if (result != MMSYSERR_NOERROR) {
        fprintf(stderr, "%s: %d\n", message, result);
        exit(EXIT_FAILURE);
    }
}

void setMicrophoneVolume() {

}

double calculateRMS(short* buffer, size_t size) {
    double rms = 0.0;
    for (size_t i = 0; i < size; ++i) {
        rms += buffer[i] * buffer[i];
    }
    rms = sqrt(rms / size);
    return rms;
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // Calibration
    if (uMsg == WIM_DATA && calibration) {
        WAVEHDR* waveHeader = (WAVEHDR*)dwParam1;
        short* buffer = (short*)waveHeader->lpData;

        double rms = calculateRMS(buffer, waveHeader->dwBufferLength / sizeof(short));
        double db = 20 * log10(rms);

        sum_db += db;

        waveInAddBuffer(hwi, waveHeader, sizeof(WAVEHDR));

        if (++calibration_attempt >= MAX_CALIBRATION_ATTEMPTS) {
            k = 30 / (sum_db / MAX_CALIBRATION_ATTEMPTS);
            calibration = 0;
        }
    }
    // Volume measurement
    else if (uMsg == WIM_DATA) {
        WAVEHDR* waveHeader = (WAVEHDR*)dwParam1;
        short* buffer = (short*)waveHeader->lpData;

        // Volume measurement and output to the console
        double rms = calculateRMS(buffer, waveHeader->dwBufferLength / sizeof(short));
        double db = 20 * log10(rms * k);

        system("cls");
        printf("Volume: %f dB\n", db);
        Sleep(200);
        waveInAddBuffer(hwi, waveHeader, sizeof(WAVEHDR));
    }
}

int main() {
    MMRESULT result;
    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx;
    WAVEHDR waveHeader;
    WAVEINCAPS wic;

    int numDevices = listMicrophones();
    
    UINT deviceId = chooseMicrophone(numDevices);

    // Getting information about the microphone device
    result = waveInGetDevCaps(deviceId, &wic, sizeof(WAVEINCAPS));
    checkError(result, "Error receiving device information");

    // Initializing the audio format
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.nAvgBytesPerSec = SAMPLE_RATE * sizeof(short);
    wfx.nBlockAlign = sizeof(short) * wfx.nChannels;
    wfx.wBitsPerSample = sizeof(short) * 8;
    wfx.cbSize = 0;

    // Opening the recorder
    result = waveInOpen(&hWaveIn, deviceId, &wfx, (DWORD_PTR)&waveInProc, 0, CALLBACK_FUNCTION);
    checkError(result, "Error opening the recorder");

    // Preparing the buffer for recording
    short buffer[BUFFER_SIZE];
    waveHeader.lpData = (LPSTR)buffer;
    waveHeader.dwBufferLength = BUFFER_SIZE * sizeof(short);
    waveHeader.dwBytesRecorded = 0;
    waveHeader.dwUser = 0;
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 1;
    waveHeader.lpNext = NULL;
    waveHeader.reserved = 0;

    result = waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    checkError(result, "Header preparation error");

    // Adding a buffer for recording
    result = waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    checkError(result, "Error adding buffer for recording");

    // Start of recording
    result = waveInStart(hWaveIn);
    checkError(result, "Error starting recording");

    printf("Calibration: speak in a whisper at a distance of one meter\n");

    while (calibration) {

    }

    printf("Volume Measurement. Press Ctrl+C to finish\n");

    // Waiting for recording to complete
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Stopping recording and clearing resources
    waveInStop(hWaveIn);
    waveInUnprepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    waveInClose(hWaveIn);

    return 0;
}
