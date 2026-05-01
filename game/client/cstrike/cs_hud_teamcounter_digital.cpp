//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a small piece of HUD that shows alive counter and win counter for each team
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"
#include "cs_gamerules.h"
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include "c_plantedc4.h"

using namespace vgui;

extern ConVar cl_draw_only_deathnotices;
extern ConVar hud_playercount_pos;
extern ConVar hud_teamcounter_style;

class CHudTeamCounterDigital: public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudTeamCounterDigital, EditablePanel );

public:
	CHudTeamCounterDigital( const char *pElementName );
	virtual void Init( void );
	virtual void PerformLayout();
	virtual void Reset( void );
	virtual bool ShouldDraw();
	virtual void OnThink();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );

private:
	int ScalePx( float basePixels ) const;
	void Layout();

	Label				*m_pCTWinCounterLabel;
	Label				*m_pCTAliveCounterLabel;
	Label				*m_pCTAliveTextLabel;
	Label				*m_pTWinCounterLabel;
	Label				*m_pTAliveCounterLabel;
	Label				*m_pTAliveTextLabel;
	Label				*m_pRoundTimerLabel;
	VectorImagePanel	*m_pBombIcon;
	ImagePanel			*m_pCTSkullImage;
	ImagePanel			*m_pTSkullImage;

	int m_iRoundTime;
	bool m_bIsAtTheBottom;

	CPanelAnimationVar( Color, m_clrC4Planted, "C4PlantedColor", "Red" );
	CPanelAnimationVar( Color, m_clrC4Defused, "C4DefusedColor", "SteamLightGreen" );
};

DECLARE_HUDELEMENT( CHudTeamCounterDigital );

CHudTeamCounterDigital::CHudTeamCounterDigital( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, "HudTeamCounter" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_pCTWinCounterLabel = new Label( this, "DigitalCTWinCounterLabel", "0" );
	m_pCTAliveCounterLabel = new Label( this, "DigitalCTAliveCounterLabel", "0" );
	m_pCTAliveTextLabel = new Label( this, "DigitalCTAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pTWinCounterLabel = new Label( this, "DigitalTWinCounterLabel", "0" );
	m_pTAliveCounterLabel = new Label( this, "DigitalTAliveCounterLabel", "0" );
	m_pTAliveTextLabel = new Label( this, "DigitalTAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pRoundTimerLabel = new Label( this, "DigitalRoundTimerLabel", "0:00" );
	m_pBombIcon = new VectorImagePanel( this, "DigitalBombIcon" );
	m_pCTSkullImage = new ImagePanel( this, "DigitalCTSkullImage" );
	m_pTSkullImage = new ImagePanel( this, "DigitalTSkullImage" );

	LoadControlSettings( "resource/hud/teamcounterdigital.res" );
}

void CHudTeamCounterDigital::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "resource/hud/teamcounterdigital.res" );

	// force recalculation of some stuff
	m_bIsAtTheBottom = false;
}

void CHudTeamCounterDigital::Init( void )
{
	m_iRoundTime = 0;
	m_bIsAtTheBottom = false;
}

int CHudTeamCounterDigital::ScalePx( float basePixels ) const
{
	float flScale = (float)ScreenHeight() / 1080.0f;
	int result = (int)( basePixels * flScale );
	return MAX( 1, result );
}

void CHudTeamCounterDigital::Layout()
{
	// Panel size - computed from .res file values at 1080p, then scaled
	// CT block (51px) + gap + timer (84px) + gap + T block (51px) = ~192px at 1080p
	int panelW = ScalePx( 192 );
	int panelH = ScalePx( 55 );  // matches DigitalCTAliveBgImage tall

	// Panel position: centered horizontally, at top (or bottom) of screen
	int panelX = ( ScreenWidth() - panelW ) / 2;
	int panelY;
	if ( m_bIsAtTheBottom )
		panelY = ScreenHeight() - panelH - ScalePx( 4 );
	else
		panelY = ScalePx( 2 );  // 2px top margin

	SetPos( panelX, panelY );
	SetSize( panelW, panelH );
}

void CHudTeamCounterDigital::PerformLayout()
{
	BaseClass::PerformLayout();
	Layout();
}

void CHudTeamCounterDigital::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );
}

bool CHudTeamCounterDigital::ShouldDraw()
{
	if ( hud_teamcounter_style.GetInt() != 2 )
		return false;

	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsObserver() )
		return false;
			
	return CHudElement::ShouldDraw();
}

void CHudTeamCounterDigital::OnThink()
{
	// Position toggle (top / bottom) - trigger relayout when changed
	if ( m_bIsAtTheBottom != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheBottom = hud_playercount_pos.GetBool();		
	}

	C_CSTeam *teamCT = GetGlobalCSTeam( TEAM_CT );
	C_CSTeam *teamT = GetGlobalCSTeam( TEAM_TERRORIST );

	wchar_t unicode[16];
	if ( teamCT )
	{
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", teamCT->Get_Score() );
		m_pCTWinCounterLabel->SetText( unicode );
	}
	if ( teamT )
	{
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", teamT->Get_Score() );
		m_pCTWinCounterLabel->SetText( unicode );
	}

	if ( g_PR )
	{
		// Count the players on the team.
		int iCTCounter = 0;
		int iTCounter = 0;
		for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
		{
			if ( g_PR->IsConnected( playerIndex ) && g_PR->IsAlive( playerIndex ) )
			{
				if ( g_PR->GetTeam( playerIndex ) == TEAM_CT )
					iCTCounter++;

				if ( g_PR->GetTeam( playerIndex ) == TEAM_TERRORIST )
					iTCounter++;
			}
		}

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", iCTCounter );
		m_pCTAliveCounterLabel->SetText( unicode );

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", iTCounter );
		m_pTAliveCounterLabel->SetText( unicode );

		C_CSTeam *team = GetGlobalCSTeam( TEAM_CT );
		if ( team )
		{
			V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", team->Get_Score() );
			m_pCTWinCounterLabel->SetText( unicode );
		}
		team = GetGlobalCSTeam( TEAM_TERRORIST );
		if ( team )
		{
			V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", team->Get_Score() );
			m_pTWinCounterLabel->SetText( unicode );
		}

		m_pCTAliveCounterLabel->SetVisible( iCTCounter > 0 );
		m_pCTAliveTextLabel->SetVisible( iCTCounter > 0 );
		m_pTAliveCounterLabel->SetVisible( iTCounter > 0 );
		m_pTAliveTextLabel->SetVisible( iTCounter > 0 );
		m_pCTSkullImage->SetVisible( iCTCounter < 1 );
		m_pTSkullImage->SetVisible( iTCounter < 1 );
	}

	C_CSGameRules *pRules = CSGameRules();
	if ( !pRules )
		return;

	bool bBombPlanted = (g_PlantedC4s.Count() > 0);
	if ( bBombPlanted )
	{
		C_PlantedC4 *pC4 = g_PlantedC4s[0];

		if ( pC4->m_bBombDefused )
		{
			m_pBombIcon->SetAlpha( 255 );
			m_pBombIcon->SetFgColor( m_clrC4Defused );
			m_pBombIcon->SetVisible( true );
		}
		else
		{
			int alpha = 255;
			if ( gpGlobals->curtime + 0.1f >= pC4->m_flNextGlow )
				alpha = 128;

			m_pBombIcon->SetAlpha( alpha );
			m_pBombIcon->SetFgColor( m_clrC4Planted );
			m_pBombIcon->SetVisible( !pC4->m_bExplodeWarning );
		}
	}
	else
		m_pBombIcon->SetVisible( false );

	if ( bBombPlanted || pRules->IsTimeOutActive() || pRules->IsWarmupPeriod() )
		m_pRoundTimerLabel->SetText( L" " );
	else
	{
		if ( m_iRoundTime < (int) ceil( pRules->GetRoundRemainingTime() ) )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );

		m_iRoundTime = (int) ceil( pRules->GetRoundRemainingTime() );

		if ( pRules->IsFreezePeriod() )
		{
			// in freeze period countdown to round start time
			m_iRoundTime = (int) ceil( pRules->GetRoundStartTime() - gpGlobals->curtime );
		}

		if ( m_iRoundTime < 0 )
			m_iRoundTime = 0;

		if ( m_iRoundTime <= 10 )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerLow" );

		int iMinutes = m_iRoundTime / 60;
		int iSeconds = m_iRoundTime % 60;

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d : %.2d", iMinutes, iSeconds );
		m_pRoundTimerLabel->SetText( unicode );
	}
}