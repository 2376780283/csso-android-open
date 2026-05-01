//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "vgui_depth_ui.h"
#include "filesystem.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Slider.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

// Forward declaration of external ConVars
extern ConVar mat_dof_override;
extern ConVar mat_dof_near_blur_depth;
extern ConVar mat_dof_near_focus_depth;
extern ConVar mat_dof_far_focus_depth;
extern ConVar mat_dof_far_blur_depth;
extern ConVar mat_dof_near_blur_radius;
extern ConVar mat_dof_far_blur_radius;

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CDepthMenu::CDepthMenu( IViewPort* pViewPort ): Frame( NULL, GetName() )
{
	m_pViewPort = pViewPort;
	SetTitle( "", true );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( false );	
	MakePopup();
	SetMouseInputEnabled( true );
	InvalidateLayout();
	
	// Initialize pointers
	m_pNearBlurDepthSlider = NULL;
	m_pNearFocusDepthSlider = NULL;
	m_pFarFocusDepthSlider = NULL;
	m_pFarBlurDepthSlider = NULL;
	m_pNearBlurRadiusSlider = NULL;
	m_pFarBlurRadiusSlider = NULL;
	
	m_pNearBlurDepthLabel = NULL;
	m_pNearFocusDepthLabel = NULL;
	m_pFarFocusDepthLabel = NULL;
	m_pFarBlurDepthLabel = NULL;
	m_pNearBlurRadiusLabel = NULL;
	m_pFarBlurRadiusLabel = NULL;
	
	m_pResetButton = NULL;
	
	CreateDOFControls();
}


void CDepthMenu::CreateDOFControls()
{
	// Near Blur Depth
	m_pNearBlurDepthLabel = new Label( this, "NearBlurDepthLabel", "Near Blur Depth:" );
	m_pNearBlurDepthSlider = new Slider( this, "NearBlurDepthSlider" );
	m_pNearBlurDepthSlider->SetRange( 0, 100 );
	m_pNearBlurDepthSlider->SetValue( (int)mat_dof_near_blur_depth.GetFloat() );
	m_pNearBlurDepthSlider->AddActionSignalTarget( this );
	
	// Near Focus Depth
	m_pNearFocusDepthLabel = new Label( this, "NearFocusDepthLabel", "Near Focus Depth:" );
	m_pNearFocusDepthSlider = new Slider( this, "NearFocusDepthSlider" );
	m_pNearFocusDepthSlider->SetRange( 0, 500 );
	m_pNearFocusDepthSlider->SetValue( (int)mat_dof_near_focus_depth.GetFloat() );
	m_pNearFocusDepthSlider->AddActionSignalTarget( this );
	
	// Far Focus Depth
	m_pFarFocusDepthLabel = new Label( this, "FarFocusDepthLabel", "Far Focus Depth:" );
	m_pFarFocusDepthSlider = new Slider( this, "FarFocusDepthSlider" );
	m_pFarFocusDepthSlider->SetRange( 0, 1000 );
	m_pFarFocusDepthSlider->SetValue( (int)mat_dof_far_focus_depth.GetFloat() );
	m_pFarFocusDepthSlider->AddActionSignalTarget( this );
	
	// Far Blur Depth
	m_pFarBlurDepthLabel = new Label( this, "FarBlurDepthLabel", "Far Blur Depth:" );
	m_pFarBlurDepthSlider = new Slider( this, "FarBlurDepthSlider" );
	m_pFarBlurDepthSlider->SetRange( 0, 2000 );
	m_pFarBlurDepthSlider->SetValue( (int)mat_dof_far_blur_depth.GetFloat() );
	m_pFarBlurDepthSlider->AddActionSignalTarget( this );
	
	// Near Blur Radius
	m_pNearBlurRadiusLabel = new Label( this, "NearBlurRadiusLabel", "Near Blur Radius:" );
	m_pNearBlurRadiusSlider = new Slider( this, "NearBlurRadiusSlider" );
	m_pNearBlurRadiusSlider->SetRange( 0, 50 );
	m_pNearBlurRadiusSlider->SetValue( (int)mat_dof_near_blur_radius.GetFloat() );
	m_pNearBlurRadiusSlider->AddActionSignalTarget( this );
	
	// Far Blur Radius
	m_pFarBlurRadiusLabel = new Label( this, "FarBlurRadiusLabel", "Far Blur Radius:" );
	m_pFarBlurRadiusSlider = new Slider( this, "FarBlurRadiusSlider" );
	m_pFarBlurRadiusSlider->SetRange( 0, 50 );
	m_pFarBlurRadiusSlider->SetValue( (int)mat_dof_far_blur_radius.GetFloat() );
	m_pFarBlurRadiusSlider->AddActionSignalTarget( this );
	
	// Reset Button
	m_pResetButton = new Button( this, "ResetButton", "Reset to Defaults" );
	m_pResetButton->AddActionSignalTarget( this );
	
	InvalidateLayout(true, true);
}

void CDepthMenu::PerformLayout()
{
	BaseClass::PerformLayout();
	
	int panelWidth = GetWide();
	int controlWidth = panelWidth - 20;
	
	int yPos = 10;
	const int labelHeight = 18;
	const int sliderHeight = 18;
	const int itemSpacing = 25;
	const int groupSpacing = 8;
	
	// Near Blur Depth
	m_pNearBlurDepthLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pNearBlurDepthSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing;
	
	// Near Focus Depth
	m_pNearFocusDepthLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pNearFocusDepthSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing;
	
	// Far Focus Depth
	m_pFarFocusDepthLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pFarFocusDepthSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing;
	
	// Far Blur Depth
	m_pFarBlurDepthLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pFarBlurDepthSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing;
	
	// Near Blur Radius
	m_pNearBlurRadiusLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pNearBlurRadiusSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing;
	
	// Far Blur Radius
	m_pFarBlurRadiusLabel->SetBounds( 10, yPos, controlWidth, labelHeight );
	yPos += itemSpacing;
	m_pFarBlurRadiusSlider->SetBounds( 10, yPos, controlWidth, sliderHeight );
	yPos += itemSpacing + groupSpacing + 5;
	
	// Reset Button
	m_pResetButton->SetBounds( 10, yPos, controlWidth, 28 );
	yPos += 40;
	
	SetSize( 260, yPos );
}

void CDepthMenu::UpdateDOFLabels()
{
	char buf[64];
	
	Q_snprintf( buf, sizeof(buf), "Near Blur Depth: %.1f", m_pNearBlurDepthSlider->GetValue() );
	m_pNearBlurDepthLabel->SetText( buf );
	
	Q_snprintf( buf, sizeof(buf), "Near Focus Depth: %.1f", m_pNearFocusDepthSlider->GetValue() );
	m_pNearFocusDepthLabel->SetText( buf );
	
	Q_snprintf( buf, sizeof(buf), "Far Focus Depth: %.1f", m_pFarFocusDepthSlider->GetValue() );
	m_pFarFocusDepthLabel->SetText( buf );
	
	Q_snprintf( buf, sizeof(buf), "Far Blur Depth: %.1f", m_pFarBlurDepthSlider->GetValue() );
	m_pFarBlurDepthLabel->SetText( buf );
	
	Q_snprintf( buf, sizeof(buf), "Near Blur Radius: %.1f", m_pNearBlurRadiusSlider->GetValue() );
	m_pNearBlurRadiusLabel->SetText( buf );
	
	Q_snprintf( buf, sizeof(buf), "Far Blur Radius: %.1f", m_pFarBlurRadiusSlider->GetValue() );
	m_pFarBlurRadiusLabel->SetText( buf );
}

void CDepthMenu::ResetDOFToDefaults()
{
	mat_dof_near_blur_depth.SetValue( 20.0f );
	mat_dof_near_focus_depth.SetValue( 100.0f );
	mat_dof_far_focus_depth.SetValue( 250.0f );
	mat_dof_far_blur_depth.SetValue( 1000.0f );
	mat_dof_near_blur_radius.SetValue( 10.0f );
	mat_dof_far_blur_radius.SetValue( 5.0f );
	
	m_pNearBlurDepthSlider->SetValue( 20 );
	m_pNearFocusDepthSlider->SetValue( 100 );
	m_pFarFocusDepthSlider->SetValue( 250 );
	m_pFarBlurDepthSlider->SetValue( 1000 );
	m_pNearBlurRadiusSlider->SetValue( 10 );
	m_pFarBlurRadiusSlider->SetValue( 5 );
	
	UpdateDOFLabels();
}

void CDepthMenu::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "ResetButton" ) )
	{
		ResetDOFToDefaults();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CDepthMenu::OnSliderMoved()
{
	// Update ConVars based on slider changes
	if ( m_pNearBlurDepthSlider )
	{
		mat_dof_near_blur_depth.SetValue( (float)m_pNearBlurDepthSlider->GetValue() );
	}
	if ( m_pNearFocusDepthSlider )
	{
		mat_dof_near_focus_depth.SetValue( (float)m_pNearFocusDepthSlider->GetValue() );
	}
	if ( m_pFarFocusDepthSlider )
	{
		mat_dof_far_focus_depth.SetValue( (float)m_pFarFocusDepthSlider->GetValue() );
	}
	if ( m_pFarBlurDepthSlider )
	{
		mat_dof_far_blur_depth.SetValue( (float)m_pFarBlurDepthSlider->GetValue() );
	}
	if ( m_pNearBlurRadiusSlider )
	{
		mat_dof_near_blur_radius.SetValue( (float)m_pNearBlurRadiusSlider->GetValue() );
	}
	if ( m_pFarBlurRadiusSlider )
	{
		mat_dof_far_blur_radius.SetValue( (float)m_pFarBlurRadiusSlider->GetValue() );
	}
	
	UpdateDOFLabels();
}

void CDepthMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		SetTitleBarVisible( false );
		Activate();
		MoveToCenterOfScreen();
		mat_dof_override.SetValue( 1 );
		UpdateDOFLabels();
	}
	else
	{
		Close();		
	}
	m_pViewPort->ShowBackGround( bShow );
}

extern ConVar mat_blur_strength;
extern ConVar mat_blur_desaturate;
void CDepthMenu::PaintBackground()
{
        if ( engine->GetDXSupportLevel() < 90 )
                BaseClass::PaintBackground();
        else
        {
                int x, y, w, h;
                GetBounds( x, y, w, h );
                DoBlurFade( mat_blur_strength.GetFloat(), mat_blur_desaturate.GetFloat(), x, y, w, h );
        }
}

void CDepthMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );	
	SetPaintBackgroundType( 2 );
	SetPaintBorderEnabled( true );
	SetPaintBackgroundEnabled( true );
}