//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BasePanel.h"
#include "ModOptionsDialog.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "vgui/IScheme.h"

#include "KeyValues.h"
#include "ModOptionsSubGameplay.h"
#include "ModOptionsSubCrosshair.h"
#include "ModOptionsSubLoadout.h"
#include "ModOptionsSubKnives.h"
#include "ModOptionsSubAgents.h"
#include "ModOptionsSubGloves.h"
#include "ModOptionsSubHUD.h"
#include "cstrike/VGUI/vgui_skin_editor.h"
#include "ModInfo.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog - Fullscreen Style
//-----------------------------------------------------------------------------
CModOptionsDialog::CModOptionsDialog(vgui::Panel *parent) : PropertyDialog(parent, "ModOptionsDialog")
{
	SetDeleteSelfOnClose(true);
	SetMoveable(false);
    SetSizeable(false);
    SetCloseButtonVisible(false);

	// Get screen size for fullscreen
	int screenW, screenH;
	vgui::surface()->GetScreenSize(screenW, screenH);

	// Set fullscreen bounds
	SetBounds(0, 0, screenW, screenH);
	
	SetSizeable(false);
	SetTitle("", false); // Hide title for fullscreen

	// Set border and background color
	IScheme* pScheme = vgui::scheme()->GetIScheme(GetScheme());
	SetBorder(pScheme->GetBorder("FrameBorder"));
	SetBgColor(pScheme->GetColor("Frame.BgColor", Color(0, 0, 0, 200)));
	SetPaintBackgroundEnabled(true);
    
	// Create sub-pages
	CModOptionsSubGameplay* pGameplay = new CModOptionsSubGameplay(this);
	CModOptionsSubCrosshair* pCrosshair = new CModOptionsSubCrosshair(this);
	CModOptionsSubLoadout* pLoadout = new CModOptionsSubLoadout(this);
	CModOptionsSubKnives* pKnives = new CModOptionsSubKnives(this);
	CModOptionsSubAgents* pAgents = new CModOptionsSubAgents(this);
	CModOptionsSubGloves* pGloves = new CModOptionsSubGloves(this);
	CModOptionsSubHUD* pHUD = new CModOptionsSubHUD(this);
	CSkinEditorPanel* pSkins = new CSkinEditorPanel(this);

    AddPage(pLoadout, "#GameUI_Loadout");
    AddPage(pSkins, "All");
	AddPage(pGameplay, "#GameUI_Gameplay");
	AddPage(pCrosshair, "#GameUI_Crosshair");	
	AddPage(pKnives, "#GameUI_Knives");
	AddPage(pAgents, "#GameUI_Agents");
	AddPage(pGloves, "#GameUI_Gloves");
	AddPage(pHUD, "#GameUI_HUD");
	

	SetApplyButtonVisible(true);
	
	// Hide the default tab bar and use full space
	GetPropertySheet()->SetTabWidth(0);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CModOptionsDialog::~CModOptionsDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Brings the dialog to the fore
//-----------------------------------------------------------------------------
void CModOptionsDialog::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);
}

void CModOptionsDialog::OnKeyCodePressed( KeyCode code )
{
	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		OnCommand( "Cancel" );
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

//-----------------------------------------------------------------------------
// Purpose: Opens the dialog
//-----------------------------------------------------------------------------
void CModOptionsDialog::Run()
{
	SetTitle("#GameUI_Mod_Options", true);
	Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the GameUI is hidden
//-----------------------------------------------------------------------------
void CModOptionsDialog::OnGameUIHidden()
{
	// tell our children about it
	for ( int i = 0 ; i < GetChildCount() ; i++ )
	{
		Panel *pChild = GetChild( i );
		if ( pChild )
		{
			PostMessage( pChild, new KeyValues( "GameUIHidden" ) );
		}
	}
}

void CModOptionsDialog::PaintBackground()
{
	m_BlurHelper.DrawBlur(this, 160.0f);
}