//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubGameplay.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>
#include "vgui_controls/QueryBox.h"

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubGameplay::CModOptionsSubGameplay( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubGameplay" )
{
	// Create controls - positions will be set in PerformLayout
#ifndef PROPVAL
	#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create scroll container panel (holds all controls for scrolling)
	m_pScrollContainer = new vgui::Panel(this, "ScrollContainer");

	// Create vertical scroll bar
	m_pVScrollBar = new vgui::ScrollBar(this, "VScrollBar", true);
	m_pVScrollBar->AddActionSignalTarget(this);

	// Create Section Labels (inside scroll container)
	// Only create labels that are actually used in PerformLayout
	m_pViewmodelLabel = new Label(m_pScrollContainer, "ViewmodelLabel", "#GameUI_Gameplay_Viewmodel");
	m_pPresetLabel = new Label(m_pScrollContainer, "PresetLabel", "#GameUI_Gameplay_ViewmodelPreset");
	m_pFOVDescription = new Label(m_pScrollContainer, "ViewmodelFOVDescription", "#GameUI_Gameplay_ViewmodelFOV");
	m_pRecoilDescription = new Label(m_pScrollContainer, "ViewmodelRecoilDescription", "#GameUI_Gameplay_ViewmodelRecoil");
	m_pViewbobLabel = new Label(m_pScrollContainer, "ViewbobLabel", "#GameUI_Gameplay_ViewbobStyle");
	m_pWeaponPosLabel = new Label(m_pScrollContainer, "WeaponPosLabel", "#GameUI_Gameplay_WeaponPos");
	m_pDivider1 = new vgui::Panel(m_pScrollContainer, "Divider1");

	// Create all controls (inside scroll container)
	m_pCloseOnBuy = new CCvarToggleCheckButton(m_pScrollContainer, "CloseOnBuyCheckbox", "#GameUI_Gameplay_CloseOnBuy", "closeonbuy");
	m_pUseOpensBuyMenu = new CCvarToggleCheckButton(m_pScrollContainer, "UseOpensBuyMenuCheckbox", "#GameUI_Gameplay_UseOpensBuyMenu", "cl_use_opens_buy_menu");
	m_pAddBotPrefix = new CCvarToggleCheckButton(m_pScrollContainer, "AddBotPrefix", "#GameUI_Gameplay_AddBotPrefix", "cl_add_bot_prefix");
	m_pDrawTracers = new CCvarToggleCheckButton(m_pScrollContainer, "DrawTracers", "#GameUI_Gameplay_DrawTracers", "r_drawtracers");
	m_pSpecInterpCamera = new CCvarToggleCheckButton(m_pScrollContainer, "SpecInterpCamera", "#GameUI_Gameplay_SpecInterpCamera", "cl_obs_interp_enable");
	m_pDisableShootingEffects = new CCvarToggleCheckButton(m_pScrollContainer, "DisableShootingEffects", "#GameUI_Gameplay_DisableShootingEffects", "cl_disable_shooting_effects");

	m_pViewmodelOffsetX = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetXSlider", "", -2.0f, 2.5f, "viewmodel_offset_x");
	m_pViewmodelOffsetXLabel = new Label(m_pScrollContainer, "ViewmodelOffsetXLabel", "0.0");
	m_pViewmodelOffsetY = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetYSlider", "", -2.0f, 2.0f, "viewmodel_offset_y");
	m_pViewmodelOffsetYLabel = new Label(m_pScrollContainer, "ViewmodelOffsetYLabel", "0.0");
	m_pViewmodelOffsetZ = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetZSlider", "", -2.0f, 2.0f, "viewmodel_offset_z");
	m_pViewmodelOffsetZLabel = new Label(m_pScrollContainer, "ViewmodelOffsetZLabel", "0.0");
	m_pViewmodelOffsetPreset = new CLabeledCommandComboBox(m_pScrollContainer, "ViewmodelOffsetPreset");
	m_pViewmodelFOV = new CCvarSlider(m_pScrollContainer, "ViewmodelFOVSlider", "", 54.0f, 68.0f, "viewmodel_fov");
	m_pViewmodelFOVLabel = new Label(m_pScrollContainer, "ViewmodelFOVLabel", "60");
	m_pViewmodelRecoil = new CCvarSlider(m_pScrollContainer, "ViewmodelRecoilSlider", "", 0.0f, 1.0f, "viewmodel_recoil");
	m_pViewmodelRecoilLabel = new Label(m_pScrollContainer, "ViewmodelRecoilLabel", "0.0");
	m_pViewbobStyle = new CLabeledCommandComboBox(m_pScrollContainer, "ViewbobStyleComboBox");
	m_pWeaponPos = new CLabeledCommandComboBox(m_pScrollContainer, "WeaponPositionComboBox");

	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_1", "viewmodel_presetpos 1");
	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_2", "viewmodel_presetpos 2");
	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_3", "viewmodel_presetpos 3");

	m_pViewbobStyle->AddItem("#GameUI_Gameplay_Viewbob_CSS", "cl_use_new_headbob 0");
	m_pViewbobStyle->AddItem("#GameUI_Gameplay_Viewbob_CSGO", "cl_use_new_headbob 1");

	m_pWeaponPos->AddItem("#GameUI_Gameplay_Hand_Left", "cl_righthand 0");
	m_pWeaponPos->AddItem("#GameUI_Gameplay_Hand_Right", "cl_righthand 1");

	m_pCloseOnBuy->AddActionSignalTarget(this);
	m_pUseOpensBuyMenu->AddActionSignalTarget(this);
	m_pAddBotPrefix->AddActionSignalTarget(this);
	m_pDrawTracers->AddActionSignalTarget(this);
	m_pSpecInterpCamera->AddActionSignalTarget(this);
	m_pDisableShootingEffects->AddActionSignalTarget(this);
	m_pViewmodelOffsetX->AddActionSignalTarget(this);
	m_pViewmodelOffsetY->AddActionSignalTarget(this);
	m_pViewmodelOffsetZ->AddActionSignalTarget(this);
	m_pViewmodelFOV->AddActionSignalTarget(this);
	m_pViewmodelRecoil->AddActionSignalTarget(this);
	m_pViewmodelOffsetPreset->AddActionSignalTarget(this);
	m_pViewbobStyle->AddActionSignalTarget(this);
	m_pWeaponPos->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size from parent
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Base values from RES file
	int margin = PROPVAL(16);
	int labelHeight = PROPVAL(24);
	int sliderHeight = PROPVAL(40);
	int controlHeight = PROPVAL(24);
	int scrollBarWidth = PROPVAL(20);

	// RES file coordinate positions
	int sliderWidth = PROPVAL(128);
	int comboWidth = PROPVAL(128);
	int sliderLabelWidth = PROPVAL(64);
	int labelWidth = PROPVAL(192);
	int dividerWidth = PROPVAL(464);
	int fullWidth = PROPVAL(496);

	// Column X positions from RES
	int leftColX = PROPVAL(24);
	int midColX = PROPVAL(184);
	int rightColX = PROPVAL(352);

	// Calculate total width and center offset
	int totalWidth = rightColX + sliderWidth; // 352 + 128 = 480
	int centerOffset = (pw - totalWidth) / 2;

	// Calculate content width
	int contentWidth = pw - (margin * 2) - scrollBarWidth;
	int controlWidth = contentWidth - margin;

	// Position scroll bar on the right side
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, margin);
	m_pVScrollBar->SetSize(scrollBarWidth, ph - (margin * 2));

	// Position scroll container
	m_pScrollContainer->SetPos(margin, margin);
	m_pScrollContainer->SetSize(contentWidth, ph - (margin * 2));

	// Total content height
	int totalContentHeight = PROPVAL(314);
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(ph - (margin * 2));

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();
	int startY = -scrollOffset;

	// ================== Viewmodel Section ==================
	// ViewmodelLabel: x=16, y=0, wide=192
	m_pViewmodelLabel->SetPos(centerOffset, startY);
	m_pViewmodelLabel->SetSize(labelWidth, labelHeight);
	m_pViewmodelLabel->SetContentAlignment(Label::a_west);

	// --- Left Column (x=24): Offset X/Y/Z sliders ---
	// OffsetX: x=24, y=24, wide=128
	m_pViewmodelOffsetX->SetPos(leftColX + centerOffset, startY + PROPVAL(24));
	m_pViewmodelOffsetX->SetSize(sliderWidth, sliderHeight);
	// Value label: x=24+128=152, y=24, wide=64
	m_pViewmodelOffsetXLabel->SetPos(leftColX + sliderWidth + centerOffset, startY + PROPVAL(24));
	m_pViewmodelOffsetXLabel->SetSize(sliderLabelWidth, labelHeight);

	// OffsetY: x=24, y=60
	m_pViewmodelOffsetY->SetPos(leftColX + centerOffset, startY + PROPVAL(60));
	m_pViewmodelOffsetY->SetSize(sliderWidth, sliderHeight);
	m_pViewmodelOffsetYLabel->SetPos(leftColX + sliderWidth + centerOffset, startY + PROPVAL(60));
	m_pViewmodelOffsetYLabel->SetSize(sliderLabelWidth, labelHeight);

	// OffsetZ: x=24, y=96
	m_pViewmodelOffsetZ->SetPos(leftColX + centerOffset, startY + PROPVAL(96));
	m_pViewmodelOffsetZ->SetSize(sliderWidth, sliderHeight);
	m_pViewmodelOffsetZLabel->SetPos(leftColX + sliderWidth + centerOffset, startY + PROPVAL(96));
	m_pViewmodelOffsetZLabel->SetSize(sliderLabelWidth, labelHeight);

	// --- Middle Column (x=184): FOV and Recoil ---
	// FOV Description: x=184, y=0, wide=192 (label, not slider)
	m_pFOVDescription->SetPos(midColX + centerOffset, startY);
	m_pFOVDescription->SetSize(labelWidth, labelHeight);
	m_pFOVDescription->SetContentAlignment(Label::a_west);

	// FOV Slider: x=184, y=24, wide=128
	m_pViewmodelFOV->SetPos(midColX + centerOffset, startY + PROPVAL(24));
	m_pViewmodelFOV->SetSize(sliderWidth, sliderHeight);
	// Value label: x=184+128=312, y=24, wide=64
	m_pViewmodelFOVLabel->SetPos(midColX + sliderWidth + centerOffset, startY + PROPVAL(24));
	m_pViewmodelFOVLabel->SetSize(sliderLabelWidth, labelHeight);

	// Recoil Description: x=184, y=72
	m_pRecoilDescription->SetPos(midColX + centerOffset, startY + PROPVAL(72));
	m_pRecoilDescription->SetSize(labelWidth, labelHeight);
	m_pRecoilDescription->SetContentAlignment(Label::a_west);

	// Recoil Slider: x=184, y=96
	m_pViewmodelRecoil->SetPos(midColX + centerOffset, startY + PROPVAL(96));
	m_pViewmodelRecoil->SetSize(sliderWidth, sliderHeight);
	m_pViewmodelRecoilLabel->SetPos(midColX + sliderWidth + centerOffset, startY + PROPVAL(96));
	m_pViewmodelRecoilLabel->SetSize(sliderLabelWidth, labelHeight);

	// --- Right Column (x=352): Preset, Viewbob, WeaponPos ---
	// Preset Label: x=352, y=0
	m_pPresetLabel->SetPos(rightColX + centerOffset, startY);
	m_pPresetLabel->SetSize(labelWidth, labelHeight);
	m_pPresetLabel->SetContentAlignment(Label::a_west);

	// Preset ComboBox: x=352, y=24
	m_pViewmodelOffsetPreset->SetPos(rightColX + centerOffset, startY + PROPVAL(24));
	m_pViewmodelOffsetPreset->SetSize(comboWidth, controlHeight);

	// Viewbob Label: x=352, y=48
	m_pViewbobLabel->SetPos(rightColX + centerOffset, startY + PROPVAL(48));
	m_pViewbobLabel->SetSize(labelWidth, labelHeight);
	m_pViewbobLabel->SetContentAlignment(Label::a_west);

	// Viewbob ComboBox: x=352, y=72
	m_pViewbobStyle->SetPos(rightColX + centerOffset, startY + PROPVAL(72));
	m_pViewbobStyle->SetSize(comboWidth, controlHeight);

	// WeaponPos Label: x=352, y=96
	m_pWeaponPosLabel->SetPos(rightColX + centerOffset, startY + PROPVAL(96));
	m_pWeaponPosLabel->SetSize(labelWidth, labelHeight);
	m_pWeaponPosLabel->SetContentAlignment(Label::a_west);

	// WeaponPos ComboBox: x=352, y=120
	m_pWeaponPos->SetPos(rightColX + centerOffset, startY + PROPVAL(120));
	m_pWeaponPos->SetSize(comboWidth, controlHeight);

	// Divider: x=16, y=152, wide=464
	m_pDivider1->SetPos(centerOffset, startY + PROPVAL(152));
	m_pDivider1->SetSize(dividerWidth, 2);

	// ================== Checkboxes ==================
	// CloseOnBuy: x=16, y=162
	m_pCloseOnBuy->SetPos(centerOffset, startY + PROPVAL(162));
	m_pCloseOnBuy->SetSize(controlWidth, controlHeight);

	// UseOpensBuyMenu: x=16, y=186
	m_pUseOpensBuyMenu->SetPos(centerOffset, startY + PROPVAL(186));
	m_pUseOpensBuyMenu->SetSize(controlWidth, controlHeight);

	// AddBotPrefix: x=16, y=210
	m_pAddBotPrefix->SetPos(centerOffset, startY + PROPVAL(210));
	m_pAddBotPrefix->SetSize(controlWidth, controlHeight);

	// DrawTracers: x=16, y=234
	m_pDrawTracers->SetPos(centerOffset, startY + PROPVAL(234));
	m_pDrawTracers->SetSize(controlWidth, controlHeight);

	// SpecInterpCamera: x=16, y=258
	m_pSpecInterpCamera->SetPos(centerOffset, startY + PROPVAL(258));
	m_pSpecInterpCamera->SetSize(controlWidth, controlHeight);

	// DisableShootingEffects: x=16, y=282
	m_pDisableShootingEffects->SetPos(centerOffset, startY + PROPVAL(282));
	m_pDisableShootingEffects->SetSize(controlWidth, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubGameplay::~CModOptionsSubGameplay()
{
}

void CModOptionsSubGameplay::UpdateViewmodelSliderLabels()
{
	char strValue[8];
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetX->GetSliderValue() );
	m_pViewmodelOffsetXLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetY->GetSliderValue() );
	m_pViewmodelOffsetYLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetZ->GetSliderValue() );
	m_pViewmodelOffsetZLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelFOV->GetSliderValue() );
	m_pViewmodelFOVLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelRecoil->GetSliderValue() );
	m_pViewmodelRecoilLabel->SetText( strValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

void CModOptionsSubGameplay::OnTextChanged( vgui::Panel *panel )
{
	if ( panel == m_pViewmodelOffsetPreset )
	{
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 0 )
		{
			m_pViewmodelOffsetX->SetSliderValue(1);
			m_pViewmodelOffsetY->SetSliderValue(1);
			m_pViewmodelOffsetZ->SetSliderValue(-1);
			m_pViewmodelFOV->SetSliderValue(60);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 1 )
		{
			m_pViewmodelOffsetX->SetSliderValue(0);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(0);
			m_pViewmodelFOV->SetSliderValue(54);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 2 )
		{
			m_pViewmodelOffsetX->SetSliderValue(2.5f);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(-1.5f);
			m_pViewmodelFOV->SetSliderValue(68);
			UpdateViewmodelSliderLabels();
		}
	}
}

void CModOptionsSubGameplay::OnSliderMoved( KeyValues *data )
{
	vgui::Panel* pPanel = static_cast<vgui::Panel*>(data->GetPtr( "panel" ));

	if ( pPanel == m_pViewmodelOffsetX || pPanel == m_pViewmodelOffsetY || pPanel == m_pViewmodelOffsetZ || pPanel == m_pViewmodelFOV || pPanel == m_pViewmodelRecoil )
	{
		UpdateViewmodelSliderLabels();
	}
	else if ( pPanel == m_pVScrollBar )
	{
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnScrollBarSliderMoved( KeyValues *data )
{
	int position = data->GetInt( "position", 0 );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnResetData()
{
	m_pCloseOnBuy->Reset();
	m_pUseOpensBuyMenu->Reset();
	m_pAddBotPrefix->Reset();
	m_pDrawTracers->Reset();
	m_pSpecInterpCamera->Reset();
	m_pDisableShootingEffects->Reset();
	m_pViewmodelOffsetX->Reset();
	m_pViewmodelOffsetY->Reset();
	m_pViewmodelOffsetZ->Reset();
	m_pViewmodelFOV->Reset();
	m_pViewmodelRecoil->Reset();
	UpdateViewmodelSliderLabels();
	
	ConVarRef viewmodel_presetpos( "viewmodel_presetpos" );
	if ( viewmodel_presetpos.IsValid() )
		m_pViewmodelOffsetPreset->SetInitialItem( viewmodel_presetpos.GetInt() - 1 );

	ConVarRef cl_use_new_headbob( "cl_use_new_headbob" );
	if ( cl_use_new_headbob.IsValid() )
		m_pViewbobStyle->SetInitialItem( cl_use_new_headbob.GetInt() );

	ConVarRef cl_righthand( "cl_righthand" );
	if ( cl_righthand.IsValid() )
		m_pWeaponPos->SetInitialItem( cl_righthand.GetInt() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnApplyChanges()
{
	m_pCloseOnBuy->ApplyChanges();
	m_pUseOpensBuyMenu->ApplyChanges();
	m_pAddBotPrefix->ApplyChanges();
	m_pDrawTracers->ApplyChanges();
	m_pSpecInterpCamera->ApplyChanges();
	m_pDisableShootingEffects->ApplyChanges();
	m_pViewmodelOffsetPreset->ApplyChanges();
	m_pViewmodelOffsetX->ApplyChanges();
	m_pViewmodelOffsetY->ApplyChanges();
	m_pViewmodelOffsetZ->ApplyChanges();
	m_pViewmodelFOV->ApplyChanges();
	m_pViewmodelRecoil->ApplyChanges();
	m_pViewbobStyle->ApplyChanges();
	m_pWeaponPos->ApplyChanges();
}
