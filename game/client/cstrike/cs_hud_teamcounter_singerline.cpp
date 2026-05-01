//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD team counter with player avatars - Singerline (single row) mode
//          Displays round timer, win counts, and per-player avatar tiles
//          with team outline + skull for dead players.
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_plantedc4.h"
#include "c_cs_playerresource.h"
#include "cs_gamerules.h"
#include "cs_shareddefs.h"
#include "vgui_avatarimage.h"
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>

extern CUtlVector<C_PlantedC4*> g_PlantedC4s;

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// ConVars
//-----------------------------------------------------------------------------

extern ConVar hud_playercount_pos;
extern ConVar hud_teamcounter_style;

static ConVarRef mp_freezetime_ref( "mp_freezetime" );

//-----------------------------------------------------------------------------
// Constants for layout - Singerline mode (single row)
//-----------------------------------------------------------------------------
#define TC_MAX_AVATAR_SLOTS     6       // max players to show per team (12 total)
#define TC_AVATARS_PER_ROW      6       // singerline mode: all 6 in single row

// Base pixel sizes designed at 1080p; all multiplied by flScale at runtime.
#define TC_BASE_AVATAR_SIZE     55      // avatar square size (px at 1080p) - Singerline mode
#define TC_BASE_AVATAR_GAP      3       // gap between avatar tiles
#define TC_BASE_OUTLINE         4       // colored border thickness around each tile
#define TC_BASE_CENTER_W        84      // width of center timer/score block
#define TC_BASE_CENTER_GAP      6       // gap between team block and center block

// Team outline colors (RGBA)
static const Color TC_CT_OUTLINE_COLOR  ( 150, 200, 255, 220 );  // CT blue
static const Color TC_T_OUTLINE_COLOR   ( 226, 212, 157, 220 );  // T yellow
static const Color TC_DEAD_BG_COLOR     ( 15,  15,  15, 200 );   // dark bg for dead slots

static const Color TC_OWN_OUTLINE_COLOR  ( 255, 255, 255, 128 );

//=============================================================================
//
// CHudTeamCounterSingerline
//
//=============================================================================
class CHudTeamCounterSingerline : public CHudElement, public EditablePanel
{
        DECLARE_CLASS_SIMPLE( CHudTeamCounterSingerline, EditablePanel );

public:
        CHudTeamCounterSingerline( const char *pElementName );
        virtual ~CHudTeamCounterSingerline() {}

        virtual void Init( void );
        virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
        virtual void ApplySchemeSettings( IScheme *pScheme );
        virtual void PerformLayout();
        virtual void PaintBackground();
        virtual void Reset( void );
        virtual bool ShouldDraw();
        virtual void OnThink();

private:
        //-------------------------------------------------------------------------
        // Internal helpers
        //-------------------------------------------------------------------------
        void    UpdateAvatarMode();
        void    Layout();
        int             ScalePx( float basePixels ) const;

        //-------------------------------------------------------------------------
        // Labels - DO NOT TOUCH
        //-------------------------------------------------------------------------
        Label           *m_pRoundTimerLabel;            // Round timer display
        Label           *m_pCTWinCounterLabel;          // CT win score
        Label           *m_pTWinCounterLabel;           // T win score
        VectorImagePanel *m_pBombIcon;                  // C4 bomb icon (shown when planted)

        //-------------------------------------------------------------------------
        // Per-player avatar slots
        //-------------------------------------------------------------------------
        struct AvatarSlot_t
        {
                CAvatarImagePanel       *pAvatar;               // draws the player's Steam avatar
                ImagePanel                      *pSkull;                // drawn when player is dead
                int                                     iLastPlayerIndex;       // tracks which player was last assigned
                bool                            bIsAlive;               // alive state for PaintBackground
        };
        AvatarSlot_t m_CTSlots[ TC_MAX_AVATAR_SLOTS ];
        AvatarSlot_t m_TSlots [ TC_MAX_AVATAR_SLOTS ];

        // Slot rects in panel-local coords, computed in Layout.
        struct SlotRect_t { int x, y, w, h; };
        SlotRect_t m_CTSlotRects[ TC_MAX_AVATAR_SLOTS ];
        SlotRect_t m_TSlotRects [ TC_MAX_AVATAR_SLOTS ];
        int  m_iNumActiveCTSlots;       // how many CT slots have a player
        int  m_iNumActiveTSlots;

        //-------------------------------------------------------------------------
        // Cached layout values for PaintBackground
        //-------------------------------------------------------------------------
        int  m_iAvatarSize;                     // avatar square in pixels (scaled)
        int  m_iOutlineThick;           // border thickness in pixels (scaled)

        //-------------------------------------------------------------------------
        // Shared state
        //-------------------------------------------------------------------------
        int  m_iRoundTime;
        bool m_bIsAtTheBottom;
        int  m_iRoundTimerLabelTall;
        int  m_iCTWinCounterLabelTall;
        int  m_iTWinCounterLabelTall;

        //-------------------------------------------------------------------------
        // C4 colors (from original script - CPanelAnimationVar style)
        //-------------------------------------------------------------------------
        CPanelAnimationVar( Color, m_clrC4Planted, "C4PlantedColor", "Red" );
        CPanelAnimationVar( Color, m_clrC4Defused, "C4DefusedColor", "SteamLightGreen" );
};

DECLARE_HUDELEMENT( CHudTeamCounterSingerline );


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CHudTeamCounterSingerline::CHudTeamCounterSingerline( const char *pElementName )
        : CHudElement( pElementName ), EditablePanel( NULL, "HudTeamCounter" )
{
        vgui::Panel *pParent = g_pClientMode->GetViewport();
        SetParent( pParent );
        SetHiddenBits( HIDEHUD_PLAYERDEAD );

        //
        // Labels - DO NOT MODIFY
        //
        m_pRoundTimerLabel      = new Label( this, "LineRoundTimerLabel",   "0:00" );
        m_pCTWinCounterLabel    = new Label( this, "LineCTWinCounterLabel", "0" );
        m_pTWinCounterLabel     = new Label( this, "LineTWinCounterLabel",  "0" );

        //
        // Bomb icon - configured entirely through teamcounter.res (image, size, position)
        //
        m_pBombIcon = new VectorImagePanel( this, "LineBombIcon" );

        //
        // Per-player avatar/skull slots
        //
        for ( int i = 0; i < TC_MAX_AVATAR_SLOTS; i++ )
        {
                // CT slot
                m_CTSlots[i].pAvatar = new CAvatarImagePanel( this, VarArgs( "CT_Av_%d", i ) );
                m_CTSlots[i].pAvatar->SetShouldScaleImage( true );
                m_CTSlots[i].pAvatar->SetShouldDrawFriendIcon( false );
                m_CTSlots[i].pAvatar->SetVisible( false );
                m_CTSlots[i].pAvatar->SetZPos( 2 );

                m_CTSlots[i].pSkull = new ImagePanel( this, VarArgs( "CT_Sk_%d", i ) );
                m_CTSlots[i].pSkull->SetImage( "hud/teamcounter_aliveskull" );
                m_CTSlots[i].pSkull->SetVisible( false );
                m_CTSlots[i].pSkull->SetZPos( 4 );
                {
                        KeyValues *kv = new KeyValues( "Panel" );
                        kv->SetInt( "scaleImage", 1 );
                        m_CTSlots[i].pSkull->ApplySettings( kv );
                        kv->deleteThis();
                }

                m_CTSlots[i].iLastPlayerIndex = 0;
                m_CTSlots[i].bIsAlive = false;

                // T slot
                m_TSlots[i].pAvatar = new CAvatarImagePanel( this, VarArgs( "T_Av_%d", i ) );
                m_TSlots[i].pAvatar->SetShouldScaleImage( true );
                m_TSlots[i].pAvatar->SetShouldDrawFriendIcon( false );
                m_TSlots[i].pAvatar->SetVisible( false );
                m_TSlots[i].pAvatar->SetZPos( 2 );

                m_TSlots[i].pSkull = new ImagePanel( this, VarArgs( "T_Sk_%d", i ) );
                m_TSlots[i].pSkull->SetImage( "hud/teamcounter_aliveskull" );
                m_TSlots[i].pSkull->SetVisible( false );
                m_TSlots[i].pSkull->SetZPos( 4 );
                {
                        KeyValues *kv = new KeyValues( "Panel" );
                        kv->SetInt( "scaleImage", 1 );
                        m_TSlots[i].pSkull->ApplySettings( kv );
                        kv->deleteThis();
                }

                m_TSlots[i].iLastPlayerIndex = 0;
                m_TSlots[i].bIsAlive = false;

                m_CTSlotRects[i] = { 0, 0, 0, 0 };
                m_TSlotRects[i]  = { 0, 0, 0, 0 };
        }

        m_iAvatarSize           = TC_BASE_AVATAR_SIZE;
        m_iOutlineThick         = TC_BASE_OUTLINE;
        m_iNumActiveCTSlots = 0;
        m_iNumActiveTSlots      = 0;

        // Load label settings from .res file (fonts, colors, etc.)
        LoadControlSettings( "resource/hud/teamcountersingerline.res" );
}


//-----------------------------------------------------------------------------
// Init
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::Init( void )
{
        m_iRoundTime            = 0;
        m_bIsAtTheBottom        = false;

        // Initialize label tall values from .res file (will be used in Layout)
        // These will be refreshed in Layout() each time
        int wide, tall;
        m_pRoundTimerLabel->GetSize( wide, m_iRoundTimerLabelTall );
        m_pCTWinCounterLabel->GetSize( wide, m_iCTWinCounterLabelTall );
        m_pTWinCounterLabel->GetSize( wide, m_iTWinCounterLabelTall );
}

void CHudTeamCounterSingerline::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "resource/hud/teamcountersingerline.res" );

	// force recalculation of some stuff
	m_bIsAtTheBottom = false;
}

//-----------------------------------------------------------------------------
// ApplySchemeSettings – set default avatars
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::ApplySchemeSettings( IScheme *pScheme )
{
        BaseClass::ApplySchemeSettings( pScheme );

        // Set default (fallback) avatars for each slot
        IImage *pDefaultCT = vgui::scheme()->GetImage( CSTRIKE_DEFAULT_CT_AVATAR, true );
        IImage *pDefaultT  = vgui::scheme()->GetImage( CSTRIKE_DEFAULT_T_AVATAR,  true );

        for ( int i = 0; i < TC_MAX_AVATAR_SLOTS; i++ )
        {
                if ( pDefaultCT ) m_CTSlots[i].pAvatar->SetDefaultAvatar( pDefaultCT );
                if ( pDefaultT  ) m_TSlots[i].pAvatar->SetDefaultAvatar( pDefaultT  );
        }
}


//-----------------------------------------------------------------------------
// ScalePx – proportionally scale a base 1080p pixel value to the current res
//-----------------------------------------------------------------------------
int CHudTeamCounterSingerline::ScalePx( float basePixels ) const
{
        float flScale = (float)ScreenHeight() / 1080.0f;
        int result = (int)( basePixels * flScale );
        return MAX( 1, result );
}


//-----------------------------------------------------------------------------
// Layout
//
// Calculates every pixel position for all controls.
// Everything is derived from ScreenHeight() so it works at any resolution.
//
// Layout (top view) - Singerline mode (single row):
//
//   [ CT team block (1 row, 6 slots) ] [ center: timer / score ] [ T team block (1 row, 6 slots) ]
//
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::Layout()
{
        // --- Scaled sizes ---
        m_iAvatarSize           = ScalePx( TC_BASE_AVATAR_SIZE );
        m_iOutlineThick         = ScalePx( TC_BASE_OUTLINE );
        int avatarGap           = ScalePx( TC_BASE_AVATAR_GAP );
        int centerW                     = ScalePx( TC_BASE_CENTER_W );
        int centerGap           = ScalePx( TC_BASE_CENTER_GAP );

        // Tile = avatar + border on all 4 sides
        int tileW = m_iAvatarSize + 2 * m_iOutlineThick;
        int tileH = m_iAvatarSize + 2 * m_iOutlineThick;

        // Singerline mode: single row
        int numRows = 1;

        // Width/height of one team's entire block
        int teamBlockW = TC_AVATARS_PER_ROW * tileW + ( TC_AVATARS_PER_ROW - 1 ) * avatarGap;
        int teamBlockH = numRows * tileH + ( numRows - 1 ) * avatarGap;

        // Total panel size
        int panelW = teamBlockW + centerGap + centerW + centerGap + teamBlockW;
        int panelH = teamBlockH;

        // Panel position: centered horizontally, at top (or bottom) of screen
        int panelX = ( ScreenWidth() - panelW ) / 2;
        int panelY;
        if ( m_bIsAtTheBottom )
                panelY = ScreenHeight() - panelH - ScalePx( 4 );
        else
                panelY = ScalePx( 2 );   // 2px top margin

        SetPos( panelX, panelY );
        SetSize( panelW, panelH );

        // --- CT block origin (left side) ---
        // Singerline mode: all slots in single row, left to right
        int ctOriginX = 0;

        for ( int i = 0; i < TC_MAX_AVATAR_SLOTS; i++ )
        {
                int row = 0;  // single row
                int col = i;  // all in one row, left to right
                col = ( TC_AVATARS_PER_ROW - 1 ) - col;  // right-to-left

                int sx = ctOriginX + col * ( tileW + avatarGap );
                int sy = row * ( tileH + avatarGap );

                m_CTSlotRects[i] = { sx, sy, tileW, tileH };

                int ax = sx + m_iOutlineThick;
                int ay = sy + m_iOutlineThick;
                m_CTSlots[i].pAvatar->SetBounds( ax, ay, m_iAvatarSize, m_iAvatarSize );
                m_CTSlots[i].pAvatar->SetAvatarSize( m_iAvatarSize, m_iAvatarSize );
                m_CTSlots[i].pSkull->SetBounds( ax, ay, m_iAvatarSize, m_iAvatarSize );
        }

        // --- Center block: Labels position dynamically based on team block size ---
        // Labels must be repositioned because panel width changes in singerline mode
        // Calculate center block position
        int centerBlockX = teamBlockW + centerGap;

        // Get Label dimensions from .res file (wide values)
        int roundTimerWide, ctWinWide, tWinWide;
        m_pRoundTimerLabel->GetSize( roundTimerWide, m_iRoundTimerLabelTall );
        m_pCTWinCounterLabel->GetSize( ctWinWide, m_iCTWinCounterLabelTall );
        m_pTWinCounterLabel->GetSize( tWinWide, m_iTWinCounterLabelTall );

        // Position labels centered within the center block
        // RoundTimerLabel: centered in center block
        int roundTimerX = centerBlockX + ( centerW - roundTimerWide ) / 2;
        m_pRoundTimerLabel->SetPos( roundTimerX, 0 );

        // BombIcon: centered in center block
        int bombWide = ScalePx( 30 );
        int bombTall = ScalePx( 30 );  // Match timer label height
        int bombX = centerBlockX + ( centerW - bombWide ) / 2;
        m_pBombIcon->SetBounds( bombX, ScalePx( -2 ), bombWide, bombTall );

        // CTWinCounterLabel: centered in left half of center block
        int ctWinX = centerBlockX + ( centerW / 2 - ctWinWide ) / 2;
        m_pCTWinCounterLabel->SetPos( ctWinX, ScalePx( 29 ) );

        // TWinCounterLabel: centered in right half of center block
        int tWinX = centerBlockX + centerW / 2 + ( 2 + centerW / 2 - tWinWide ) / 2;
        m_pTWinCounterLabel->SetPos( tWinX + 1, ScalePx( 29 ) );

        // --- T block origin (right side) ---
        int centerOriginX = teamBlockW + centerGap;
        int tOriginX = centerOriginX + centerW + centerGap;

        // Singerline mode: single row for T block
        for ( int i = 0; i < TC_MAX_AVATAR_SLOTS; i++ )
        {
                int row = 0;
                int col = i;

                int sx = tOriginX + col * ( tileW + avatarGap );
                int sy = row * ( tileH + avatarGap );

                m_TSlotRects[i] = { sx, sy, tileW, tileH };

                int ax = sx + m_iOutlineThick;
                int ay = sy + m_iOutlineThick;
                m_TSlots[i].pAvatar->SetBounds( ax, ay, m_iAvatarSize, m_iAvatarSize );
                m_TSlots[i].pAvatar->SetAvatarSize( m_iAvatarSize, m_iAvatarSize );
                m_TSlots[i].pSkull->SetBounds( ax, ay, m_iAvatarSize, m_iAvatarSize );
        }
}


//-----------------------------------------------------------------------------
// PerformLayout – called on init and every resolution change
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::PerformLayout()
{
        BaseClass::PerformLayout();
        Layout();
}


//-----------------------------------------------------------------------------
// PaintBackground
//
// Draws the colored team outlines around each active avatar slot.
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::PaintBackground()
{
        BaseClass::PaintBackground();

        vgui::ISurface *pSurface = vgui::surface();
        if ( !pSurface )
                return;

        C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
        int iLocalIndex = pLocalPlayer ? pLocalPlayer->entindex() : -1;

        // --- CT slot outlines (blue) - ONLY for ALIVE players ---
        for ( int i = 0; i < m_iNumActiveCTSlots; i++ )
        {
                // Skip dead players - they only show skull, no outline/box
                if ( !m_CTSlots[i].bIsAlive )
                        continue;

                const SlotRect_t &r = m_CTSlotRects[i];

                // Outer border (own color or team color)
                if ( m_CTSlots[i].iLastPlayerIndex == iLocalIndex )
                        pSurface->DrawSetColor( TC_OWN_OUTLINE_COLOR );
                else
                        pSurface->DrawSetColor( TC_CT_OUTLINE_COLOR );

                pSurface->DrawFilledRect( r.x, r.y, r.x + r.w, r.y + r.h );

                // Inner fill (dark background)
                pSurface->DrawSetColor( TC_DEAD_BG_COLOR );
                pSurface->DrawFilledRect(
                        r.x + m_iOutlineThick,
                        r.y + m_iOutlineThick,
                        r.x + r.w - m_iOutlineThick,
                        r.y + r.h - m_iOutlineThick );
        }

        // --- T slot outlines (green) - ONLY for ALIVE players ---
        for ( int i = 0; i < m_iNumActiveTSlots; i++ )
        {
                // Skip dead players - they only show skull, no outline/box
                if ( !m_TSlots[i].bIsAlive )
                        continue;

                const SlotRect_t &r = m_TSlotRects[i];

                // Outer border (own color or team color)
                if ( m_TSlots[i].iLastPlayerIndex == iLocalIndex )
                        pSurface->DrawSetColor( TC_OWN_OUTLINE_COLOR );
                else
                        pSurface->DrawSetColor( TC_T_OUTLINE_COLOR );

                pSurface->DrawFilledRect( r.x, r.y, r.x + r.w, r.y + r.h );

                pSurface->DrawSetColor( TC_DEAD_BG_COLOR );
                pSurface->DrawFilledRect(
                        r.x + m_iOutlineThick,
                        r.y + m_iOutlineThick,
                        r.x + r.w - m_iOutlineThick,
                        r.y + r.h - m_iOutlineThick );
        }
}


//-----------------------------------------------------------------------------
// Reset
//-----------------------------------------------------------------------------
void CHudTeamCounterSingerline::Reset()
{
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );
}


//-----------------------------------------------------------------------------
// ShouldDraw
//-----------------------------------------------------------------------------
bool CHudTeamCounterSingerline::ShouldDraw()
{
        // Part of hud_teamcounter_style = 1
        if ( hud_teamcounter_style.GetInt() != 1 )
                return false;

        // Use this single-row layout when there are 10 or fewer players
        int iPlayerCount = 0;
        if ( g_PR )
        {
                for ( int i = 1; i <= MAX_PLAYERS; i++ )
                {
                        if ( g_PR->IsConnected( i ) && ( g_PR->GetTeam( i ) == TEAM_CT || g_PR->GetTeam( i ) == TEAM_TERRORIST ) )
                        {
                                iPlayerCount++;
                        }
                }
        }

        if ( iPlayerCount > 10 )
                return false;

        C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
        if ( !pPlayer )
                return false;

        if ( pPlayer->IsObserver() )
                return false;
        return true;
}


//=============================================================================
// UpdateAvatarMode – assign players to slots, update alive/dead state
//=============================================================================
void CHudTeamCounterSingerline::UpdateAvatarMode()
{
        if ( !g_PR )
                return;

        // Build per-team player index lists
        CUtlVector<int> ctList, tList;
        ctList.EnsureCapacity( TC_MAX_AVATAR_SLOTS );
        tList.EnsureCapacity( TC_MAX_AVATAR_SLOTS );

        for ( int i = 1; i <= MAX_PLAYERS; i++ )
        {
                if ( !g_PR->IsConnected(i) )
                        continue;

                int team = g_PR->GetTeam(i);
                if ( team == TEAM_CT && ctList.Count() < TC_MAX_AVATAR_SLOTS )
                        ctList.AddToTail(i);
                else if ( team == TEAM_TERRORIST && tList.Count() < TC_MAX_AVATAR_SLOTS )
                        tList.AddToTail(i);
        }

        m_iNumActiveCTSlots = ctList.Count();
        m_iNumActiveTSlots  = tList.Count();

        // --- Update CT slots ---
        for ( int slot = 0; slot < TC_MAX_AVATAR_SLOTS; slot++ )
        {
                if ( slot < ctList.Count() )
                {
                        int playerIndex = ctList[ slot ];
                        bool bAlive     = g_PR->IsAlive( playerIndex );

                        if ( m_CTSlots[slot].iLastPlayerIndex != playerIndex )
                        {
                                m_CTSlots[slot].iLastPlayerIndex = playerIndex;
                                m_CTSlots[slot].pAvatar->SetPlayer( playerIndex, k_EAvatarSize32x32 );
                        }

                        m_CTSlots[slot].bIsAlive = bAlive;
                        m_CTSlots[slot].pAvatar->SetVisible( bAlive  );
                        m_CTSlots[slot].pSkull->SetVisible( !bAlive  );
                }
                else
                {
                        m_CTSlots[slot].iLastPlayerIndex = 0;
                        m_CTSlots[slot].bIsAlive = false;
                        m_CTSlots[slot].pAvatar->SetVisible( false );
                        m_CTSlots[slot].pSkull->SetVisible( false );
                }
        }

        // --- Update T slots ---
        for ( int slot = 0; slot < TC_MAX_AVATAR_SLOTS; slot++ )
        {
                if ( slot < tList.Count() )
                {
                        int playerIndex = tList[ slot ];
                        bool bAlive     = g_PR->IsAlive( playerIndex );

                        if ( m_TSlots[slot].iLastPlayerIndex != playerIndex )
                        {
                                m_TSlots[slot].iLastPlayerIndex = playerIndex;
                                m_TSlots[slot].pAvatar->SetPlayer( playerIndex, k_EAvatarSize32x32 );
                        }

                        m_TSlots[slot].bIsAlive = bAlive;
                        m_TSlots[slot].pAvatar->SetVisible( bAlive  );
                        m_TSlots[slot].pSkull->SetVisible( !bAlive  );
                }
                else
                {
                        m_TSlots[slot].iLastPlayerIndex = 0;
                        m_TSlots[slot].bIsAlive = false;
                        m_TSlots[slot].pAvatar->SetVisible( false );
                        m_TSlots[slot].pSkull->SetVisible( false );
                }
        }
}


//=============================================================================
// OnThink – called every frame the panel is visible
//=============================================================================
void CHudTeamCounterSingerline::OnThink()
{
        //--------------------------------------------------------------------------
        // Position toggle (top / bottom)
        //--------------------------------------------------------------------------
        if ( m_bIsAtTheBottom != hud_playercount_pos.GetBool() )
        {
                m_bIsAtTheBottom = hud_playercount_pos.GetBool();
                InvalidateLayout( true, false );
        }

        //--------------------------------------------------------------------------
        // Team win scores
        //--------------------------------------------------------------------------
        wchar_t unicode[8];

        C_CSTeam *teamCT = static_cast<C_CSTeam*>( GetGlobalTeam( TEAM_CT ) );
        C_CSTeam *teamT  = static_cast<C_CSTeam*>( GetGlobalTeam( TEAM_TERRORIST ) );

        if ( teamCT )
        {
                V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d", teamCT->Get_Score() );
                m_pCTWinCounterLabel->SetText( unicode );
        }
        if ( teamT )
        {
                V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d", teamT->Get_Score() );
                m_pTWinCounterLabel->SetText( unicode );
        }

        //--------------------------------------------------------------------------
        // Update avatar slots
        //--------------------------------------------------------------------------
        UpdateAvatarMode();

        //--------------------------------------------------------------------------
        // Round timer / C4 bomb icon (from original script)
        //--------------------------------------------------------------------------
        C_CSGameRules *pRules = CSGameRules();
        if ( !pRules )
                return;

        // Check if bomb is planted
        bool bBombPlanted = ( g_PlantedC4s.Count() > 0 );

        if ( bBombPlanted )
        {
                C_PlantedC4 *pC4 = g_PlantedC4s[0];

                // Check if defused
                if ( pC4->m_bBombDefused )
                {
                        // Bomb defused - show solid green icon
                        m_pBombIcon->SetAlpha( 255 );
                        m_pBombIcon->SetFgColor( m_clrC4Defused );
                        m_pBombIcon->SetVisible( true );
                }
                else
                {
                        // Bomb still ticking - pulsing effect based on m_flNextGlow
                        int alpha = 255;
                        if ( gpGlobals->curtime + 0.1f >= pC4->m_flNextGlow )
                                alpha = 128;  // Dim when not glowing

                        m_pBombIcon->SetAlpha( alpha );
                        m_pBombIcon->SetFgColor( m_clrC4Planted );

                        // Hide bomb icon when explode warning is active
                        m_pBombIcon->SetVisible( !pC4->m_bExplodeWarning );
                }
        }
        else
        {
                m_pBombIcon->SetVisible( false );
        }

        // Timer text - empty when bomb planted, time out active, or warmup
        if ( bBombPlanted || pRules->IsTimeOutActive() || pRules->IsWarmupPeriod() )
        {
                // Show empty space (like original script)
                m_pRoundTimerLabel->SetText( L" " );
        }
        else
        {
                // Normal timer logic
                if ( m_iRoundTime < (int)ceil( pRules->GetRoundRemainingTime() ) )
                        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );

                m_iRoundTime = (int)ceil( pRules->GetRoundRemainingTime() );

                if ( pRules->IsFreezePeriod() )
                {
                        // In freeze period, countdown to round start time
                        m_iRoundTime = (int)ceil( pRules->GetRoundStartTime() - gpGlobals->curtime );
                }

                if ( m_iRoundTime < 0 )
                        m_iRoundTime = 0;

                if ( m_iRoundTime <= 10 )
                        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerLow" );

                int iMinutes = m_iRoundTime / 60;
                int iSeconds = m_iRoundTime % 60;

                V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d : %.2d", iMinutes, iSeconds );
                m_pRoundTimerLabel->SetText( unicode );
        }
}
