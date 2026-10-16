#pragma once

// 窗口逻辑坐标，原点在左上；App 再换算到 framebuffer 像素。
struct ViewportRect
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    bool contains(double cursorX, double cursorY) const
    {
        return cursorX >= x && cursorY >= y && cursorX < x + width && cursorY < y + height;
    }
};
