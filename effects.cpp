#include "effects.h"
#include <cmath>
#include <algorithm>

inline void SetPixel(uint32_t* buffer, int width, int height, int x, int y, uint32_t color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        buffer[y * width + x] = color;
    }
}

CracktroEffects::CracktroEffects() : scrollOffset(0), logoImage(nullptr) {
    scrollText = "           WELCOME TO THE MAPENO CRACKTRO REMAKE ...     CODE BY MAPENOTEAM ...     GREETINGS TO ALL DEMOSCENE LEGENDS ...";
}

CracktroEffects::~CracktroEffects() {
    if (logoImage) delete logoImage;
}

#include "resource.h"

void CracktroEffects::Initialize() {
    InitStars();
    InitCopperBars();
    
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_LOGO), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(NULL, hRes);
        if (hData) {
            DWORD size = SizeofResource(NULL, hRes);
            void* pData = LockResource(hData);
            if (pData) {
                HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
                if (hBuffer) {
                    void* pBuffer = GlobalLock(hBuffer);
                    if (pBuffer) {
                        memcpy(pBuffer, pData, size);
                        GlobalUnlock(hBuffer);
                        IStream* pStream = NULL;
                        if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK) {
                            logoImage = Gdiplus::Image::FromStream(pStream);
                            pStream->Release();
                        }
                    }
                }
            }
        }
    }
}

void CracktroEffects::InitStars() {
    for (int i = 0; i < 200; ++i) {
        Star s;
        s.x = (float)(rand() % 2000 - 1000);
        s.y = (float)(rand() % 2000 - 1000);
        s.z = (float)(rand() % 1000 + 1);
        s.speed = 2.0f;
        stars.push_back(s);
    }
}

void CracktroEffects::InitCopperBars() {
    for (int i = 0; i < 40; ++i) {
        CopperBar bar;
        bar.y = rand() % SCREEN_HEIGHT;
        bar.height = 8;
        
        float wave = (sin(i * 0.15f) + 1.0f) * 0.5f;
        
        int r = (int)(0 + wave * 135);
        int g = (int)(60 + wave * 146);
        int b = (int)(120 + wave * 130);
        
        bar.color = Gdiplus::Color(255, r, g, b);
        bar.speed = (float)(sin(i * 0.1f) * 1.5f + 1.0f);
        copperBars.push_back(bar);
    }
}

void CracktroEffects::UpdateCopperBars(float deltaTime) {
    static float time = 0.0f;
    time += deltaTime;
    
    for (size_t i = 0; i < copperBars.size(); ++i) {
        float center = SCREEN_HEIGHT * 0.75f;
        float amplitude = SCREEN_HEIGHT * 0.20f;
        copperBars[i].y = (int)(center + sin(time * 1.5f + i * 0.1f) * amplitude);
    }
}

void CracktroEffects::Update(float deltaTime) {
    UpdateStars(deltaTime);
    UpdateCopperBars(deltaTime);
    scrollOffset += 150.0f * deltaTime;
    
    float fontSize = (float)SCREEN_HEIGHT / 5.0f;
    float charSpacing = fontSize * 0.6f;
    float totalTextWidth = scrollText.length() * charSpacing;
    
    if (scrollOffset > totalTextWidth + fontSize) { 
        scrollOffset = -(float)SCREEN_WIDTH;
    }
}

void CracktroEffects::UpdateStars(float deltaTime) {
    for (auto& s : stars) {
        s.z -= s.speed * 50.0f * deltaTime;
        if (s.z <= 0) {
            s.z += 1000.0f;
            s.x = (float)(rand() % 2000 - 1000);
            s.y = (float)(rand() % 2000 - 1000);
        }
    }
}

void CracktroEffects::RenderToBuffer(uint32_t* buffer, int width, int height) {
    std::fill(buffer, buffer + width * height, 0xFF000000);

    DrawCopperBars(buffer, width, height);
    DrawStars(buffer, width, height);
}

void CracktroEffects::DrawStars(uint32_t* buffer, int width, int height) {
    int cx = width / 2;
    int cy = height / 2;
    
    for (const auto& s : stars) {
        if (s.z <= 0) continue;
        
        int sx = (int)(s.x / s.z * 100.0f) + cx;
        int sy = (int)(s.y / s.z * 100.0f) + cy;
        
        int brightness = 255 - (int)(s.z * 255 / 1000);
        if (brightness < 0) brightness = 0;
        if (brightness > 255) brightness = 255;
        
        uint32_t color = 0xFF000000 | (brightness << 16) | (brightness << 8) | brightness;
        
        SetPixel(buffer, width, height, sx, sy, color);
    }
}

void CracktroEffects::DrawCopperBars(uint32_t* buffer, int width, int height) {
    for (const auto& bar : copperBars) {
        for (int y = bar.y; y < bar.y + bar.height; ++y) {
            if (y >= 0 && y < height) {
                uint32_t color = bar.color.GetValue();
                for (int x = 0; x < width; ++x) {
                    buffer[y * width + x] = color; 
                }
            }
        }
    }
}

void CracktroEffects::Render(HDC hdc, int width, int height) {
    Gdiplus::Graphics graphics(hdc);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    if (logoImage && logoImage->GetLastStatus() == Gdiplus::Ok) {
        int imgW = logoImage->GetWidth();
        int imgH = logoImage->GetHeight();

        float scale = 1.0f;
        if (imgW > width - 40) scale = (float)(width - 40) / imgW;
        
        int drawW = (int)(imgW * scale);
        int drawH = (int)(imgH * scale);
        int drawX = (width - drawW) / 2;
        int drawY = (int)(height * 0.75f) - (drawH / 2);
        
        Gdiplus::ImageAttributes imageAtt;
        imageAtt.SetColorKey(Gdiplus::Color(0, 0, 0), Gdiplus::Color(10, 10, 10), Gdiplus::ColorAdjustTypeBitmap); // Range of black

        Gdiplus::Rect destRect(drawX, drawY, drawW, drawH);
        graphics.DrawImage(logoImage, destRect, 0, 0, imgW, imgH, Gdiplus::UnitPixel, &imageAtt);
    } else {
        Gdiplus::FontFamily fontFamily(L"Impact");
        Gdiplus::Font logoFont(&fontFamily, 60, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush logoBrush(Gdiplus::Color(255, 255, 255, 255));
        graphics.DrawString(L"MAPENO", -1, &logoFont, Gdiplus::PointF((float)width/2 - 100, (float)height * 0.75f), &logoBrush);
    }

    DrawScroller(graphics, width, height);
}

void CracktroEffects::DrawScroller(Gdiplus::Graphics& graphics, int width, int height) {
    int fontSize = height / 5;

    Gdiplus::Font font(L"Courier New", (Gdiplus::REAL)fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 200, 200, 255));
    
    std::wstring wText(scrollText.begin(), scrollText.end());
    
    float centerY = (height * 0.25f) - (fontSize / 2.0f);
    
    static float wavePhase = 0.0f;
    wavePhase += 0.1f;

    float amplitude = fontSize * 0.4f;
    float frequency = 0.015f;

    for (size_t i = 0; i < wText.length(); ++i) {
        float x = (float)i * (fontSize * 0.6f) - scrollOffset;
        if (x < -fontSize || x > width) continue;
        
        float angleArg = x * frequency + wavePhase;
        float y = centerY + sin(angleArg) * amplitude;
        
        float slope = amplitude * frequency * cos(angleArg);
        float angleRad = atan(slope);
        float angleDeg = angleRad * (180.0f / 3.14159f);

        Gdiplus::GraphicsState state = graphics.Save();

        graphics.TranslateTransform(x, y);
        graphics.RotateTransform(angleDeg);
        graphics.DrawString(std::wstring(1, wText[i]).c_str(), -1, &font, Gdiplus::PointF(0, 0), &brush);
        graphics.Restore(state);
    }
}
