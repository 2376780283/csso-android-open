//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubCrosshair.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/FileOpenDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ScrollBar.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "filesystem.h"
#include "EngineInterface.h"
#include "BitmapImagePanel.h"
#include "tier1/utlbuffer.h"
#include "ModInfo.h"
#include "tier1/convar.h"
#include "tier0/icommandline.h"

#ifdef WIN32
#include <io.h>
#endif

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct ColorItem_t
{
	const char	*name;
	int			r, g, b;
};

static ColorItem_t s_crosshairColors[] = 
{
	{ "#Valve_Red",		250,	50,		50 },
	{ "#Valve_Green",	50,		250,	50 },
	{ "#Valve_Yellow",	250,	250,	50 },
	{ "#Valve_Blue",	50,		50,		250 },
	{ "#Valve_Ltblue",	50,		250,	250 },
	{ "#GameUI_Crosshair_Color_Custom", 0, 0, 0 }
};

static const char* crosshairBackgroundImages[] =
{
	"crosshair/de_dust2",
	"crosshair/de_mirage",
	"crosshair/de_aztec",
	"crosshair/cs_office",
};


//-----------------------------------------------------------------------------
class CrosshairImagePanelCS : public CrosshairImagePanelBase
{
	DECLARE_CLASS_SIMPLE( CrosshairImagePanelCS, CrosshairImagePanelBase );

public:
	CrosshairImagePanelCS( Panel *parent, const char *name, CModOptionsSubCrosshair* pOptionsPanel );
	virtual void ResetData();
	virtual void ApplyChanges();

	// Public access to controls for layout positioning
	CLabeledCommandComboBox	*m_pCrosshairStyle;
	CCvarSlider				*m_pCrosshairAlpha;
	CCvarToggleCheckButton	*m_pCrosshairUseAlpha;
	CCvarSlider				*m_pCrosshairGap;
	CCvarToggleCheckButton	*m_pCrosshairGapUseWeaponValue;
	CCvarSlider				*m_pCrosshairSize;
	CCvarSlider				*m_pCrosshairThickness;
	CCvarToggleCheckButton	*m_pCrosshairDot;
	CCvarSlider				*m_pCrosshairColorR;
	CCvarSlider				*m_pCrosshairColorG;
	CCvarSlider				*m_pCrosshairColorB;
	CCvarToggleCheckButton	*m_pCrosshairDrawOutline;
	CCvarSlider				*m_pCrosshairOutlineThickness;
	CCvarToggleCheckButton	*m_pCrosshairT;
	CLabeledCommandComboBox	*m_pCrosshairColor;

protected:
	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );

	virtual void Paint();
	void DrawCrosshairRect( int r, int g, int b, int a, int x0, int y0, int x1, int y1, bool bAdditive );
	void UpdateCrosshair();

private:
	CModOptionsSubCrosshair	*m_pOptionsPanel;
	int m_iCrosshairTextureID;
};

//-----------------------------------------------------------------------------
CrosshairImagePanelCS::CrosshairImagePanelCS( Panel *parent, const char *name, CModOptionsSubCrosshair* pOptionsPanel ) : CrosshairImagePanelBase( parent, name )
{
	m_pOptionsPanel = pOptionsPanel;

	// Get the scroll container from parent options panel
	Panel* pScrollContainer = pOptionsPanel->GetScrollContainer();

	// Create all controls inside the scroll container
	m_pCrosshairStyle = new CLabeledCommandComboBox( pScrollContainer, "CrosshairStyle" );
	m_pCrosshairAlpha = new CCvarSlider( pScrollContainer, "CrosshairAlpha", "#GameUI_Crosshair_Alpha", 0.0f, 255.0f, "cl_crosshairalpha" );
	m_pCrosshairUseAlpha = new CCvarToggleCheckButton( pScrollContainer, "CrosshairUseAlpha", "#GameUI_Crosshair_UseAlpha", "cl_crosshairusealpha" );
	m_pCrosshairGap = new CCvarSlider( pScrollContainer, "CrosshairGap", "#GameUI_Crosshair_Gap", -5.0f, 5.0f, "cl_crosshairgap" );
	m_pCrosshairGapUseWeaponValue = new CCvarToggleCheckButton( pScrollContainer, "CrosshairGapUseWeaponValue", "#GameUI_Crosshair_Gap_UseWeaponValue", "cl_crosshairgap_useweaponvalue" );
	m_pCrosshairSize = new CCvarSlider( pScrollContainer, "CrosshairSize", "#GameUI_Crosshair_Size", 0.0f, 10.0f, "cl_crosshairsize" );
	m_pCrosshairThickness = new CCvarSlider( pScrollContainer, "CrosshairThickness", "#GameUI_Crosshair_Thickness", 0.1f, 6.0f, "cl_crosshairthickness" );
	m_pCrosshairDot = new CCvarToggleCheckButton( pScrollContainer, "CrosshairDot", "#GameUI_CrosshairDot", "cl_crosshairdot" );
	m_pCrosshairColorR = new CCvarSlider( pScrollContainer, "CrosshairColorR", "#GameUI_Crosshair_Color_R", 0.0f, 255.0f, "cl_crosshaircolor_r" );
	m_pCrosshairColorG = new CCvarSlider( pScrollContainer, "CrosshairColorG", "#GameUI_Crosshair_Color_G", 0.0f, 255.0f, "cl_crosshaircolor_g" );
	m_pCrosshairColorB = new CCvarSlider( pScrollContainer, "CrosshairColorB", "#GameUI_Crosshair_Color_B", 0.0f, 255.0f, "cl_crosshaircolor_b" );
	m_pCrosshairDrawOutline = new CCvarToggleCheckButton( pScrollContainer, "CrosshairDrawOutline", "#GameUI_Crosshair_DrawOutline", "cl_crosshair_drawoutline" );
	m_pCrosshairOutlineThickness = new CCvarSlider( pScrollContainer, "CrosshairOutlineThickness", "#GameUI_Crosshair_OutlineThickness", 0.0f, 3.0f, "cl_crosshair_outlinethickness" );
	m_pCrosshairT = new CCvarToggleCheckButton( pScrollContainer, "CrosshairT", "#GameUI_Crosshair_T", "cl_crosshair_t" );
	m_pCrosshairColor = new CLabeledCommandComboBox( pScrollContainer, "CrosshairColor" );

	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_0", "cl_crosshairstyle 0" );
	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_1", "cl_crosshairstyle 1" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_2", "cl_crosshairstyle 2" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_3", "cl_crosshairstyle 3" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_4", "cl_crosshairstyle 4" );
	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_5", "cl_crosshairstyle 5" ); // deprecated in CSGO

	for ( int i = 0; i < ARRAYSIZE( s_crosshairColors ); i++ )
	{
		char command[64];
		Q_snprintf( command, sizeof( command ), "cl_crosshaircolor %d", i );
		m_pCrosshairColor->AddItem( s_crosshairColors[i].name, command );
	}

	m_pCrosshairStyle->AddActionSignalTarget( this );
	m_pCrosshairAlpha->AddActionSignalTarget( this );
	m_pCrosshairUseAlpha->AddActionSignalTarget( this );
	m_pCrosshairGap->AddActionSignalTarget( this );
	m_pCrosshairGapUseWeaponValue->AddActionSignalTarget( this );
	m_pCrosshairSize->AddActionSignalTarget( this );
	m_pCrosshairThickness->AddActionSignalTarget( this );
	m_pCrosshairDot->AddActionSignalTarget( this );
	m_pCrosshairColorR->AddActionSignalTarget( this );
	m_pCrosshairColorG->AddActionSignalTarget( this );
	m_pCrosshairColorB->AddActionSignalTarget( this );
	m_pCrosshairDrawOutline->AddActionSignalTarget( this );
	m_pCrosshairOutlineThickness->AddActionSignalTarget( this );
	m_pCrosshairT->AddActionSignalTarget( this );
	m_pCrosshairColor->AddActionSignalTarget( this );

	m_iCrosshairTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iCrosshairTextureID, "vgui/white_additive" , true, false);

	ResetData();
}

//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::DrawCrosshairRect( int r, int g, int b, int a, int x0, int y0, int x1, int y1, bool bAdditive )
{
	if ( m_pCrosshairDrawOutline->IsSelected() )
	{
		float flThick = m_pCrosshairOutlineThickness->GetSliderValue();
		vgui::surface()->DrawSetColor( 0, 0, 0, a );
		vgui::surface()->DrawFilledRect( x0-flThick, y0-flThick, x1+flThick, y1+flThick );
	}

	vgui::surface()->DrawSetColor( r, g, b, a );

	if ( bAdditive )
	{
		vgui::surface()->DrawTexturedRect( x0, y0, x1, y1 );
	}
	else
	{
		// Alpha-blended crosshair
		vgui::surface()->DrawFilledRect( x0, y0, x1, y1 );
	}
}

//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::Paint()
{
	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );;

	BaseClass::Paint();

	int wide, tall;
	GetSize( wide, tall );

	bool bAdditive = !m_pCrosshairUseAlpha->IsSelected();

	int a = 200;
	if ( !bAdditive )
		a = m_pCrosshairAlpha->GetSliderValue();

	int r, g, b;
	switch ( m_pCrosshairColor->GetActiveItem() )
	{
		case 0:	r = 250;	g = 50;		b = 50;		break;
		case 1:	r = 50;		g = 250;	b = 50;		break;
		case 2:	r = 250;	g = 250;	b = 50;		break;
		case 3:	r = 50;		g = 50;		b = 250;	break;
		case 4:	r = 50;		g = 250;	b = 250;	break;
		case 5:
			r = m_pCrosshairColorR->GetSliderValue();
			g = m_pCrosshairColorG->GetSliderValue();
			b = m_pCrosshairColorB->GetSliderValue();
			break;
		default:	r = 50;		g = 250;	b = 50;		break;
	}

	vgui::surface()->DrawSetColor( r, g, b, a );

	if ( bAdditive )
	{
		vgui::surface()->DrawSetTexture( m_iCrosshairTextureID );
	}

	int centerX = wide / 2;
	int centerY = tall / 2;

	int iBarSize = RoundFloatToInt(m_pCrosshairSize->GetSliderValue() * screenTall / 480.0f);
	int iBarThickness = max(1, RoundFloatToInt(m_pCrosshairThickness->GetSliderValue() * (float)screenTall / 480.0f));

	int iBarGap = m_pCrosshairGap->GetSliderValue() + 4;

	// draw horizontal crosshair lines
	int iInnerLeft = centerX - iBarGap - iBarThickness / 2;
	int iInnerRight = iInnerLeft + 2 * iBarGap + iBarThickness;
	int iOuterLeft = iInnerLeft - iBarSize;
	int iOuterRight = iInnerRight + iBarSize;
	int y0 = centerY - iBarThickness / 2;
	int y1 = y0 + iBarThickness;
	DrawCrosshairRect( r, g, b, a, iOuterLeft, y0, iInnerLeft, y1, bAdditive );
	DrawCrosshairRect( r, g, b, a, iInnerRight, y0, iOuterRight, y1, bAdditive );

	// draw vertical crosshair lines
	int iInnerTop = centerY - iBarGap - iBarThickness / 2;
	int iInnerBottom = iInnerTop + 2 * iBarGap + iBarThickness;
	int iOuterTop = iInnerTop - iBarSize;
	int iOuterBottom = iInnerBottom + iBarSize;
	int x0 = centerX - iBarThickness / 2;
	int x1 = x0 + iBarThickness;
	if ( !m_pCrosshairT->IsSelected() )
		DrawCrosshairRect( r, g, b, a, x0, iOuterTop, x1, iInnerTop, bAdditive );
	DrawCrosshairRect( r, g, b, a, x0, iInnerBottom, x1, iOuterBottom, bAdditive );

	// draw dot
	if ( m_pCrosshairDot->IsSelected() )
	{
		x0 = centerX - iBarThickness / 2;
		x1 = x0 + iBarThickness;
		y0 = centerY - iBarThickness / 2;
		y1 = y0 + iBarThickness;
		DrawCrosshairRect( r, g, b, a, x0, y0, x1, y1, bAdditive );
	}
}


//-----------------------------------------------------------------------------
// Purpose: takes the settings from the crosshair settings combo boxes and sliders
//          and apply it to the crosshair illustrations.
//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::UpdateCrosshair()
{
}


void CrosshairImagePanelCS::OnSliderMoved(KeyValues *data)
{
	m_pOptionsPanel->OnControlModified();

	UpdateCrosshair();
}


//-----------------------------------------------------------------------------
// Purpose: Called whenever color combo changes
//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::OnTextChanged(vgui::Panel *panel)
{
	m_pCrosshairColorR->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pCrosshairColorG->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pCrosshairColorB->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pOptionsPanel->OnControlModified();
	UpdateCrosshair();
}

void CrosshairImagePanelCS::OnCheckButtonChecked()
{
	m_pCrosshairAlpha->SetEnabled(m_pCrosshairUseAlpha->IsSelected());
	m_pCrosshairOutlineThickness->SetEnabled(m_pCrosshairDrawOutline->IsSelected());
	m_pOptionsPanel->OnControlModified();
	UpdateCrosshair();
}

void CrosshairImagePanelCS::ResetData()
{
	ConVarRef cl_crosshairstyle( "cl_crosshairstyle" );
	m_pCrosshairStyle->SetInitialItem( Clamp( cl_crosshairstyle.GetInt() - 2, 0, 2 ) ); // remove Clamp if unlocking other styles

	m_pCrosshairAlpha->Reset();
	m_pCrosshairUseAlpha->Reset();
	m_pCrosshairGap->Reset();
	m_pCrosshairGapUseWeaponValue->Reset();
	m_pCrosshairSize->Reset();
	m_pCrosshairThickness->Reset();
	m_pCrosshairDot->Reset();
	m_pCrosshairColorR->Reset();
	m_pCrosshairColorG->Reset();
	m_pCrosshairColorB->Reset();
	m_pCrosshairDrawOutline->Reset();
	m_pCrosshairOutlineThickness->Reset();
	m_pCrosshairT->Reset();

	ConVarRef cl_crosshaircolor( "cl_crosshaircolor" );
	m_pCrosshairColor->SetInitialItem( cl_crosshaircolor.GetInt() );

	SetImage( crosshairBackgroundImages[RandomInt(0, ARRAYSIZE(crosshairBackgroundImages) - 1)] ); // bruh

	UpdateCrosshair();
}

void CrosshairImagePanelCS::ApplyChanges()
{
	m_pCrosshairStyle->ApplyChanges();
	m_pCrosshairAlpha->ApplyChanges();
	m_pCrosshairUseAlpha->ApplyChanges();
	m_pCrosshairGap->ApplyChanges();
	m_pCrosshairGapUseWeaponValue->ApplyChanges();
	m_pCrosshairSize->ApplyChanges();
	m_pCrosshairThickness->ApplyChanges();
	m_pCrosshairDot->ApplyChanges();
	m_pCrosshairColorR->ApplyChanges();
	m_pCrosshairColorG->ApplyChanges();
	m_pCrosshairColorB->ApplyChanges();
	m_pCrosshairDrawOutline->ApplyChanges();
	m_pCrosshairOutlineThickness->ApplyChanges();
	m_pCrosshairT->ApplyChanges();
	m_pCrosshairColor->ApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout and Scroll
//-----------------------------------------------------------------------------
CModOptionsSubCrosshair::CModOptionsSubCrosshair(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubCrosshair")
{
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

	// Create the crosshair preview image - in scroll container (right column)
	m_pCrosshairImage = new CrosshairImagePanelCS(m_pScrollContainer, "CrosshairImage", this);

	// Create labels for controls inside scroll container
	// Description label (from RES file)
	new vgui::Label(m_pScrollContainer, "CrosshairLabel", "#GameUI_CrosshairDescription");

	// Style section
	new vgui::Label(m_pScrollContainer, "StyleLabel", "#GameUI_Crosshair_Style");

	// Size section
	new vgui::Label(m_pScrollContainer, "SizeLabel", "#GameUI_Crosshair_Size");
	new vgui::Label(m_pScrollContainer, "ThicknessLabel", "#GameUI_Crosshair_Thickness");
	new vgui::Label(m_pScrollContainer, "GapLabel", "#GameUI_Crosshair_Gap");

	// Color section
	new vgui::Label(m_pScrollContainer, "CrosshairColorLabel", "#GameUI_Crosshair_Color");
	new vgui::Label(m_pScrollContainer, "RedLabel", "#GameUI_Crosshair_Color_R");
	new vgui::Label(m_pScrollContainer, "GreenLabel", "#GameUI_Crosshair_Color_G");
	new vgui::Label(m_pScrollContainer, "BlueLabel", "#GameUI_Crosshair_Color_B");

	// Alpha section
	new vgui::Label(m_pScrollContainer, "AlphaLabel", "#GameUI_Crosshair_Alpha");

	// Note: CrosshairDrawOutline is a checkbutton with built-in label, no separate label needed
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - matching RES file coordinates exactly
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// RES file coordinates
	int labelHeight = PROPVAL(24);
	int sliderHeight = PROPVAL(40);
	int controlHeight = PROPVAL(24);

	// Left column X position
	int leftColX = PROPVAL(16);
	// Right column X position
	int rightColX = PROPVAL(356);
	// Slider widths from RES
	int sliderWidth = PROPVAL(128);
	int comboWidth = PROPVAL(192);

	// Calculate total width and center offset
	int totalWidth = rightColX + sliderWidth; // 356 + 128 = 484
	int centerOffset = (pw - totalWidth) / 2;

	// Calculate content height based on RES file (tall=314) + offset for moved controls
	int contentHeight = PROPVAL(350); // Increased to accommodate moved controls
	int visibleHeight = ph - PROPVAL(32);
	int scrollBarWidth = PROPVAL(16);
	int margin = PROPVAL(16);

	// Position scroll bar on the right side (adjusted for center offset)
	m_pVScrollBar->SetVisible(true);
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, margin);
	m_pVScrollBar->SetSize(scrollBarWidth, visibleHeight);
	m_pVScrollBar->SetRange(0, contentHeight - visibleHeight);
	m_pVScrollBar->SetRangeWindow(visibleHeight);

	// Position scroll container and apply scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();
	m_pScrollContainer->SetPos(margin, margin - scrollOffset);
	m_pScrollContainer->SetSize(pw - (margin * 2) - scrollBarWidth, contentHeight);

	// ================== LEFT COLUMN (x=16) ==================
	int leftX = leftColX + centerOffset;
	// CrosshairLabel: x=16, y=4, wide=128
	Panel* pCrosshairLabel = m_pScrollContainer->FindChildByName("CrosshairLabel");
	if (pCrosshairLabel)
	{
		pCrosshairLabel->SetPos(leftX, PROPVAL(4));
		pCrosshairLabel->SetSize(PROPVAL(128), labelHeight);
	}

	// CrosshairImage: x=16, y=28, wide=80, tall=80
	m_pCrosshairImage->SetPos(leftX, PROPVAL(28));
	m_pCrosshairImage->SetSize(PROPVAL(80), PROPVAL(80));

	// CrosshairDot: x=100, y=24
	m_pCrosshairImage->m_pCrosshairDot->SetPos(leftX + PROPVAL(84), PROPVAL(24));
	m_pCrosshairImage->m_pCrosshairDot->SetSize(PROPVAL(100), controlHeight);

	// CrosshairT: x=100, y=48
	m_pCrosshairImage->m_pCrosshairT->SetPos(leftX + PROPVAL(84), PROPVAL(48));
	m_pCrosshairImage->m_pCrosshairT->SetSize(PROPVAL(100), controlHeight);

	// CrosshairGapUseWeaponValue: x=100, y=72, wide=256
	m_pCrosshairImage->m_pCrosshairGapUseWeaponValue->SetPos(leftX + PROPVAL(84), PROPVAL(72));
	m_pCrosshairImage->m_pCrosshairGapUseWeaponValue->SetSize(PROPVAL(256), controlHeight);

	// CrosshairUseAlpha: x=100, y=96 (between Gap and Outline)
	m_pCrosshairImage->m_pCrosshairUseAlpha->SetPos(leftX + PROPVAL(84), PROPVAL(96));
	m_pCrosshairImage->m_pCrosshairUseAlpha->SetSize(PROPVAL(192), controlHeight);

	// CrosshairSize: x=16, y=120, wide=128, tall=40
	// SizeLabel: pin_to_sibling=CrosshairSize, pin_corner_to_sibling=0, pin_to_sibling_corner=1 (above left)
	Panel* pSizeLabel = m_pScrollContainer->FindChildByName("SizeLabel");
	if (pSizeLabel)
	{
		pSizeLabel->SetPos(leftX, PROPVAL(120) - labelHeight);
		pSizeLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairSize->SetPos(leftX, PROPVAL(120));
	m_pCrosshairImage->m_pCrosshairSize->SetSize(sliderWidth, sliderHeight);

	// CrosshairThickness: x=16, y=156, wide=128, tall=40
	// ThicknessLabel: pin_to_sibling=CrosshairThickness, pin_corner_to_sibling=0, pin_to_sibling_corner=1
	Panel* pThicknessLabel = m_pScrollContainer->FindChildByName("ThicknessLabel");
	if (pThicknessLabel)
	{
		pThicknessLabel->SetPos(leftX, PROPVAL(156) - labelHeight);
		pThicknessLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairThickness->SetPos(leftX, PROPVAL(156));
	m_pCrosshairImage->m_pCrosshairThickness->SetSize(sliderWidth, sliderHeight);

	// CrosshairGap: x=16, y=192, wide=128, tall=40
	// GapLabel: pin_to_sibling=CrosshairGap, pin_corner_to_sibling=0, pin_to_sibling_corner=1
	Panel* pGapLabel = m_pScrollContainer->FindChildByName("GapLabel");
	if (pGapLabel)
	{
		pGapLabel->SetPos(leftX, PROPVAL(192) - labelHeight);
		pGapLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairGap->SetPos(leftX, PROPVAL(192));
	m_pCrosshairImage->m_pCrosshairGap->SetSize(sliderWidth, sliderHeight);

	// CrosshairOutlineThickness: x=16, y=228, wide=128, tall=40
	m_pCrosshairImage->m_pCrosshairOutlineThickness->SetPos(leftX, PROPVAL(228));
	m_pCrosshairImage->m_pCrosshairOutlineThickness->SetSize(sliderWidth, sliderHeight);

	// CrosshairDrawOutline: pin_to_sibling=CrosshairOutlineThickness, pin_corner_to_sibling=0, pin_to_sibling_corner=1
	// RES: x=0, y=0 relative to sibling, positioned below the slider
	m_pCrosshairImage->m_pCrosshairDrawOutline->SetPos(leftX, PROPVAL(241) + sliderHeight);
	m_pCrosshairImage->m_pCrosshairDrawOutline->SetSize(PROPVAL(192), controlHeight);

	// StyleLabel: pin_to_sibling=CrosshairStyle, pin_corner_to_sibling=0, pin_to_sibling_corner=1
	// RES: x=8, y=0 relative to CrosshairStyle, positioned above
	Panel* pStyleLabel = m_pScrollContainer->FindChildByName("StyleLabel");
	if (pStyleLabel)
	{
		pStyleLabel->SetPos(leftX - PROPVAL(8), PROPVAL(280) - labelHeight);
		pStyleLabel->SetSize(PROPVAL(192), labelHeight);
	}

	// CrosshairStyle: x=16, y=280, wide=192, tall=24
	m_pCrosshairImage->m_pCrosshairStyle->SetPos(leftX, PROPVAL(310));
	m_pCrosshairImage->m_pCrosshairStyle->SetSize(comboWidth, controlHeight);

	// ================== RIGHT COLUMN (x=356) ==================
	int rightX = rightColX + centerOffset;
	// CrosshairColorLabel: x=356, y=10 (moved down 70 pixels)
	Panel* pColorLabel = m_pScrollContainer->FindChildByName("CrosshairColorLabel");
	if (pColorLabel)
	{
		pColorLabel->SetPos(rightX, PROPVAL(10));
		pColorLabel->SetSize(PROPVAL(128), labelHeight);
	}

	// CrosshairColor: x=356, y=98 (moved down 70 pixels)
	m_pCrosshairImage->m_pCrosshairColor->SetPos(rightX, PROPVAL(30));
	m_pCrosshairImage->m_pCrosshairColor->SetSize(PROPVAL(128), controlHeight);

	// CrosshairColorR: x=356, y=134 (moved down 70 pixels)
	// RedLabel: pin_to_sibling=CrosshairColorR, pin_corner_to_sibling=3, pin_to_sibling_corner=7 (left of top)
	Panel* pRedLabel = m_pScrollContainer->FindChildByName("RedLabel");
	if (pRedLabel)
	{
		pRedLabel->SetPos(rightX - PROPVAL(8), PROPVAL(134) - labelHeight);
		pRedLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorR->SetPos(rightX, PROPVAL(134));
	m_pCrosshairImage->m_pCrosshairColorR->SetSize(sliderWidth, sliderHeight);

	// CrosshairColorG: x=356, y=170 (moved down 70 pixels)
	// GreenLabel: pin_to_sibling=CrosshairColorG, pin_corner_to_sibling=3, pin_to_sibling_corner=7
	Panel* pGreenLabel = m_pScrollContainer->FindChildByName("GreenLabel");
	if (pGreenLabel)
	{
		pGreenLabel->SetPos(rightX - PROPVAL(8), PROPVAL(170) - labelHeight);
		pGreenLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorG->SetPos(rightX, PROPVAL(170));
	m_pCrosshairImage->m_pCrosshairColorG->SetSize(sliderWidth, sliderHeight);

	// CrosshairColorB: x=356, y=206 (moved down 70 pixels)
	// BlueLabel: pin_to_sibling=CrosshairColorB, pin_corner_to_sibling=3, pin_to_sibling_corner=7
	Panel* pBlueLabel = m_pScrollContainer->FindChildByName("BlueLabel");
	if (pBlueLabel)
	{
		pBlueLabel->SetPos(rightX - PROPVAL(8), PROPVAL(206) - labelHeight);
		pBlueLabel->SetSize(PROPVAL(64), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorB->SetPos(rightX, PROPVAL(206));
	m_pCrosshairImage->m_pCrosshairColorB->SetSize(sliderWidth, sliderHeight);

	// CrosshairAlpha: x=356, y=242 (moved down 70 pixels)
	// AlphaLabel: pin_to_sibling=CrosshairAlpha, pin_corner_to_sibling=3, pin_to_sibling_corner=7 (left of top)
	Panel* pAlphaLabel = m_pScrollContainer->FindChildByName("AlphaLabel");
	if (pAlphaLabel)
	{
		pAlphaLabel->SetPos(rightX - PROPVAL(8), PROPVAL(242) - labelHeight);
		pAlphaLabel->SetSize(PROPVAL(128), labelHeight);
	}
	m_pCrosshairImage->m_pCrosshairAlpha->SetPos(rightX, PROPVAL(242));
	m_pCrosshairImage->m_pCrosshairAlpha->SetSize(sliderWidth, sliderHeight);

	// Update visibility
	if (m_pCrosshairImage)
		m_pCrosshairImage->UpdateVisibility();
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnScrollBarSliderMoved(KeyValues *data)
{
	int position = data->GetInt("position", 0);
	// Reposition scroll container based on scroll bar position
	if (m_pScrollContainer)
	{
		int margin = PROPVAL(16);
		m_pScrollContainer->SetPos(margin, margin - position);
	}
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubCrosshair::~CModOptionsSubCrosshair()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define SUIT_HUE_START 192
#define SUIT_HUE_END 223
#define PLATE_HUE_START 160
#define PLATE_HUE_END 191

#ifdef POSIX 
typedef struct tagRGBQUAD { 
  uint8 rgbBlue;
  uint8 rgbGreen;
  uint8 rgbRed;
  uint8 rgbReserved;
} RGBQUAD;
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void PaletteHueReplace( RGBQUAD *palSrc, int newHue, int Start, int end )
{
	int i;
	float r, b, g;
	float maxcol, mincol;
	float hue, val, sat;

	hue = (float)(newHue * (360.0 / 255));

	for (i = Start; i <= end; i++)
	{
		b = palSrc[ i ].rgbBlue;
		g = palSrc[ i ].rgbGreen;
		r = palSrc[ i ].rgbRed;
		
		maxcol = max( max( r, g ), b ) / 255.0f;
		mincol = min( min( r, g ), b ) / 255.0f;
		
		val = maxcol;
		sat = (maxcol - mincol) / maxcol;

		mincol = val * (1.0f - sat);

		if (hue <= 120)
		{
			b = mincol;
			if (hue < 60)
			{
				r = val;
				g = mincol + hue * (val - mincol)/(120 - hue);
			}
			else
			{
				g = val;
				r = mincol + (120 - hue)*(val-mincol)/hue;
			}
		}
		else if (hue <= 240)
		{
			r = mincol;
			if (hue < 180)
			{
				g = val;
				b = mincol + (hue - 120)*(val-mincol)/(240 - hue);
			}
			else
			{
				b = val;
				g = mincol + (240 - hue)*(val-mincol)/(hue - 120);
			}
		}
		else
		{
			g = mincol;
			if (hue < 300)
			{
				b = val;
				r = mincol + (hue - 240)*(val-mincol)/(360 - hue);
			}
			else
			{
				r = val;
				b = mincol + (360 - hue)*(val-mincol)/(hue - 240);
			}
		}

		palSrc[ i ].rgbBlue = (unsigned char)(b * 255);
		palSrc[ i ].rgbGreen = (unsigned char)(g * 255);
		palSrc[ i ].rgbRed = (unsigned char)(r * 255);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnResetData()
{
	if ( m_pCrosshairImage )
		m_pCrosshairImage->ResetData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnApplyChanges()
{
	if ( m_pCrosshairImage != NULL )
		m_pCrosshairImage->ApplyChanges();
}
