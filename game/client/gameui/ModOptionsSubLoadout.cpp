//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubLoadout.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#include "GameUI_Interface.h"

#include "cs_shareddefs.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

extern ConVar loadout_music;

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::CModOptionsSubLoadout(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubLoadout")
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create scroll container panel (holds all controls for scrolling)
	m_pScrollContainer = new vgui::Panel(this, "ScrollContainer");

	// Create vertical scroll bar
	m_pVScrollBar = new vgui::ScrollBar(this, "VScrollBar", true);
	m_pVScrollBar->AddActionSignalTarget(this);

	// Create labels (inside scroll container)
	// CT Weapons labels
	m_pM4ComboBoxLabel = new vgui::Label(m_pScrollContainer, "M4ComboBoxLabel", "#GameUI_Loadout_M4");
	m_pHKP2000ComboBoxLabel = new vgui::Label(m_pScrollContainer, "HKP2000ComboBoxLabel", "#GameUI_Loadout_HKP2000");
	m_pFiveSevenComboBoxLabel = new vgui::Label(m_pScrollContainer, "FiveSevenComboBoxLabel", "#GameUI_Loadout_FiveSeven");
	m_pMP7CTComboBoxLabel = new vgui::Label(m_pScrollContainer, "MP7CTComboBoxLabel", "#GameUI_Loadout_MP7_CT");
	m_pDeagleCTComboBoxLabel = new vgui::Label(m_pScrollContainer, "DeagleCTComboBoxLabel", "#GameUI_Loadout_Deagle_CT");

	// T Weapons labels
	m_pTec9ComboBoxLabel = new vgui::Label(m_pScrollContainer, "Tec9ComboBoxLabel", "#GameUI_Loadout_Tec9");
	m_pMP7TComboBoxLabel = new vgui::Label(m_pScrollContainer, "MP7TComboBoxLabel", "#GameUI_Loadout_MP7_T");
	m_pDeagleTComboBoxLabel = new vgui::Label(m_pScrollContainer, "DeagleTComboBoxLabel", "#GameUI_Loadout_Deagle_T");

	// Other labels
	m_pMusicSelectionLabel = new vgui::Label(m_pScrollContainer, "MusicSelectionLabel", "#GameUI_Gameplay_Music");

	// Create all controls (inside scroll container)
	m_pLoadoutM4ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "M4ComboBox");
	m_pLoadoutM4ComboBox->AddItem("#Cstrike_WPNHUD_M4A4", "loadout_slot_m4_weapon 0");
	m_pLoadoutM4ComboBox->AddItem("#Cstrike_WPNHUD_M4A1", "loadout_slot_m4_weapon 1");
	m_pLoadoutM4ComboBox->AddActionSignalTarget(this);

	m_pLoadoutHKP2000ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "HKP2000ComboBox");
	m_pLoadoutHKP2000ComboBox->AddItem("#Cstrike_WPNHUD_HKP2000", "loadout_slot_hkp2000_weapon 0");
	m_pLoadoutHKP2000ComboBox->AddItem("#Cstrike_WPNHUD_USP45", "loadout_slot_hkp2000_weapon 1");
	m_pLoadoutHKP2000ComboBox->AddActionSignalTarget(this);

	m_pLoadoutFiveSevenComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "FiveSevenComboBox");
	m_pLoadoutFiveSevenComboBox->AddItem("#Cstrike_WPNHUD_FiveSeven", "loadout_slot_fiveseven_weapon 0");
	m_pLoadoutFiveSevenComboBox->AddItem("#Cstrike_WPNHUD_CZ75", "loadout_slot_fiveseven_weapon 1");
	m_pLoadoutFiveSevenComboBox->AddActionSignalTarget(this);

	m_pLoadoutMP7CTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "MP7CTComboBox");
	m_pLoadoutMP7CTComboBox->AddItem("#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_ct 0");
	m_pLoadoutMP7CTComboBox->AddItem("#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_ct 1");
	m_pLoadoutMP7CTComboBox->AddActionSignalTarget(this);

	m_pLoadoutDeagleCTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "DeagleCTComboBox");
	m_pLoadoutDeagleCTComboBox->AddItem("#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_ct 0");
	m_pLoadoutDeagleCTComboBox->AddItem("#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_ct 1");
	m_pLoadoutDeagleCTComboBox->AddActionSignalTarget(this);

	m_pLoadoutTec9ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "Tec9ComboBox");
	m_pLoadoutTec9ComboBox->AddItem("#Cstrike_WPNHUD_Tec9", "loadout_slot_tec9_weapon 0");
	m_pLoadoutTec9ComboBox->AddItem("#Cstrike_WPNHUD_CZ75", "loadout_slot_tec9_weapon 1");
	m_pLoadoutTec9ComboBox->AddActionSignalTarget(this);

	m_pLoadoutMP7TComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "MP7TComboBox");
	m_pLoadoutMP7TComboBox->AddItem("#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_t 0");
	m_pLoadoutMP7TComboBox->AddItem("#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_t 1");
	m_pLoadoutMP7TComboBox->AddActionSignalTarget(this);

	m_pLoadoutDeagleTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "DeagleTComboBox");
	m_pLoadoutDeagleTComboBox->AddItem("#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_t 0");
	m_pLoadoutDeagleTComboBox->AddItem("#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_t 1");
	m_pLoadoutDeagleTComboBox->AddActionSignalTarget(this);

	m_pStatTrak = new CCvarToggleCheckButton(m_pScrollContainer, "EnableStatTrak", "#GameUI_Loadout_StatTrak", "loadout_stattrak");
	m_pStatTrak->AddActionSignalTarget(this);

	m_pMusicSelection = new CLabeledCommandComboBox(m_pScrollContainer, "MusicSelectionComboBox");
	for (int i = 0; i < MAX_MUSIC; i++)
	{
		char command[128];
		char string[128];
		Q_snprintf(command, sizeof(command), "loadout_music %d", i);
		Q_snprintf(string, sizeof(string), "#GameUI_Gameplay_MusicKit_%s", g_szMusicKits[i]);
		m_pMusicSelection->AddItem(string, command);
	}
	m_pMusicSelection->AddActionSignalTarget(this);

#if !INSTANT_MUSIC_CHANGE
	m_bNeedToWarnAboutMusic = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Padding and spacing values - scaled for different resolutions
	int margin = PROPVAL(24);
	int spacing = PROPVAL(12);
	int labelWidth = PROPVAL(224);
	int controlHeight = PROPVAL(24);
	int sectionSpacing = PROPVAL(28);
	int labelControlGap = PROPVAL(10);
	int scrollBarWidth = PROPVAL(20);

	// Column positions from RES file
	int leftColumnX = PROPVAL(16);
	int rightColumnX = PROPVAL(256);
	int labelHeight = PROPVAL(24);

	// Calculate total width and center offset
	int totalWidth = rightColumnX + labelWidth; // 256 + 224 = 480
	int centerOffset = (pw - totalWidth) / 2;

	// Calculate content width (accounting for scroll bar)
	int contentWidth = pw - (margin * 2) - scrollBarWidth;
	int controlWidth = contentWidth - labelWidth - labelControlGap;

	// Position scroll bar on the right side
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, margin);
	m_pVScrollBar->SetSize(scrollBarWidth, ph - (margin * 2));

	// Position scroll container
	m_pScrollContainer->SetPos(margin, margin);
	m_pScrollContainer->SetSize(contentWidth, ph - (margin * 2));

	// Calculate total content height first
	// Based on RES file: last control is MusicSelectionComboBox at y=280 with height 24
	int totalContentHeight = PROPVAL(304); // 280 + 24

	// Add bottom margin
	totalContentHeight += margin;

	// Set scroll bar range
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(ph - (margin * 2));

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();

	// Now position all controls inside the scroll container (offset by scroll position)
	// Using exact positions from RES file for consistency
	// ================== SECTION 1: Weapons (Two Column Layout) ==================

	// Row 1: M4 (left, y=8), HKP2000 (right, y=8)
	m_pM4ComboBoxLabel->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(8));
	m_pM4ComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutM4ComboBox->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(32));
	m_pLoadoutM4ComboBox->SetSize(labelWidth, controlHeight);

	m_pHKP2000ComboBoxLabel->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(8));
	m_pHKP2000ComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutHKP2000ComboBox->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(32));
	m_pLoadoutHKP2000ComboBox->SetSize(labelWidth, controlHeight);

	// Row 2: FiveSeven (left, y=56), Tec9 (right, y=56)
	m_pFiveSevenComboBoxLabel->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(56));
	m_pFiveSevenComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutFiveSevenComboBox->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(80));
	m_pLoadoutFiveSevenComboBox->SetSize(labelWidth, controlHeight);

	m_pTec9ComboBoxLabel->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(56));
	m_pTec9ComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutTec9ComboBox->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(80));
	m_pLoadoutTec9ComboBox->SetSize(labelWidth, controlHeight);

	// Row 3: MP7 CT (left, y=104), MP7 T (right, y=104)
	m_pMP7CTComboBoxLabel->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(104));
	m_pMP7CTComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutMP7CTComboBox->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(128));
	m_pLoadoutMP7CTComboBox->SetSize(labelWidth, controlHeight);

	m_pMP7TComboBoxLabel->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(104));
	m_pMP7TComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutMP7TComboBox->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(128));
	m_pLoadoutMP7TComboBox->SetSize(labelWidth, controlHeight);

	// Row 4: Deagle CT (left, y=152), Deagle T (right, y=152)
	m_pDeagleCTComboBoxLabel->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(152));
	m_pDeagleCTComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutDeagleCTComboBox->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(176));
	m_pLoadoutDeagleCTComboBox->SetSize(labelWidth, controlHeight);

	m_pDeagleTComboBoxLabel->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(152));
	m_pDeagleTComboBoxLabel->SetSize(labelWidth, labelHeight);
	m_pLoadoutDeagleTComboBox->SetPos(rightColumnX + centerOffset, -scrollOffset + PROPVAL(176));
	m_pLoadoutDeagleTComboBox->SetSize(labelWidth, controlHeight);

	// ================== SECTION 2: Options ==================
	// StatTrak (y=234)
	m_pStatTrak->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(234));
	m_pStatTrak->SetSize(labelWidth * 2 + spacing, controlHeight);

	// Music Selection (label y=256, combo y=280)
	m_pMusicSelectionLabel->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(256));
	m_pMusicSelectionLabel->SetSize(labelWidth * 2 + spacing, labelHeight);
	m_pMusicSelection->SetPos(leftColumnX + centerOffset, -scrollOffset + PROPVAL(280));
	m_pMusicSelection->SetSize(labelWidth * 2 + spacing, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::~CModOptionsSubLoadout()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnScrollBarSliderMoved( KeyValues *data )
{
	int position = data->GetInt( "position", 0 );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle slider movement
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnSliderMoved( KeyValues *data )
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
void CModOptionsSubLoadout::OnResetData()
{
	ConVarRef loadout_slot_m4_weapon( "loadout_slot_m4_weapon" );
	if ( loadout_slot_m4_weapon.IsValid() )
		m_pLoadoutM4ComboBox->SetInitialItem( loadout_slot_m4_weapon.GetInt() );

	ConVarRef loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon" );
	if ( loadout_slot_hkp2000_weapon.IsValid() )
		m_pLoadoutHKP2000ComboBox->SetInitialItem( loadout_slot_hkp2000_weapon.GetInt() );

	ConVarRef loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon" );
	if ( loadout_slot_fiveseven_weapon.IsValid() )
		m_pLoadoutFiveSevenComboBox->SetInitialItem( loadout_slot_fiveseven_weapon.GetInt() );

	ConVarRef loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon" );
	if ( loadout_slot_tec9_weapon.IsValid() )
		m_pLoadoutTec9ComboBox->SetInitialItem( loadout_slot_tec9_weapon.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct" );
	if ( loadout_slot_mp7_weapon_ct.IsValid() )
		m_pLoadoutMP7CTComboBox->SetInitialItem( loadout_slot_mp7_weapon_ct.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t" );
	if ( loadout_slot_mp7_weapon_t.IsValid() )
		m_pLoadoutMP7TComboBox->SetInitialItem( loadout_slot_mp7_weapon_t.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct" );
	if ( loadout_slot_deagle_weapon_ct.IsValid() )
		m_pLoadoutDeagleCTComboBox->SetInitialItem( loadout_slot_deagle_weapon_ct.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t" );
	if ( loadout_slot_deagle_weapon_t.IsValid() )
		m_pLoadoutDeagleTComboBox->SetInitialItem( loadout_slot_deagle_weapon_t.GetInt() );

	m_pStatTrak->Reset();

	m_pMusicSelection->SetInitialItem( Clamp( loadout_music.GetInt(), 0, MAX_MUSIC - 1 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnApplyChanges()
{
	m_pLoadoutM4ComboBox->ApplyChanges();
	m_pLoadoutHKP2000ComboBox->ApplyChanges();
	m_pLoadoutFiveSevenComboBox->ApplyChanges();
	m_pLoadoutTec9ComboBox->ApplyChanges();
	m_pLoadoutMP7CTComboBox->ApplyChanges();
	m_pLoadoutMP7TComboBox->ApplyChanges();
	m_pLoadoutDeagleCTComboBox->ApplyChanges();
	m_pLoadoutDeagleTComboBox->ApplyChanges();
	m_pStatTrak->ApplyChanges();

#if INSTANT_MUSIC_CHANGE
	if ( loadout_music.GetInt() != m_pMusicSelection->GetActiveItem() )
#else
	if ( m_bNeedToWarnAboutMusic && loadout_music.GetInt() != m_pMusicSelection->GetActiveItem() )
#endif
	{
		// Bring up the confirmation dialog
#if INSTANT_MUSIC_CHANGE
		m_pMusicSelection->ApplyChanges();
		GameUI().ReleaseBackgroundMusic();
#else
		MessageBox *box = new MessageBox( "#GameUI_OptionsRestartRequired_Title", "#GameUI_Gameplay_MusicRestartHint", this );
		box->MoveToFront();
		box->DoModal();
		m_bNeedToWarnAboutMusic = false;
#endif
	}
}
