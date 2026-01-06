#include "Termgl.h"

namespace termgl {

    // ============================================================================
    // INPUT IMPLEMENTATION
    // ============================================================================

    bool Window::isKeyDown(int key) const { return keys[key & 0xFF]; }
    bool Window::isKeyPressed(int key) const { return keys[key & 0xFF] && !prevKeys[key & 0xFF]; }

    bool Window::isControlDown() const { return (GetKeyState(VK_CONTROL) & 0x8000) != 0; }
    bool Window::isShiftDown() const { return (GetKeyState(VK_SHIFT) & 0x8000) != 0; }
    bool Window::isAltDown() const { return (GetKeyState(VK_MENU) & 0x8000) != 0; }

    bool Window::isMouseLeftDown() const { return mouseLeft; }
    bool Window::isMouseRightDown() const { return mouseRight; }
    
    Vec2 Window::getMousePos() const { 
        int x = mouseX;
        int y = mouseY;
        // Transform based on active partition to make coordinates relative
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            x -= p.rect.x;
            y -= (p.rect.y + 20);
        }
        return Vec2(x, y); 
    }

    int Window::getMouseScrollDelta() const { return mouseScrollDelta; }
    int Window::getMouseHScrollDelta() const { return mouseHScrollDelta; }
}
