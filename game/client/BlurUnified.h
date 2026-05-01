#ifndef BLUR_UNIFIED_H
#define BLUR_UNIFIED_H

#include "tier1/refcount.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/MaterialSystemUtil.h"
// Unified blur helper that works both in-game and in UI menus
class CUnifiedBlurHelper
{
public:
    CUnifiedBlurHelper();
    ~CUnifiedBlurHelper();

    // Initialize resources (can be called multiple times safely)
    void InitResources();

    // Apply blur to a screen area
    // In-game mode: Uses high-quality material-based blur
    // Menu mode: Falls back to VGUI-based blur if needed
    void ApplyBlur(int x, int y, int w, int h, float fStrength = 1.0f, bool bForceMenuMode = false);

    // Clean up resources
    void Cleanup();

    // Check if we can use high-quality blur (in-game)
    bool CanUseHighQualityBlur() const { return m_bCanUseHighQuality; }

private:
    // High-quality (in-game) blur implementation
    void ApplyHighQualityBlur(int x, int y, int w, int h, float fStrength);

    // Fallback (menu) blur implementation
    void ApplyFallbackBlur(int x, int y, int w, int h, float fStrength);

    CMaterialReference m_xblur_mat;
    CMaterialReference m_yblur_mat;
    CTextureReference m_dest_rt0;
    CTextureReference m_dest_rt1;
    CTextureReference m_BlurRT;

    bool m_bInitialized;
    bool m_bCanUseHighQuality;  // Detect if we're in-game
};

#endif
