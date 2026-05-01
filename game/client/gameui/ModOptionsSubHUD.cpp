//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubHUD.h"
#include <stdio.h>

#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ScrollBar.h>

#include "LabeledCommandComboBox.h"
#include "cvarslider.h"
#include "CvarToggleCheckButton.h"
#include "EngineInterface.h"
#include "tier1/convar.h"
#include "BasePanel.h"
#include "GameUI_Interface.h"

#include "hud.h" // for MAX_HUD_COLORS

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModOptionsSubHUD::CModOptionsSubHUD( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubHUD" )
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create scroll container panel (holds all controls for scrolling)
	m_pScrollContainer = new vgui::Panel(this, "ScrollContainer");

	// Create vertical scroll bar
	m_pVScrollBar = new vgui::ScrollBar(this, "VScrollBar", true);
	m_pVScrollBar->AddActionSignalTarget(this);

	// Create labels
	m_pPlayerCountPosLabel = new vgui::Label( m_pScrollContainer, "PlayerCountPositionLabel", "#GameUI_HUD_PlayerCount_Position" );
	m_pHealthAmmoStyleLabel = new vgui::Label( m_pScrollContainer, "HealthAmmoStyleLabel", "#GameUI_HUD_HealthAmmoStyle" );
	m_pHUDColorLabel = new vgui::Label( m_pScrollContainer, "HUDColorLabel", "#GameUI_HUD_Color" );
	m_pHUDBackgroundAlphaLabel = new vgui::Label( m_pScrollContainer, "HUDBackgroundAlphaLabel", "#GameUI_HUD_BackgroundAlpha" );
	m_pRadarScaleLabel = new vgui::Label( m_pScrollContainer, "RadarScaleLabel", "#GameUI_HUD_RadarScale" );
	m_pRadarSquareLabel = new vgui::Label( m_pScrollContainer, "RadarSquareLabel", "#GameUI_HUD_RadarSquare" );
	m_pMenuBackgroundLabel = new vgui::Label( m_pScrollContainer, "MenuBackgroundLabel", "#GameUI_HUD_MenuBackground" );
	m_pMenuAgentLabel = new vgui::Label( m_pScrollContainer, "MenuAgentLabel", "#GameUI_HUD_MenuAgent" );

	// Create controls
	m_pPlayerCountPos = new CLabeledCommandComboBox( m_pScrollContainer, "PlayerCountPositionComboBox" );
	m_pHealthAmmoStyle = new CLabeledCommandComboBox( m_pScrollContainer, "HealthAmmoStyleComboBox" );
	m_pHUDColor = new CLabeledCommandComboBox( m_pScrollContainer, "HUDColorComboBox" );
	m_pHUDBackgroundAlpha = new CCvarSlider( m_pScrollContainer, "HUDBackgroundAlphaSlider", "", 0.0f, 1.0f, "cl_hud_background_alpha" );
	m_pRadarScale = new CCvarSlider( m_pScrollContainer, "RadarScaleSlider", "", 0.25f, 1.0f, "cl_radar_scale" );
	m_pAlwaysShowInventory = new CCvarToggleCheckButton( m_pScrollContainer, "AlwaysShowInventoryCheckbox", "#GameUI_HUD_AlwaysShowInventory", "cl_showloadout" );
	m_pRadarRotate = new CCvarToggleCheckButton( m_pScrollContainer, "RadarRotateCheckbox", "#GameUI_HUD_RotateRadar", "cl_radar_rotate" );
	m_pRadarSquare = new CLabeledCommandComboBox( m_pScrollContainer, "RadarSquareComboBox" );
	m_pMenuBackground = new CLabeledCommandComboBox( m_pScrollContainer, "MenuBackgroundComboBox" );
	m_pMenuAgent = new CLabeledCommandComboBox( m_pScrollContainer, "MenuAgentComboBox" );

	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Top", "hud_playercount_pos 0" );
	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Bottom", "hud_playercount_pos 1" );

	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_0", "cl_hud_healthammo_style 0" );
	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_1", "cl_hud_healthammo_style 1" );

	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_0", "cl_radar_square 0" );
	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_1", "cl_radar_square 1" );
	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_2", "cl_radar_square 2" );

	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_None", "loadout_mainmenu_agent 0" );
	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_T", "loadout_mainmenu_agent 1" );
	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_CT", "loadout_mainmenu_agent 2" );

	char localization[64];
	char command[64];
	for ( int i = 0; i < ARRAYSIZE(s_MenuBackgrounds); i++ )
	{
		Q_snprintf( command, sizeof( command ), "cl_menu_background %d", i );
		m_pMenuBackground->AddItem( s_MenuBackgrounds[i].szName, command );
	}
	for ( int i = 0; i < MAX_HUD_COLORS; i++ )
	{
		Q_snprintf( localization, sizeof( localization ), "#GameUI_HUD_Color_%d", i );
		Q_snprintf( command, sizeof( command ), "cl_hud_color %d", i );
		m_pHUDColor->AddItem( localization, command );
	}

	m_pPlayerCountPos->AddActionSignalTarget( this );
	m_pHealthAmmoStyle->AddActionSignalTarget( this );
	m_pHUDColor->AddActionSignalTarget( this );
	m_pHUDBackgroundAlpha->AddActionSignalTarget( this );
	m_pRadarScale->AddActionSignalTarget( this );
	m_pAlwaysShowInventory->AddActionSignalTarget( this );
	m_pRadarRotate->AddActionSignalTarget( this );
	m_pRadarSquare->AddActionSignalTarget( this );
	m_pMenuBackground->AddActionSignalTarget( this );
	m_pMenuAgent->AddActionSignalTarget( this );
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

#ifndef PROPVAL
	#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

	int margin = PROPVAL(16);
	int spacing = PROPVAL(12);
	int controlHeight = PROPVAL(24);
	int sliderHeight = PROPVAL(32);
	int columnWidth = PROPVAL(224);
	int scrollBarWidth = PROPVAL(20);

	// Calculate total width and center offset
	int totalWidth = columnWidth * 2 + spacing; // 224 * 2 + 12 = 460
	int centerOffset = (pw - totalWidth) / 2;

	// Left column X position
	int leftColumnX = margin + centerOffset;
	// Right column X position
	int rightColumnX = leftColumnX + columnWidth + spacing;

	int contentWidth = pw - (margin * 2) - scrollBarWidth;
	int controlWidth = columnWidth;

	// Position scroll bar on the right side
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, margin);
	m_pVScrollBar->SetSize(scrollBarWidth, ph - (margin * 2));

	// Position scroll container
	m_pScrollContainer->SetPos(margin, margin);
	m_pScrollContainer->SetSize(contentWidth, ph - (margin * 2));

	// Calculate total content height based on RES file positions
	int totalContentHeight = 0;
	// Find the maximum Y position from all controls + bottom margin
	// Menu Background/Agent at y=280 + controlHeight(24) + margin(16) = ~320
	totalContentHeight = PROPVAL(320);

	// Set scroll bar range
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(ph - (margin * 2));

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();

	// Now position all controls inside the scroll container (offset by scroll position)
	// Using Y positions from RES file
	int leftY = -scrollOffset;
	int rightY = -scrollOffset;
	int labelHeight = PROPVAL(24);

	// ================== LEFT COLUMN ==================
	// Player Count Position Label (y=8 in RES)
	leftY += PROPVAL(8);
	m_pPlayerCountPosLabel->SetPos(leftColumnX, leftY);
	m_pPlayerCountPosLabel->SetSize(controlWidth, labelHeight);

	// Player Count Position ComboBox (y=32 in RES)
	leftY += PROPVAL(24);
	m_pPlayerCountPos->SetPos(leftColumnX, leftY);
	m_pPlayerCountPos->SetSize(controlWidth, controlHeight);

	// Health Ammo Style Label (y=56 in RES)
	leftY = -scrollOffset + PROPVAL(56);
	m_pHealthAmmoStyleLabel->SetPos(leftColumnX, leftY);
	m_pHealthAmmoStyleLabel->SetSize(controlWidth, labelHeight);

	// Health Ammo Style ComboBox (y=80 in RES)
	leftY += PROPVAL(24);
	m_pHealthAmmoStyle->SetPos(leftColumnX, leftY);
	m_pHealthAmmoStyle->SetSize(controlWidth, controlHeight);

	// HUD Color Label (y=104 in RES)
	leftY = -scrollOffset + PROPVAL(104);
	m_pHUDColorLabel->SetPos(leftColumnX, leftY);
	m_pHUDColorLabel->SetSize(controlWidth, labelHeight);

	// HUD Color ComboBox (y=128 in RES)
	leftY += PROPVAL(24);
	m_pHUDColor->SetPos(leftColumnX, leftY);
	m_pHUDColor->SetSize(controlWidth, controlHeight);

	// Always Show Inventory (y=204 in RES)
	leftY = -scrollOffset + PROPVAL(204);
	m_pAlwaysShowInventory->SetPos(leftColumnX, leftY);
	m_pAlwaysShowInventory->SetSize(controlWidth, controlHeight);

	// Radar Rotate (y=228 in RES)
	leftY += controlHeight + spacing;
	m_pRadarRotate->SetPos(leftColumnX, leftY);
	m_pRadarRotate->SetSize(controlWidth, controlHeight);

	// Menu Background Label (y=256 in RES)
	leftY = -scrollOffset + PROPVAL(256);
	m_pMenuBackgroundLabel->SetPos(leftColumnX, leftY);
	m_pMenuBackgroundLabel->SetSize(controlWidth, labelHeight);

	// Menu Background ComboBox (y=280 in RES)
	leftY += PROPVAL(24);
	m_pMenuBackground->SetPos(leftColumnX, leftY);
	m_pMenuBackground->SetSize(controlWidth, controlHeight);

	// ================== RIGHT COLUMN ==================
	// Radar Square Label (y=8 in RES)
	rightY += PROPVAL(8);
	m_pRadarSquareLabel->SetPos(rightColumnX, rightY);
	m_pRadarSquareLabel->SetSize(controlWidth, labelHeight);

	// Radar Square ComboBox (y=32 in RES)
	rightY += PROPVAL(24);
	m_pRadarSquare->SetPos(rightColumnX, rightY);
	m_pRadarSquare->SetSize(controlWidth, controlHeight);

	// HUD Background Alpha Label (y=56 in RES)
	rightY = -scrollOffset + PROPVAL(56);
	m_pHUDBackgroundAlphaLabel->SetPos(rightColumnX, rightY);
	m_pHUDBackgroundAlphaLabel->SetSize(controlWidth, labelHeight);

	// HUD Background Alpha Slider (y=76 in RES) - slider is taller
	rightY += PROPVAL(20);
	m_pHUDBackgroundAlpha->SetPos(rightColumnX, rightY);
	m_pHUDBackgroundAlpha->SetSize(controlWidth, sliderHeight);

	// Radar Scale Label (y=104 in RES)
	rightY = -scrollOffset + PROPVAL(104);
	m_pRadarScaleLabel->SetPos(rightColumnX, rightY);
	m_pRadarScaleLabel->SetSize(controlWidth, labelHeight);

	// Radar Scale Slider (y=124 in RES) - slider is taller
	rightY += PROPVAL(20);
	m_pRadarScale->SetPos(rightColumnX, rightY);
	m_pRadarScale->SetSize(controlWidth, sliderHeight);

	// Menu Agent Label (y=256 in RES)
	rightY = -scrollOffset + PROPVAL(256);
	m_pMenuAgentLabel->SetPos(rightColumnX, rightY);
	m_pMenuAgentLabel->SetSize(controlWidth, labelHeight);

	// Menu Agent ComboBox (y=280 in RES)
	rightY += PROPVAL(24);
	m_pMenuAgent->SetPos(rightColumnX, rightY);
	m_pMenuAgent->SetSize(controlWidth, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubHUD::~CModOptionsSubHUD()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnScrollBarSliderMoved( KeyValues *data )
{
	int position = data->GetInt( "position", 0 );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle slider movement
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnSliderMoved( KeyValues *data )
{
	vgui::Panel* pPanel = static_cast<vgui::Panel*>(data->GetPtr( "panel" ));

	if ( pPanel == m_pVScrollBar )
	{
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnResetData()
{
	ConVarRef hud_playercount_pos( "hud_playercount_pos" );
	if ( hud_playercount_pos.IsValid() )
		m_pPlayerCountPos->SetInitialItem( hud_playercount_pos.GetInt() );

	ConVarRef cl_hud_healthammo_style( "cl_hud_healthammo_style" );
	if ( cl_hud_healthammo_style.IsValid() )
		m_pHealthAmmoStyle->SetInitialItem( cl_hud_healthammo_style.GetInt() );

	ConVarRef cl_hud_color( "cl_hud_color" );
	if ( cl_hud_color.IsValid() )
		m_pHUDColor->SetInitialItem( cl_hud_color.GetInt() );

	ConVarRef cl_radar_square( "cl_radar_square" );
	if ( cl_radar_square.IsValid() )
		m_pRadarSquare->SetInitialItem( cl_radar_square.GetInt() );

	ConVarRef cl_menu_background( "cl_menu_background" );
	if ( cl_menu_background.IsValid() )
		m_pMenuBackground->SetInitialItem( cl_menu_background.GetInt() );

	ConVarRef loadout_mainmenu_agent( "loadout_mainmenu_agent" );
	if ( loadout_mainmenu_agent.IsValid() )
		m_pMenuAgent->SetInitialItem( loadout_mainmenu_agent.GetInt() );

	m_pHUDBackgroundAlpha->Reset();
	m_pRadarScale->Reset();
	m_pAlwaysShowInventory->Reset();
	m_pRadarRotate->Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnApplyChanges()
{
	m_pPlayerCountPos->ApplyChanges();
	m_pHealthAmmoStyle->ApplyChanges();
	m_pHUDColor->ApplyChanges();
	m_pHUDBackgroundAlpha->ApplyChanges();
	m_pRadarScale->ApplyChanges();
	m_pAlwaysShowInventory->ApplyChanges();
	m_pRadarRotate->ApplyChanges();
	m_pRadarSquare->ApplyChanges();

	ConVarRef cl_menu_background( "cl_menu_background" );
	if ( cl_menu_background.GetInt() != m_pMenuBackground->GetActiveItem() )
	{
		m_pMenuBackground->ApplyChanges();
		BasePanel()->RestartBackgroundVideo();
	}

	m_pMenuAgent->ApplyChanges();

	// update agent on main menu
	GameUI().UpdateAgentModel();
}