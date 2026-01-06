#include "Termgl.h"
#include <algorithm>
#include <cmath>

namespace termgl {

    // ============================================================================
    // PARTITION MANAGEMENT
    // ============================================================================

    int Window::addPartition(int x, int y, int w, int h, const std::string& title) {
        int id = partitions.size();
        partitions.emplace_back(id, x, y, w, h, title);
        return id;
    }

    void Window::setActivePartition(int id) {
        if (id >= -1 && id < (int)partitions.size()) {
            activePartitionID = id;
            if (id != -1) partitions[id].active = true;
        }
    }

    void Window::drawPartitionFrames() {
        int prevID = activePartitionID;
        activePartitionID = -1;

        for (const auto& p : partitions) {
            fillRect(p.rect.x, p.rect.y, p.rect.w, p.rect.h, Color(0, 0, 0));
            Color borderC = (prevID == p.id) ? Color::White() : p.borderColor;
            drawRect(p.rect.x, p.rect.y, p.rect.w, p.rect.h, borderC);
            fillRect(p.rect.x, p.rect.y, p.rect.w, 20, (prevID == p.id) ? Color(40, 40, 40) : Color(20, 20, 20));
            drawText(p.rect.x + 5, p.rect.y + 2, p.title, p.titleColor);
        }

        activePartitionID = prevID;
    }

    void Window::clearPartition(int id, Color color) {
        if (id >= 0 && id < (int)partitions.size()) {
            int prevID = activePartitionID;
            activePartitionID = -1; 
            const auto& p = partitions[id];
            fillRect(p.rect.x + 1, p.rect.y + 21, p.rect.w - 2, p.rect.h - 22, color);
            activePartitionID = prevID;
        }
    }

    void Window::transformCoordinates(int& x, int& y) const {
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            x += p.rect.x;
            y += p.rect.y + 20;
        }
    }

    bool Window::clipCoordinates(int x, int y) const {
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            int minX = p.rect.x + 1;
            int maxX = p.rect.x + p.rect.w - 2;
            int minY = p.rect.y + 21;
            int maxY = p.rect.y + p.rect.h - 2;
            return (x >= minX && x < maxX && y >= minY && y < maxY);
        }
        return (x >= 0 && x < width && y >= 0 && y < height);
    }

    int Window::getWidth() const {
        if (activePartitionID != -1) return partitions[activePartitionID].rect.w;
        return width;
    }

    int Window::getHeight() const {
        if (activePartitionID != -1) return partitions[activePartitionID].rect.h - 20; 
        return height;
    }

    // ============================================================================
    // DRAWING PRIMITIVES (OPTIMIZED)
    // ============================================================================

    void Window::drawPixel(int x, int y, Color color) {
        transformCoordinates(x, y);
        if (clipCoordinates(x, y)) {
            buffer[x + y * width] = color.toInt();
        }
    }

    void Window::clear(Color color) {
        if (activePartitionID == -1) {
            uint32_t c = color.toInt();
            std::fill(buffer, buffer + (width * height), c);
        }
        else {
            clearPartition(activePartitionID, color);
        }
    }

    void Window::drawBuffer(int x, int y, int w, int h, const uint32_t* data) {
        int tx = x, ty = y;
        transformCoordinates(tx, ty);

        int minX = 0, maxX = width, minY = 0, maxY = height;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            minX = p.rect.x + 1; maxX = p.rect.x + p.rect.w - 1;
            minY = p.rect.y + 21; maxY = p.rect.y + p.rect.h - 1;
        }

        int drawX = std::max(minX, tx);
        int drawY = std::max(minY, ty);
        int drawW = std::min(maxX, tx + w) - drawX;
        int drawH = std::min(maxY, ty + h) - drawY;

        if (drawW <= 0 || drawH <= 0) return;

        int srcOffsetX = drawX - tx;
        int srcOffsetY = drawY - ty;

        for (int row = 0; row < drawH; ++row) {
            uint32_t* destPtr = &buffer[(drawY + row) * width + drawX];
            const uint32_t* srcPtr = &data[(srcOffsetY + row) * w + srcOffsetX];
            memcpy(destPtr, srcPtr, drawW * sizeof(uint32_t));
        }
    }

    void Window::drawLine(int x0, int y0, int x1, int y1, Color color) {
        int clipMinX = 0, clipMaxX = width;
        int clipMinY = 0, clipMaxY = height;

        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            x0 += p.rect.x; y0 += p.rect.y + 20;
            x1 += p.rect.x; y1 += p.rect.y + 20;
            clipMinX = p.rect.x + 1;
            clipMaxX = p.rect.x + p.rect.w - 1;
            clipMinY = p.rect.y + 21;
            clipMaxY = p.rect.y + p.rect.h - 1;
        }

        uint32_t c = color.toInt();
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            if (x0 >= clipMinX && x0 < clipMaxX && y0 >= clipMinY && y0 < clipMaxY) {
                buffer[y0 * width + x0] = c;
            }
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void Window::drawRect(int x, int y, int w, int h, Color color) {
        drawLine(x, y, x + w - 1, y, color);
        drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
        drawLine(x, y, x, y + h - 1, color);
        drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    }

    void Window::fillRect(int x, int y, int w, int h, Color color) {
        int tx = x, ty = y;
        transformCoordinates(tx, ty);

        int minX = 0, maxX = width, minY = 0, maxY = height;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            minX = p.rect.x + 1; maxX = p.rect.x + p.rect.w - 1;
            minY = p.rect.y + 21; maxY = p.rect.y + p.rect.h - 1;
        }

        int startX = std::max(minX, tx);
        int startY = std::max(minY, ty);
        int endX = std::min(maxX, tx + w);
        int endY = std::min(maxY, ty + h);

        if (startX >= endX || startY >= endY) return;

        uint32_t c = color.toInt();
        for (int j = startY; j < endY; j++) {
            uint32_t* row = &buffer[j * width];
            for (int i = startX; i < endX; i++) {
                row[i] = c;
            }
        }
    }

    void Window::fillGradientRect(int x, int y, int w, int h, Color c1, Color c2, bool vertical) {
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                float ratio = vertical ? (float)j / h : (float)i / w;
                if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
                uint8_t r = (uint8_t)(c1.r + (c2.r - c1.r) * ratio);
                uint8_t g = (uint8_t)(c1.g + (c2.g - c1.g) * ratio);
                uint8_t b = (uint8_t)(c1.b + (c2.b - c1.b) * ratio);
                uint8_t a = (uint8_t)(c1.a + (c2.a - c1.a) * ratio);
                drawPixel(x + i, y + j, Color(r, g, b, a));
            }
        }
    }

    void Window::drawCircle(int xc, int yc, int r, Color color) {
        int x = 0, y = r, d = 3 - 2 * r;
        auto plot8 = [&](int cx, int cy, int xx, int yy) {
            drawPixel(cx + xx, cy + yy, color); drawPixel(cx - xx, cy + yy, color);
            drawPixel(cx + xx, cy - yy, color); drawPixel(cx - xx, cy - yy, color);
            drawPixel(cx + yy, cy + xx, color); drawPixel(cx - yy, cy + xx, color);
            drawPixel(cx + yy, cy - xx, color); drawPixel(cx - yy, cy - xx, color);
            };
        while (y >= x) {
            plot8(xc, yc, x, y);
            x++;
            if (d > 0) { y--; d = d + 4 * (x - y) + 10; }
            else { d = d + 4 * x + 6; }
        }
    }

    void Window::fillCircle(int xc, int yc, int r, Color color) {
        if (r <= 0) return;

        int clipMinX = 0, clipMaxX = width;
        int clipMinY = 0, clipMaxY = height;

        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            xc += p.rect.x; yc += p.rect.y + 20;
            clipMinX = p.rect.x + 1; 
            clipMaxX = p.rect.x + p.rect.w - 1;
            clipMinY = p.rect.y + 21; 
            clipMaxY = p.rect.y + p.rect.h - 1;
        }

        if (xc + r < clipMinX || xc - r >= clipMaxX || yc + r < clipMinY || yc - r >= clipMaxY) return;

        uint32_t c = color.toInt();
        int x = 0, y = r;
        int d = 3 - 2 * r;

        auto drawScanline = [&](int y1, int x1, int x2) {
            if (y1 < clipMinY || y1 >= clipMaxY) return;
            if (x1 > x2) std::swap(x1, x2);
            x1 = std::max(x1, clipMinX);
            x2 = std::min(x2, clipMaxX - 1);
            if (x1 > x2) return;
            uint32_t* row = &buffer[y1 * width];
            for (int i = x1; i <= x2; ++i) row[i] = c;
        };

        while (y >= x) {
            drawScanline(yc + y, xc - x, xc + x);
            drawScanline(yc - y, xc - x, xc + x);
            drawScanline(yc + x, xc - y, xc + y);
            drawScanline(yc - x, xc - y, xc + y);
            x++;
            if (d > 0) { y--; d = d + 4 * (x - y) + 10; }
            else { d = d + 4 * x + 6; }
        }
    }

    void Window::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color) {
        drawLine(x1, y1, x2, y2, color);
        drawLine(x2, y2, x3, y3, color);
        drawLine(x3, y3, x1, y1, color);
    }

    // Optimized Scanline Triangle Fill
    void Window::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color) {
        // Transform and Clip Bounds first (Simplified: Using primitive clipping)
        // A robust implementation would clip the triangle polygon against the rect. 
        // Here we rely on scanline clipping for Simplicity + Speed upgrade.
        
        int tx1 = x1, ty1 = y1; transformCoordinates(tx1, ty1);
        int tx2 = x2, ty2 = y2; transformCoordinates(tx2, ty2);
        int tx3 = x3, ty3 = y3; transformCoordinates(tx3, ty3);

        int clipMinX = 0, clipMaxX = width;
        int clipMinY = 0, clipMaxY = height;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            clipMinX = p.rect.x + 1; clipMaxX = p.rect.x + p.rect.w - 1;
            clipMinY = p.rect.y + 21; clipMaxY = p.rect.y + p.rect.h - 1;
        }

        auto drawScanLine = [&](int y, int xLeft, int xRight) {
            if (y < clipMinY || y >= clipMaxY) return;
            if (xLeft > xRight) std::swap(xLeft, xRight);
            xLeft = std::max(xLeft, clipMinX);
            xRight = std::min(xRight, clipMaxX - 1);
            if (xLeft > xRight) return;
            
            uint32_t c = color.toInt();
            uint32_t* row = &buffer[y * width];
            for (int x = xLeft; x <= xRight; x++) row[x] = c;
        };

        // Sort vertices by Y
        if (ty1 > ty2) { std::swap(tx1, tx2); std::swap(ty1, ty2); }
        if (ty1 > ty3) { std::swap(tx1, tx3); std::swap(ty1, ty3); }
        if (ty2 > ty3) { std::swap(tx2, tx3); std::swap(ty2, ty3); }

        int totalHeight = ty3 - ty1;
        if (totalHeight == 0) return;

        for (int i = 0; i < totalHeight; i++) {
            int y = ty1 + i;
            bool secondHalf = i > ty2 - ty1 || ty2 == ty1;
            int segmentHeight = secondHalf ? ty3 - ty2 : ty2 - ty1;
            float alpha = (float)i / totalHeight;
            float beta = (float)(i - (secondHalf ? ty2 - ty1 : 0)) / segmentHeight;
            
            int A = (int)(tx1 + (tx3 - tx1) * alpha);
            int B = secondHalf ? (int)(tx2 + (tx3 - tx2) * beta) : (int)(tx1 + (tx2 - tx1) * beta);
            drawScanLine(y, A, B);
        }
    }

    // Include drawSprite implementation here or keep it in Header if inline
    // We already have a fast implementation in Window::drawSprite
    void Window::drawSprite(const Sprite& sprite) {
        if (!sprite.texture) return;

        // Transform position
        int startX = (int)sprite.x;
        int startY = (int)sprite.y;
        transformCoordinates(startX, startY);

        int clipMinX = 0, clipMaxX = width;
        int clipMinY = 0, clipMaxY = height;

        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            clipMinX = p.rect.x + 1;
            clipMaxX = p.rect.x + p.rect.w - 1;
            clipMinY = p.rect.y + 21;
            clipMaxY = p.rect.y + p.rect.h - 1;
        }

        // Calculate Sprite Dims
        int srcX = sprite.srcRect.x;
        int srcY = sprite.srcRect.y;
        int srcW = sprite.srcRect.w;
        int srcH = sprite.srcRect.h;
        float scale = sprite.scale;

        int destW = (int)(srcW * scale);
        int destH = (int)(srcH * scale);

        // Simple visibility check
        if (startX + destW < clipMinX || startX >= clipMaxX || startY + destH < clipMinY || startY >= clipMaxY) 
            return;

        // Clip destination rectangle
        int visibleStartX = std::max(startX, clipMinX);
        int visibleStartY = std::max(startY, clipMinY);
        int visibleEndX = std::min(startX + destW, clipMaxX);
        int visibleEndY = std::min(startY + destH, clipMaxY);

        int visibleWidth = visibleEndX - visibleStartX;
        int visibleHeight = visibleEndY - visibleStartY;

        if (visibleWidth <= 0 || visibleHeight <= 0) return;

        const std::vector<uint32_t>& srcPixels = sprite.texture->pixels;
        int texWidth = sprite.texture->width;

        // Precompute X mapping
        std::vector<int> xMapping(visibleWidth);
        for (int i = 0; i < visibleWidth; i++) {
            int destRelX = visibleStartX + i - startX;
            int srcRelX = (int)(destRelX / scale);
            xMapping[i] = std::min(srcRelX + srcX, texWidth - 1);
        }

        // Render Loop
        for (int j = 0; j < visibleHeight; j++) {
            int destRelY = visibleStartY + j - startY;
            int srcRelY = (int)(destRelY / scale);
            int srcPY = std::min(srcRelY + srcY, sprite.texture->height - 1);

            uint32_t* destRow = &buffer[(visibleStartY + j) * width + visibleStartX];
            const uint32_t* srcRowStart = &srcPixels[srcPY * texWidth];

            for (int i = 0; i < visibleWidth; i++) {
                uint32_t pixel = srcRowStart[xMapping[i]];
                if ((pixel & 0xFF000000) != 0) {
                    destRow[i] = pixel;
                }
            }
        }
    }
}
