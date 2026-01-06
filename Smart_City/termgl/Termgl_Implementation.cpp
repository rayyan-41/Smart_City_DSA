// ============================================================================
// TERMGL IMPLEMENTATION - UNITY BUILD
// ============================================================================
// This file includes all termgl implementation files for a single compilation unit.
// This approach (unity build) can improve compile times and ensures proper linking.
// ============================================================================

// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Ensure WM_MOUSEHWHEEL is defined for older SDKs
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

// STB_IMAGE_IMPLEMENTATION must be defined exactly once before including stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Include the main header (pulls in all declarations)
#include "Termgl.h"

// ============================================================================
// Include all implementation files
// ============================================================================

#include "Termgl_Window.cpp"
#include "Termgl_Renderer.cpp"
#include "Termgl_Input.cpp"
#include "Termgl_Texture.cpp"
#include "Termgl_UI.cpp"