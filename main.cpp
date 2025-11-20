#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include "effects.h"
#include "m4p.h"

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Gdi32.lib")
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"Winmm.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken;
CracktroEffects* g_Effects = nullptr;
void* g_pBits = nullptr;
HBITMAP g_hBitmap = nullptr;
HDC g_hMemDC = nullptr;
int g_Width = SCREEN_WIDTH;
int g_Height = SCREEN_HEIGHT;

HWAVEOUT g_hWaveOut = NULL;
#define NUM_BUFFERS 3
#define BUFFER_SIZE 8192
#define SAMPLE_RATE 44100
WAVEHDR g_WaveHeaders[NUM_BUFFERS];
int16_t* g_AudioBuffers[NUM_BUFFERS];
uint8_t* g_ModData = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InitDIBSection(HDC hdc, int width, int height);

#include "resource.h"

bool LoadResourceToMemory(int resourceId, uint8_t** data, uint32_t* size) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hRes) return false;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;

    *size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);
    
    *data = (uint8_t*)malloc(*size);
    if (!*data) return false;
    
    memcpy(*data, pData, *size);
    return true;
}

void FillBuffer(LPWAVEHDR pHeader) {
    m4p_GenerateSamples((int16_t*)pHeader->lpData, BUFFER_SIZE);
}

void InitAudio(HWND hWnd) {
    uint32_t dataSize;
    if (!LoadResourceToMemory(IDR_MUSIC, &g_ModData, &dataSize)) {
        MessageBox(hWnd, "Failed to load music resource", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (!m4p_LoadFromData(g_ModData, dataSize, SAMPLE_RATE, BUFFER_SIZE)) {
        MessageBox(hWnd, "Failed to load module data", "Error", MB_OK | MB_ICONERROR);
        free(g_ModData);
        g_ModData = nullptr;
        return;
    }

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    if (waveOutOpen(&g_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)hWnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR) {
        MessageBox(hWnd, "Failed to open waveOut", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    m4p_PlaySong();

    for (int i = 0; i < NUM_BUFFERS; i++) {
        g_AudioBuffers[i] = (int16_t*)malloc(BUFFER_SIZE * 2 * sizeof(int16_t));
        g_WaveHeaders[i].lpData = (LPSTR)g_AudioBuffers[i];
        g_WaveHeaders[i].dwBufferLength = BUFFER_SIZE * 2 * sizeof(int16_t);
        g_WaveHeaders[i].dwFlags = 0;
        g_WaveHeaders[i].dwLoops = 0;

        waveOutPrepareHeader(g_hWaveOut, &g_WaveHeaders[i], sizeof(WAVEHDR));
        FillBuffer(&g_WaveHeaders[i]);
        waveOutWrite(g_hWaveOut, &g_WaveHeaders[i], sizeof(WAVEHDR));
    }
}

void CleanupAudio() {
    if (g_hWaveOut) {
        waveOutReset(g_hWaveOut);
        for (int i = 0; i < NUM_BUFFERS; i++) {
            waveOutUnprepareHeader(g_hWaveOut, &g_WaveHeaders[i], sizeof(WAVEHDR));
            free(g_AudioBuffers[i]);
        }
        waveOutClose(g_hWaveOut);
        g_hWaveOut = NULL;
    }
    m4p_FreeSong();
    m4p_Close();
    if (g_ModData) {
        free(g_ModData);
        g_ModData = nullptr;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "MapenoClass";
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow("MapenoClass", "Mapeno Cracktro Remake",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, g_Width, g_Height,
        NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return 0;
    }

    g_Effects = new CracktroEffects();
    g_Effects->Initialize();

    InitAudio(hWnd);

    MSG msg = {0};
    DWORD lastTime = GetTickCount();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            DWORD currentTime = GetTickCount();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            g_Effects->Update(deltaTime);

            InvalidateRect(hWnd, NULL, FALSE);
            Sleep(1);
        }
    }

    CleanupAudio();
    delete g_Effects;
    if (g_hBitmap) DeleteObject(g_hBitmap);
    if (g_hMemDC) DeleteDC(g_hMemDC);
    
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

void InitDIBSection(HDC hdc, int width, int height) {
    if (g_hBitmap) {
        DeleteObject(g_hBitmap);
        g_hBitmap = nullptr;
    }
    if (g_hMemDC) {
        DeleteDC(g_hMemDC);
        g_hMemDC = nullptr;
    }

    g_hMemDC = CreateCompatibleDC(hdc);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    g_hBitmap = CreateDIBSection(g_hMemDC, &bmi, DIB_RGB_COLORS, &g_pBits, NULL, 0);
    SelectObject(g_hMemDC, g_hBitmap);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        {
            HDC hdc = GetDC(hWnd);
            InitDIBSection(hdc, g_Width, g_Height);
            ReleaseDC(hWnd, hdc);
        }
        break;
    case MM_WOM_DONE:
        {
            LPWAVEHDR pHeader = (LPWAVEHDR)lParam;
            FillBuffer(pHeader);
            waveOutWrite(g_hWaveOut, pHeader, sizeof(WAVEHDR));
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            if (g_Effects && g_pBits) {
                g_Effects->RenderToBuffer((uint32_t*)g_pBits, g_Width, g_Height);
                g_Effects->Render(g_hMemDC, g_Width, g_Height);
                BitBlt(hdc, 0, 0, g_Width, g_Height, g_hMemDC, 0, 0, SRCCOPY);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
