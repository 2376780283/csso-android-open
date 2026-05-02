//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_PROGRESSBAR_H
#define HUD_PROGRESSBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/VectorImagePanel.h>
#include "igameevents.h"
#include <vgui/ISurface.h>
#include "VGuiMatSurface/IMatSystemSurface.h"

#define CIRCLE_SEGMENTS 64

namespace vgui
{

class CircularProgressBar : public Panel
{
    DECLARE_CLASS_SIMPLE( CircularProgressBar, Panel );

public:
    CircularProgressBar( Panel *parent, const char *name );
    ~CircularProgressBar();

    void SetProgress( float progress );
    float GetProgress() const { return m_flProgress; }

    void SetFgColor( Color color ) { m_FgColor = color; }
    void SetBgColor( Color color ) { m_BgColor = color; }

    enum ActionIcon
    {
        ICON_NONE = 0,
        ICON_DEFUSE,
        ICON_HOSTAGE,
    };

    void SetActionIcon( ActionIcon icon );

    virtual void Paint();
    virtual void PerformLayout();

protected:
    void DrawFilledRing( int centerX, int centerY, int outerRadius, int innerRadius,
                         float startAngle, float endAngle, Color color );

    float m_flProgress;
    int m_iLineWidth;
    Color m_FgColor;
    Color m_BgColor;

    VectorImagePanel* m_pActionIcon;
    ActionIcon m_nCurrentIcon;

    CPanelAnimationVar( int, m_iRadius, "radius", "30" );
    CPanelAnimationVar( int, m_iThickness, "thickness", "6" );
};

} // namespace vgui

#endif // HUD_PROGRESSBAR_H