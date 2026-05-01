//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "c_cs_player.h"
#include "clientmode_csnormal.h"
#include "hud_progressbar.h"

#include "tier0/memdbgon.h"

extern ConVar cl_draw_only_deathnotices;

class CHudProgressBar : public CHudElement, public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE( CHudProgressBar, vgui::EditablePanel );

    CHudProgressBar( const char *name );

    virtual bool ShouldDraw();
    virtual void OnThink();

private:
    vgui::Label* m_pActionText;
    vgui::Label* m_pTimerText;
    vgui::CircularProgressBar* m_pActionProgress;

    vgui::CircularProgressBar::ActionIcon m_nLastIcon;

    CPanelAnimationVar( Color, m_clrStart, "ColorStart", "255 0 0 255" );
    CPanelAnimationVar( Color, m_clrMiddle, "ColorMiddle", "255 255 0 255" );
    CPanelAnimationVar( Color, m_clrEnd, "ColorEnd", "0 128 0 255" );
};

DECLARE_HUDELEMENT( CHudProgressBar );

CHudProgressBar::CHudProgressBar( const char *name ) :
    vgui::EditablePanel( NULL, "HudProgressBar" ), CHudElement( name )
{
    vgui::Panel *pParent = g_pClientMode->GetViewport();
    SetParent( pParent );

    m_pActionText = new vgui::Label( this, "ActionText", L" " );
    m_pTimerText = new vgui::Label( this, "TimerText", L" " );
    m_pActionProgress = new vgui::CircularProgressBar( this, "ActionProgress" );

    m_nLastIcon = vgui::CircularProgressBar::ICON_NONE;

    SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_WEAPONSELECTION );

    LoadControlSettings( "resource/hud/progressbar.res" );
}

bool CHudProgressBar::ShouldDraw()
{
    if ( cl_draw_only_deathnotices.GetBool() )
        return false;

    C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

    if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
    {
        C_BaseEntity *pTarget = pPlayer->GetObserverTarget();

        if ( pTarget && pTarget->IsPlayer() )
        {
            pPlayer = ToCSPlayer( pTarget );

            if ( !pPlayer->IsAlive() )
                return false;
        }
        else
            return false;
    }

    if ( !pPlayer || pPlayer->m_iProgressBarDuration == 0 || pPlayer->m_lifeState == LIFE_DEAD )
    {
        if ( m_nLastIcon != vgui::CircularProgressBar::ICON_NONE )
        {
            m_pActionProgress->SetActionIcon( vgui::CircularProgressBar::ICON_NONE );
            m_nLastIcon = vgui::CircularProgressBar::ICON_NONE;
        }
        return false;
    }

    return CHudElement::ShouldDraw();
}

void CHudProgressBar::OnThink()
{
    bool bSpectating = false;
    C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();

    if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
    {
        C_BaseEntity* pTarget = pPlayer->GetObserverTarget();

        if ( pTarget && pTarget->IsPlayer() )
        {
            pPlayer = ToCSPlayer( pTarget );
            bSpectating = true;
        }
    }

    if ( !pPlayer || pPlayer->m_iProgressBarDuration == 0 || !pPlayer->IsAlive() )
    {
        return;
    }

    vgui::CircularProgressBar::ActionIcon newIcon = vgui::CircularProgressBar::ICON_NONE;

    if ( pPlayer->m_bIsGrabbingHostage )
    {
        newIcon = vgui::CircularProgressBar::ICON_HOSTAGE;
    }
    else if ( pPlayer->m_bIsDefusing )
    {
        newIcon = vgui::CircularProgressBar::ICON_DEFUSE;
    }

    if ( newIcon != m_nLastIcon )
    {
        m_pActionProgress->SetActionIcon( newIcon );
        m_nLastIcon = newIcon;
    }

    if ( bSpectating )
    {
        wchar_t wszLocalized[128];
        wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
        g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof( wszPlayerName ) );

        if ( pPlayer->m_bIsGrabbingHostage )
        {
            g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_HostageText" ), 1, wszPlayerName );
        }
        else
        {
            if ( pPlayer->HasDefuser() )
                g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_DefuseText" ), 1, wszPlayerName );
            else
                g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_DefuseText_NoKit" ), 1, wszPlayerName );
        }

        m_pActionText->SetText( wszLocalized );
    }
    else
    {
        if ( pPlayer->m_bIsGrabbingHostage )
        {
            m_pActionText->SetText( "#Cstrike_Progress_HostageText" );
        }
        else
        {
            if ( pPlayer->HasDefuser() )
                m_pActionText->SetText( "#Cstrike_Progress_DefuseText" );
            else
                m_pActionText->SetText( "#Cstrike_Progress_DefuseText_NoKit" );
        }
    }

    float flTimeLeft =
    ( pPlayer->m_flProgressBarStartTime + (float)pPlayer->m_iProgressBarDuration ) -
    pPlayer->m_flSimulationTime;
    flTimeLeft = MAX( flTimeLeft, 0.0f );


    wchar_t wszTimer[16];
    V_snwprintf( wszTimer, sizeof( wszTimer ), L"00:%06.3f", flTimeLeft );
    m_pTimerText->SetText( wszTimer );

    float flElapsed =
    pPlayer->m_flSimulationTime - pPlayer->m_flProgressBarStartTime;

    float flPercentage =
    flElapsed / (float)pPlayer->m_iProgressBarDuration;

    flPercentage = clamp( flPercentage, 0.0f, 1.0f );

    m_pActionProgress->SetProgress( flPercentage );

    float flR, flG, flB;
    if ( flPercentage > 0.5f )
    {
        flR = ( ( m_clrMiddle.r() - m_clrEnd.r() ) * ( 1.0f - ( ( flPercentage - 0.5f ) * 2.0f ) ) ) + m_clrEnd.r();
        flG = ( ( m_clrMiddle.g() - m_clrEnd.g() ) * ( 1.0f - ( ( flPercentage - 0.5f ) * 2.0f ) ) ) + m_clrEnd.g();
        flB = ( ( m_clrMiddle.b() - m_clrEnd.b() ) * ( 1.0f - ( ( flPercentage - 0.5f ) * 2.0f ) ) ) + m_clrEnd.b();
    }
    else
    {
        flR = ( ( m_clrStart.r() - m_clrMiddle.r() ) * ( 1.0f - ( flPercentage / 0.5f ) ) ) + m_clrMiddle.r();
        flG = ( ( m_clrStart.g() - m_clrMiddle.g() ) * ( 1.0f - ( flPercentage / 0.5f ) ) ) + m_clrMiddle.g();
        flB = ( ( m_clrStart.b() - m_clrMiddle.b() ) * ( 1.0f - ( flPercentage / 0.5f ) ) ) + m_clrMiddle.b();
    }

    m_pActionProgress->SetFgColor( Color( flR, flG, flB, 255 ) );
}

//-----------------------------------------------------------------------------
// CircularProgressBar
//-----------------------------------------------------------------------------
DECLARE_BUILD_FACTORY( CircularProgressBar );

CircularProgressBar::CircularProgressBar( Panel *parent, const char *name ) : BaseClass( parent, name )
{
    m_flProgress = 0.0f;
    m_iLineWidth = 8;
    m_FgColor = Color( 255, 255, 255, 255 );
    m_BgColor = Color( 50, 50, 50, 180 );

    m_pActionIcon = new VectorImagePanel( this, "ActionIcon" );
    m_pActionIcon->SetMirrorX( true );
    m_pActionIcon->SetVisible( false );

    m_nCurrentIcon = ICON_NONE;

    SetPaintBackgroundEnabled( false );
    SetPaintBorderEnabled( false );
}

CircularProgressBar::~CircularProgressBar()
{
}

void CircularProgressBar::SetProgress( float progress )
{
    m_flProgress = clamp( progress, 0.0f, 1.0f );
}

void CircularProgressBar::SetActionIcon( ActionIcon icon )
{
    if ( m_nCurrentIcon == icon )
        return;

    m_nCurrentIcon = icon;

    if ( icon == ICON_NONE )
    {
        m_pActionIcon->SetVisible( false );
        return;
    }

    const char *pszIconPath = NULL;

    switch ( icon )
    {
        case ICON_DEFUSE:
            pszIconPath = "materials/vgui/weapons/svg/defuser.svg";
            break;
        case ICON_HOSTAGE:
            pszIconPath = "materials/vgui/hud/svg/hostage_transit.svg";
            break;
        default:
            m_pActionIcon->SetVisible( false );
            return;
    }

    int innerDiameter = ( m_iRadius - m_iThickness ) * 2;
    int iconSize = innerDiameter / 2;
    if ( iconSize < 16 )
        iconSize = 16;

    m_pActionIcon->SetRenderSize( iconSize, iconSize );
    m_pActionIcon->SetTexture( pszIconPath );
    m_pActionIcon->SetVisible( true );

    InvalidateLayout();
}

void CircularProgressBar::PerformLayout()
{
    BaseClass::PerformLayout();

    if ( m_pActionIcon && m_pActionIcon->IsVisible() )
    {
        int wide, tall;
        GetSize( wide, tall );

        int iconW, iconH;
        m_pActionIcon->GetSize( iconW, iconH );

        m_pActionIcon->SetPos( ( wide - iconW ) / 2, ( tall - iconH ) / 2 );
        m_pActionIcon->SetFgColor( Color( 255, 255, 255, 255 ) );
    }
}

void CircularProgressBar::Paint()
{
    int wide, tall;
    GetSize( wide, tall );

    int centerX = wide / 2;
    int centerY = tall / 2;
    int outerRadius = m_iRadius;
    int innerRadius = m_iRadius - m_iThickness;

    DrawFilledRing( centerX, centerY, outerRadius, innerRadius, 0.0f, 360.0f, m_BgColor );

    if ( m_flProgress > 0.0f )
    {
        float halfArc = 180.0f * m_flProgress;
        
        DrawFilledRing( centerX, centerY, outerRadius, innerRadius, 90.0f - halfArc, 90.0f, m_FgColor );
        
        DrawFilledRing( centerX, centerY, outerRadius, innerRadius, 90.0f, 90.0f + halfArc, m_FgColor );
    }
}

void CircularProgressBar::DrawFilledRing( int centerX, int centerY, int outerRadius, int innerRadius,
                                           float startAngle, float endAngle, Color color )
{
    if ( startAngle > endAngle )
    {
        float temp = startAngle;
        startAngle = endAngle;
        endAngle = temp;
    }

    float angleRange = endAngle - startAngle;
    int numSegments = max( 4, (int)( ( angleRange / 360.0f ) * CIRCLE_SEGMENTS ) );

    surface()->DrawSetColor( color );

    for ( int i = 0; i < numSegments; i++ )
    {
        float angle1 = DEG2RAD( startAngle + ( angleRange * i / (float)numSegments ) );
        float angle2 = DEG2RAD( startAngle + ( angleRange * ( i + 1 ) / (float)numSegments ) );

        float cos1 = cos( angle1 );
        float sin1 = sin( angle1 );
        float cos2 = cos( angle2 );
        float sin2 = sin( angle2 );

        Vertex_t quad[4];
        quad[0].m_Position = Vector2D( centerX + outerRadius * cos1, centerY + outerRadius * sin1 );
        quad[1].m_Position = Vector2D( centerX + outerRadius * cos2, centerY + outerRadius * sin2 );
        quad[2].m_Position = Vector2D( centerX + innerRadius * cos2, centerY + innerRadius * sin2 );
        quad[3].m_Position = Vector2D( centerX + innerRadius * cos1, centerY + innerRadius * sin1 );

        g_pMatSystemSurface->DrawFilledPolygon( 4, quad );
    }
}