#ifndef EFFECTS_H
#define EFFECTS_H

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

struct Star {
    float x, y, z;
    float speed;
};

struct CopperBar {
    int y;
    int height;
    Gdiplus::Color color;
    float speed;
};

class CracktroEffects {
public:
    CracktroEffects();
    ~CracktroEffects();

    void Initialize();
    void Update(float deltaTime);
    void Render(HDC hdc, int width, int height);
    void RenderToBuffer(uint32_t* buffer, int width, int height);

private:
    std::vector<Star> stars;
    std::vector<CopperBar> copperBars;
    std::string scrollText;
    float scrollOffset;
    
    void InitStars();
    void UpdateStars(float deltaTime);
    void DrawStars(uint32_t* buffer, int width, int height);
    
    void InitCopperBars();
    void UpdateCopperBars(float deltaTime);
    void DrawCopperBars(uint32_t* buffer, int width, int height);

    void DrawScroller(Gdiplus::Graphics& graphics, int width, int height);

    Gdiplus::Image* logoImage;
};

#endif
