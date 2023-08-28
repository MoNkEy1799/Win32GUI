#include "Dinogui.h"
#include "Utils.h"

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cmath>

using namespace DINOGUI;

Canvas::Canvas(Core* core, int width, int height, const Color& fillColor)
    : Widget(core), m_drawingBitmap(nullptr), m_wicBitmap(nullptr), m_wicLock(nullptr),
      m_buffer(nullptr), m_bufferWidth(width), m_bufferHeight(height)
{
    m_type = WidgetType::CANVAS;
    m_drawBorder = true;
    m_size = { (float)width, (float)height };
    createPixelBuffer();
    fill(fillColor);
}

Canvas::~Canvas()
{
    safeReleaseInterface(&m_drawingBitmap);
    safeReleaseInterface(&m_wicBitmap);
    safeReleaseInterface(&m_wicLock);
}

void Canvas::draw(ID2D1HwndRenderTarget* renderTarget, ID2D1SolidColorBrush* brush)
{
    drawBasicShape(renderTarget, brush);

    if (!m_drawingBitmap && m_wicBitmap)
    {
        throwIfFailed(renderTarget->CreateBitmapFromWicBitmap(m_wicBitmap, nullptr, &m_drawingBitmap), "Failed to create Bitmap");
    }

    if (m_drawingBitmap)
    {
        renderTarget->DrawBitmap(m_drawingBitmap, DPIHandler::adjusted(bufferRect()));
    }
}

void Canvas::place(int x, int y)
{
    basicPlace(x, y);
}

void Canvas::fill(const Color& color, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    for (size_t pos = 0; pos < m_bufferWidth * m_bufferHeight; pos++)
    {
        setColor(color, pos * 4);
    }

    if (autoLock)
    {
        lock();
    }
}

void Canvas::setPixel(const Color& color, int x, int y, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    setColor(color, bytePosFromXY(x, y));

    if (autoLock)
    {
        lock();
    }
}

void Canvas::setPixel(const Color& color, size_t pos, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    setColor(color, pos * 4);

    if (autoLock)
    {
        lock();
    }
}

void Canvas::drawLine(Point p1, Point p2, const Color& color, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    int xa = (int)p1.x;
    int ya = (int)p1.y;
    int xb = (int)p2.x;
    int yb = (int)p2.y;
    bool steep = std::abs(yb - ya) > std::abs(xb - xa);
    if (steep)
    {
        swap(xa, ya);
        swap(xb, yb);
    }
    if (xa > xb)
    {
        swap(xa, xb);
        swap(ya, yb);
    }

    float dx = (float)xb - xa;
    float dy = (float)yb - ya;
    float grad = dy / dx;
    float yInter = (float)ya;
    if (dx == 0.0f)
    {
        grad = 1.0f;
    }

    for (int x = xa; x <= xb; x++)
    {
        if (steep)
        {
            setColor(color, bytePosFromXY((int)yInter, x));
            setColor(color, bytePosFromXY((int)yInter - 1, x));
            yInter += grad;
        }
        else
        {
            setColor(color, bytePosFromXY(x, (int)yInter));
            setColor(color, bytePosFromXY(x, (int)yInter - 1));
            yInter += grad;
        }
    }

    if (autoLock)
    {
        lock();
    }
}

void Canvas::drawRectangle(Point p1, Point p2, const Color& color, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    int xmin, xmax, ymin, ymax;
    if (p1.x < p2.x)
    {
        xmin = (int)p1.x;
        xmax = (int)p2.x;
    }
    else
    {
        xmin = (int)p2.x;
        xmax = (int)p1.x;
    }
    if (p1.y < p2.y)
    {
        ymin = (int)p1.y;
        ymax = (int)p2.y;
    }
    else
    {
        ymin = (int)p2.y;
        ymax = (int)p1.y;
    }
    checkBounds(xmin);
    checkBounds(xmax);
    checkBounds(ymin);
    checkBounds(ymax);

    for (int x = xmin; x <= xmax; x++)
    {
        for (int y = ymin; y <= ymax; y++)
        {
            setColor(color, bytePosFromXY(x, y));
        }
    }

    if (autoLock)
    {
        lock();
    }
}

void Canvas::drawTriangle(Point p1, Point p2, Point p3, const Color& color, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    if (p1.y < p2.y && p1.y < p3.y)
    {
        if (p3.y < p2.y)
        {
            swap(p2, p3);
        }
    }
    else if (p2.y < p1.y && p2.y < p3.y)
    {
        if (p1.y < p3.y)
        {
            swap(p1, p2);
        }
        else
        {
            swap(p1, p3);
            swap(p1, p2);
        }
    }
    else
    {
        if (p2.y < p1.y)
        {
            swap(p1, p3);
        }
        else
        {
            swap(p1, p3);
            swap(p2, p3);
        }
    }

    if (p2.y == p3.y)
    {
        fillBottomTriangle(p1, p2, p3, color);
    }
    else if (p1.y == p2.y)
    {
        fillTopTriangle(p1, p2, p3, color);
    }
    else
    {
        Point p4 = { p1.x + (p2.y - p1.y) / (p3.y - p1.y) * (p3.x - p1.x), p2.y };
        fillBottomTriangle(p1, p2, p4, color);
        fillTopTriangle(p2, p4, p3, color);
    }

    if (autoLock)
    {
        lock();
    }
}

void Canvas::drawCircle(Point p, int r, const Color& color, bool autoLock)
{
    drawEllipse(p, r, r, color, autoLock);
}

void Canvas::drawEllipse(Point p, int ra, int rb, const Color& color, bool autoLock)
{
    if (autoLock)
    {
        unlock();
    }
    else if (!m_buffer)
    {
        return;
    }

    int xmin = (int)p.x - ra;
    int xmax = (int)p.x + ra;
    int ymin = (int)p.y - rb;
    int ymax = (int)p.y + rb;
    checkBounds(xmin);
    checkBounds(xmax);
    checkBounds(ymin);
    checkBounds(ymax);

    for (int xs = xmin; xs <= xmax; xs++)
    {
        for (int ys = ymin; ys <= ymax; ys++)
        {
            double ellipse = std::pow(xs - p.x, 2) / std::pow(ra, 2) + std::pow(ys - p.y, 2) / std::pow(rb, 2);
            if (ellipse < 1)
            {
                setColor(color, bytePosFromXY(xs, ys));
            }
        }
    }

    if (autoLock)
    {
        lock();
    }
}

void Canvas::lock()
{
    m_buffer = nullptr;
    safeReleaseInterface(&m_wicLock);
}

void Canvas::unlock()
{
    WICRect wicRect = { 0, 0, m_bufferWidth, m_bufferHeight };
    uint32_t bufferSize;
    m_wicBitmap->Lock(&wicRect, WICBitmapLockWrite, &m_wicLock);
    m_wicLock->GetDataPointer(&bufferSize, &m_buffer);
}

void Canvas::createPixelBuffer()
{
    uint32_t* buffer = new uint32_t[m_bufferWidth * m_bufferHeight]{ 0 };
    throwIfFailed(m_core->getImageFactory()->CreateBitmapFromMemory(m_bufferWidth, m_bufferHeight, GUID_WICPixelFormat32bppPRGBA,
        m_bufferWidth * 4, m_bufferWidth * m_bufferHeight * 4, (byte*)buffer, &m_wicBitmap));
    setSize(m_bufferWidth, m_bufferHeight);
    delete[] buffer;
}

void Canvas::setColor(const Color& color, size_t bytePos)
{
    m_buffer[bytePos + 0] = (byte)color.r;
    m_buffer[bytePos + 1] = (byte)color.g;
    m_buffer[bytePos + 2] = (byte)color.b;
    m_buffer[bytePos + 3] = (byte)color.a;
}

void Canvas::checkBounds(int& n) const
{
    if (n < 0)
    {
        n = 0;
    }
    else if (n > m_bufferWidth)
    {
        n = m_bufferWidth;
    }
}

int Canvas::bytePosFromXY(int x, int y) const
{
    return (y * m_bufferWidth + x) * 4;
}

D2D1_RECT_F Canvas::bufferRect() const
{
    D2D1_RECT_F current = currentRect();
    return { current.left + 1.0f, current.top + 1.0f, current.right, current.bottom };
}

void Canvas::fillBottomTriangle(Point p1, Point p2, Point p3, const Color& color)
{
    if (p2.x > p3.x)
    {
        swap(p2, p3);
    }
    float grad1 = (p2.x - p1.x) / (p2.y - p1.y);
    float grad2 = (p3.x - p1.x) / (p3.y - p1.y);
    float x1 = p1.x;
    float x2 = p1.x;

    for (int y = (int)p1.y; y <= p2.y; y++)
    {
        for (int x = (int)x1; x <= (int)x2; x++)
        {
            setColor(color, bytePosFromXY(x, y));
        }
        x1 += grad1;
        x2 += grad2;
    }
}

void Canvas::fillTopTriangle(Point p1, Point p2, Point p3, const Color& color)
{
    if (p1.x > p2.x)
    {
        swap(p1, p2);
    }
    float grad1 = (p3.x - p1.x) / (p3.y - p1.y);
    float grad2 = (p3.x - p2.x) / (p3.y - p2.y);
    float x1 = p3.x;
    float x2 = p3.x;

    for (int y = (int)p3.y; y >= p1.y; y--)
    {
        for (int x = (int)x1; x <= (int)x2; x++)
        {
            setColor(color, bytePosFromXY(x, y));
        }
        x1 -= grad1;
        x2 -= grad2;
    }
}

void Canvas::swap(int& a, int& b)
{
    int temp = a;
    a = b;
    b = temp;
}

void Canvas::swap(Point& a, Point& b)
{
    float temp = a.x;
    a.x = b.x;
    b.x = temp;
    temp = a.y;
    a.y = b.y;
    b.y = temp;
}

float Canvas::frac(float a)
{
    return a - (int)a;
}

float Canvas::rfrac(float a)
{
    return 1 - frac(a);
}
