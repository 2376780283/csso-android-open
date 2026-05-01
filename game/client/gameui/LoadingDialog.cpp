//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "LoadingDialog.h"
#include "EngineInterface.h"
#include "IGameUIFuncs.h"
#include "EngineInterface.h"
#include "vstdlib/random.h"

#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/AnimationController.h>
#include "tier0/icommandline.h"

#include "GameUI_Interface.h"
#include "ModInfo.h"
#include "BasePanel.h"
#include "gametypes.h"

#include "iclientmode.h"
#include "cs_shareddefs.h"
#include <filesystem.h>
#include "fmtstr.h"
#include "tier1/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

class CGradientProgressBar : public vgui::ContinuousProgressBar
{
    DECLARE_CLASS_SIMPLE( CGradientProgressBar, vgui::ContinuousProgressBar );
    
public:
    CGradientProgressBar( Panel *parent, const char *panelName );
    
    void SetGradientColors( Color left, Color right );
    
protected:
    virtual void PaintBackground();
    virtual void Paint();
    
private:
    Color m_LeftColor;
    Color m_RightColor;
};


//-----------------------------------------------------------------------------
// CGradientProgressBar Implementation
//-----------------------------------------------------------------------------
CGradientProgressBar::CGradientProgressBar( Panel *parent, const char *panelName ) 
    : BaseClass( parent, panelName )
{
    m_LeftColor = Color( 0, 100, 200, 255 );
    m_RightColor = Color( 100, 200, 255, 255 );
}

void CGradientProgressBar::SetGradientColors( Color left, Color right )
{
    m_LeftColor = left;
    m_RightColor = right;
}

void CGradientProgressBar::PaintBackground()
{
    BaseClass::PaintBackground();
}

void CGradientProgressBar::Paint()
{
    int wide, tall;
    GetSize( wide, tall );
    surface()->DrawSetColor( GetBgColor() );
    surface()->DrawFilledRect( 0, 0, wide, tall );
    
    float progress = GetProgress();
    int progressWide = (int)( wide * progress );
    
    if ( progressWide > 0 )
    {
        surface()->DrawSetColor( m_LeftColor );
        surface()->DrawFilledRectFade( 0, 0, progressWide, tall, 255, 0, true );
        surface()->DrawSetColor( m_RightColor );
        surface()->DrawFilledRectFade( 0, 0, progressWide, tall, 0, 255, true );
    }
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLoadingDialog::CLoadingDialog( vgui::Panel *parent ) : Frame(parent, "LoadingDialog")
{
	SetDeleteSelfOnClose(true);
	SetProportional( true );

	// Use console style
	m_bConsoleStyle = GameUI().IsConsoleUI();

	if ( !m_bConsoleStyle )
	{
		SetSize( 416, 100 );
		SetTitle( "#GameUI_Loading", true );
	}

	// center the loading dialog, unless we have another dialog to show in the background
	m_bCenter = !GameUI().HasLoadingBackgroundDialog();

	m_bShowingSecondaryProgress = false;
	m_flSecondaryProgress = 0.0f;
	m_flLastSecondaryProgressUpdateTime = 0.0f;
	m_flSecondaryProgressStartTime = 0.0f;
	m_bExtendedServerInfoLoaded = false;

    m_pTipPanel = new CLoadingTipPanel(this);
	m_pProgress = new CGradientProgressBar( this, "Progress" );
	m_pProgress2 = new ContinuousProgressBar( this, "Progress2" );
	m_pInfoLabel = new Label( this, "InfoLabel", "" );
	m_pGameModeIcon = new VectorImagePanel( this, "GameModeIcon" );
	m_pCancelButton = new Button( this, "CancelButton", "#GameUI_Cancel" );
	m_pTimeRemainingLabel = new Label( this, "TimeRemainingLabel", "" );
	m_pMapNameLabel = new Label( this, "MapNameLabel", "" );
	m_pMapImage = new ImagePanel( this, "MapImage" );
	m_pMapImageBackground = new ImagePanel( this, "MapImageBackground" );
	m_pMapIconImage = new ImagePanel( this, "MapIconImage" );
	m_pGameModeNameLabel = new Label( this, "GameModeNameLabel", "" );
	m_pGameModeDescriptionLabel = new Label( this, "GameModeDescriptionLabel", "" );
	m_pCancelButton->SetCommand( "Cancel" );
    m_pMapOverviewPanel = new vgui::Panel( this, "MapOverviewPanel" );
    m_pMapOverviewPanel->SetVisible( false );

	if ( ModInfo().IsSinglePlayerOnly() == false && m_bConsoleStyle == true )
	{
		m_pLoadingBackground = new Panel( this, "LoadingDialogBG" );
	}
	else
	{
		m_pLoadingBackground = NULL;
	}

	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );

	if ( m_bConsoleStyle )
	{
		m_bCenter = false;
		m_pProgress->SetVisible( false );
		m_pProgress2->SetVisible( false );
		m_pInfoLabel->SetVisible( false );
		m_pCancelButton->SetVisible( false );
		m_pTimeRemainingLabel->SetVisible( false );
		m_pCancelButton->SetVisible( false );
		m_pMapNameLabel->SetVisible( false );
		m_pMapImage->SetVisible( false );
		m_pMapImageBackground->SetVisible( false );
		m_pMapIconImage->SetVisible( false );
		m_pGameModeNameLabel->SetVisible( false );
		m_pGameModeDescriptionLabel->SetVisible( false );

		SetMinimumSize( 0, 0 );
		SetTitleBarVisible( false );

		m_flProgressFraction = 0;
	}
	else
	{
		m_pInfoLabel->SetBounds(20, 32, 392, 24);
		m_pProgress->SetBounds(20, 64, 300, 24); 
		m_pCancelButton->SetBounds(330, 64, 72, 24);
		m_pProgress2->SetVisible(false);
	}
    
    SetupMapIcons();

	ListenForGameEvent( "server_shutdown" );

	SetupControlSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLoadingDialog::~CLoadingDialog()
{
	if ( input()->GetAppModalSurface() == GetVPanel() )
	{
		vgui::surface()->RestrictPaintToSinglePanel( NULL );
	}
    
    ClearMapIcons();
    
	if (m_pTipPanel)
    {
        m_pTipPanel->DeletePanel();
        m_pTipPanel = NULL;
    }
}

void CLoadingDialog::SetupMapIcons()
{
    const char *iconNames[] = {
        "CTSpawn",
        "TSpawn",
        "Bomb",
        "BombA",
        "BombB",
        "Hostage1",
        "Hostage2",
        "Hostage3",
        "Hostage4",
        "Hostage5",
        "Hostage6"
    };

    for ( int i = 0; i < ARRAYSIZE(iconNames); i++ )
    {
        MapIconInfo info;
        info.pIconPanel = new vgui::ImagePanel( m_pMapOverviewPanel, CFmtStr("Icon_%s", iconNames[i]) );
        info.pIconPanel->SetShouldScaleImage( true );
        info.pIconPanel->SetVisible( false );
        info.x = 0.0f;
        info.y = 0.0f;
        info.bVisible = false;
        
        if ( Q_stristr( iconNames[i], "CTSpawn" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_ct_spawn" );
        }
        else if ( Q_stristr( iconNames[i], "TSpawn" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_t_spawn" );
        }
        else if ( Q_stristr( iconNames[i], "BombA" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_bombsite_a" );
        }
        else if ( Q_stristr( iconNames[i], "BombB" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_bombsite_b" );
        }
        else if ( Q_stristr( iconNames[i], "Bomb" ) && !Q_stristr( iconNames[i], "BombA" ) && !Q_stristr( iconNames[i], "BombB" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_c4" );
        }
        else if ( Q_stristr( iconNames[i], "Hostage" ) )
        {
            info.pIconPanel->SetImage( "vgui/hud/icon_hostage" );
        }
        
        m_MapIcons.Insert( iconNames[i], info );
    }
}

void CLoadingDialog::ClearMapIcons()
{
    for ( int i = m_MapIcons.First(); i != m_MapIcons.InvalidIndex(); i = m_MapIcons.Next(i) )
    {
        MapIconInfo &info = m_MapIcons[i];
        if ( info.pIconPanel )
        {
            info.pIconPanel->MarkForDeletion();
            info.pIconPanel = NULL;
        }
    }
    m_MapIcons.Purge();
}

void CLoadingDialog::LoadMapOverviewData( const char *mapName )
{
    if ( !mapName || !mapName[0] )
        return;

    char tempfile[MAX_PATH];
    Q_snprintf( tempfile, sizeof(tempfile), "resource/overviews/%s.txt", mapName );

    KeyValues *pMapKeyValues = new KeyValues( mapName );
    if ( !pMapKeyValues->LoadFromFile( g_pFullFileSystem, tempfile, "GAME" ) )
    {
        DevMsg( 1, "CLoadingDialog::LoadMapOverviewData: couldn't load file %s.\n", tempfile );
        pMapKeyValues->deleteThis();
        return;
    }

    int iGameType = g_pGameTypes->GetCurrentGameType();
    int iGameMode = g_pGameTypes->GetCurrentGameMode();

    bool isGunGameProgressive = ( iGameType == CS_GameType_GunGame ) && 
                                ( iGameMode == CS_GameMode::GunGame_Progressive );

    bool bShowBomb = !isGunGameProgressive;
    bool bShowHostages = !isGunGameProgressive;

    SetIconPosition( "CTSpawn", 
        pMapKeyValues->GetFloat( "CTSpawn_x" ), 
        pMapKeyValues->GetFloat( "CTSpawn_y" ) );

    SetIconPosition( "TSpawn", 
        pMapKeyValues->GetFloat( "TSpawn_x" ), 
        pMapKeyValues->GetFloat( "TSpawn_y" ) );

    if ( bShowBomb )
    {
        SetIconPosition( "Bomb", 
            pMapKeyValues->GetFloat( "bomb_x" ), 
            pMapKeyValues->GetFloat( "bomb_y" ) );

        SetIconPosition( "BombA", 
            pMapKeyValues->GetFloat( "bombA_x" ), 
            pMapKeyValues->GetFloat( "bombA_y" ) );

        SetIconPosition( "BombB", 
            pMapKeyValues->GetFloat( "bombB_x" ), 
            pMapKeyValues->GetFloat( "bombB_y" ) );
    }
    else
    {
        SetIconPosition( "Bomb", 0.0f, 0.0f );
        SetIconPosition( "BombA", 0.0f, 0.0f );
        SetIconPosition( "BombB", 0.0f, 0.0f );
    }
    
    if ( bShowHostages )
    {
        for ( int i = 1; i <= 6; i++ )
        {
            char iconName[32];
            char keyX[32];
            char keyY[32];
            
            Q_snprintf( iconName, sizeof(iconName), "Hostage%d", i );
            Q_snprintf( keyX, sizeof(keyX), "Hostage%d_x", i );
            Q_snprintf( keyY, sizeof(keyY), "Hostage%d_y", i );
            
            SetIconPosition( iconName, 
                pMapKeyValues->GetFloat( keyX ), 
                pMapKeyValues->GetFloat( keyY ) );
        }
    }
    else
    {
        for ( int i = 1; i <= 6; i++ )
        {
            char iconName[32];
            Q_snprintf( iconName, sizeof(iconName), "Hostage%d", i );
            SetIconPosition( iconName, 0.0f, 0.0f );
        }
    }

    pMapKeyValues->deleteThis();

    PositionMapIcons();
    
    m_pMapOverviewPanel->SetVisible( true );
    m_pMapOverviewPanel->SetAlpha(0);
    vgui::GetAnimationController()->RunAnimationCommand(m_pMapOverviewPanel, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
}

void CLoadingDialog::SetIconPosition( const char *iconName, float x, float y )
{
    int index = m_MapIcons.Find( iconName );
    if ( index == m_MapIcons.InvalidIndex() )
        return;

    MapIconInfo &info = m_MapIcons[index];
    info.x = x;
    info.y = y;
    
    // Если координаты 0,0 - скрываем иконку
    if ( x == 0.0f && y == 0.0f )
    {
        info.bVisible = false;
        if ( info.pIconPanel )
        {
            info.pIconPanel->SetVisible( false );
        }
    }
    else
    {
        info.bVisible = true;
        if ( info.pIconPanel )
        {
            info.pIconPanel->SetVisible( true );
        }
    }
}

void CLoadingDialog::PositionMapIcons()
{
    if ( !m_pMapImage || !m_pMapOverviewPanel )
        return;

    int mapX, mapY, mapWide, mapTall;
    m_pMapImage->GetBounds( mapX, mapY, mapWide, mapTall );

    int iconSize = scheme()->GetProportionalScaledValue( 16 );

    m_pMapOverviewPanel->SetBounds( mapX, mapY, mapWide, mapTall );

    for ( int i = m_MapIcons.First(); i != m_MapIcons.InvalidIndex(); i = m_MapIcons.Next(i) )
    {
        MapIconInfo &info = m_MapIcons[i];
        if ( !info.pIconPanel || !info.bVisible )
            continue;

        int pixelX = (int)( info.x * mapWide ) - ( iconSize / 2 );
        int pixelY = (int)( info.y * mapTall ) - ( iconSize / 2 );

        info.pIconPanel->SetBounds( pixelX, pixelY, iconSize, iconSize );
    }
}

//-----------------------------------------------------------------------------
// Purpose: Updates the hint label with a new hint
//-----------------------------------------------------------------------------

void CLoadingDialog::FireGameEvent( IGameEvent* event )
{
	const char* eventname = event->GetName();
	if ( !eventname || !eventname[0] )
		return;

	if ( Q_strcmp( "server_shutdown", eventname ) == 0 )
	{
		ResetExtendedServerInfo();
	}
}

void CLoadingDialog::SetExtendedServerInfo( KeyValues* pExtendedServerInfo )
{
	// don't load it twice: skirmish game modes default themselves
	// to vanilla modes which results in incorrect loading screen
	// information (eg. flying scoutsman -> casual)
	if ( m_bExtendedServerInfoLoaded )
		return;

	m_bExtendedServerInfoLoaded = true;

	char const* szMapName = pExtendedServerInfo->GetString( "map", "" );
	if ( szMapName && *szMapName )
	{
		m_pMapNameLabel->SetVisible( true );
		m_pMapNameLabel->SetText( g_pGameTypes->GetMapNameID( szMapName ) );

		KeyValues* kvMapData = new KeyValues( szMapName );
		KeyValues* kvMapbackgroundData = new KeyValues( szMapName );
		KeyValues* kvMapIconData = new KeyValues( szMapName );
		char tempfile[MAX_PATH];
		Q_snprintf( tempfile, sizeof( tempfile ), "resource/overviews/%s.txt", szMapName );

		if ( kvMapData->LoadFromFile( g_pFullFileSystem, tempfile, "GAME" ) )
		{
			Q_snprintf( tempfile, sizeof( tempfile ), "../%s", kvMapData->GetString( "material" ) ); // use map overview material
			m_pMapImage->SetImage( tempfile );

			m_pMapImage->SetAlpha(0);
			vgui::GetAnimationController()->RunAnimationCommand(m_pMapImage, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}

		kvMapData->deleteThis();

		m_pGameModeNameLabel->SetText( g_pGameTypes->GetCurrentGameModeNameID() );
		m_pGameModeNameLabel->SetVisible( true );
		m_pGameModeDescriptionLabel->SetText( g_pGameTypes->GetCurrentGameModeDescID() );
		m_pGameModeDescriptionLabel->SetVisible( true );

		char tempfile2[MAX_PATH];
		Q_snprintf( tempfile2, sizeof( tempfile2 ), "resource/background/%s.txt", szMapName );

		if ( kvMapbackgroundData->LoadFromFile( g_pFullFileSystem, tempfile2, "GAME" ) )
		{
			Q_snprintf( tempfile2, sizeof( tempfile2 ), "../%s", kvMapbackgroundData->GetString( "material" ) ); // use map overview material
			m_pMapImageBackground->SetImage( tempfile2 );

			m_pMapImageBackground->SetAlpha(0);
			vgui::GetAnimationController()->RunAnimationCommand(m_pMapImageBackground, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}

		kvMapbackgroundData->deleteThis();

		char tempfile3[MAX_PATH];
		Q_snprintf( tempfile3, sizeof( tempfile3 ), "resource/icons/%s.txt", szMapName );

		if ( kvMapIconData->LoadFromFile( g_pFullFileSystem, tempfile3, "GAME" ) )
		{
			Q_snprintf( tempfile3, sizeof( tempfile3 ), "../%s", kvMapIconData->GetString( "material" ) ); // use map overview material
			m_pMapIconImage->SetImage( tempfile3 );

			m_pMapIconImage->SetAlpha(0);
			vgui::GetAnimationController()->RunAnimationCommand(m_pMapIconImage, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}

		int iGameType = g_pGameTypes->GetCurrentGameType();
	    int iGameMode = g_pGameTypes->GetCurrentGameMode();

		const char* pszCurrentGameMode = g_pGameTypes->GetGameModeFromInt( iGameType, iGameMode );
		if ( pszCurrentGameMode )
		{
			char szIconPath[64];
			V_snprintf( szIconPath, sizeof( szIconPath ), "materials/vgui/hud/svg/%s.svg", pszCurrentGameMode );
			if ( !g_pFullFileSystem->FileExists( szIconPath ) )
				m_pGameModeIcon->SetTexture( "materials/vgui/hud/svg/casual.svg" );
			else
				m_pGameModeIcon->SetTexture( szIconPath );
			
			m_pGameModeIcon->SetAlpha(0);
			vgui::GetAnimationController()->RunAnimationCommand(m_pGameModeIcon, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}
		else
		{
			m_pGameModeIcon->SetTexture( "materials/vgui/hud/svg/casual.svg" );
		}
        
        LoadMapOverviewData( szMapName );

		kvMapIconData->deleteThis();

		m_pMapNameLabel->SetAlpha(0);
		vgui::GetAnimationController()->RunAnimationCommand(m_pMapNameLabel, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);

		m_pGameModeNameLabel->SetAlpha(0);
		vgui::GetAnimationController()->RunAnimationCommand(m_pGameModeNameLabel, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);

		m_pGameModeDescriptionLabel->SetAlpha(0);
		vgui::GetAnimationController()->RunAnimationCommand(m_pGameModeDescriptionLabel, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);
	}
}

void CLoadingDialog::ResetExtendedServerInfo()
{
	m_bExtendedServerInfoLoaded = false;
	m_pMapNameLabel->SetText( "#GameUI_Loading" );
	m_pMapImage->SetImage( "map_blank" );
	m_pMapImageBackground->SetImage( "map_blank" );
	m_pMapIconImage->SetImage( "map_blank" );
	m_pGameModeNameLabel->SetVisible( false );
	m_pGameModeDescriptionLabel->SetVisible( false );
    
    m_pMapOverviewPanel->SetVisible( false );
    for ( int i = m_MapIcons.First(); i != m_MapIcons.InvalidIndex(); i = m_MapIcons.Next(i) )
    {
        MapIconInfo &info = m_MapIcons[i];
        if ( info.pIconPanel )
        {
            info.pIconPanel->SetVisible( false );
        }
    }
}

void CLoadingDialog::PaintBackground()
{
	if ( !m_bConsoleStyle )
	{
		BaseClass::PaintBackground();
		return;
	}

	// draw solid progress bar with curved endcaps
	int panelWide, panelTall;
	GetSize( panelWide, panelTall );
	int barWide, barTall;
	m_pProgress->GetSize( barWide, barTall );
	int x = ( panelWide - barWide )/2;
	int y = panelTall - barTall;

	if ( m_pLoadingBackground )
	{
		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		Color color = GetSchemeColor( "TanDarker", Color(255, 255, 255, 255), vgui::scheme()->GetIScheme(scheme) );

		m_pLoadingBackground->SetFgColor( color );
		m_pLoadingBackground->SetBgColor( color );

		m_pLoadingBackground->SetPaintBackgroundEnabled( true );
	}
	
	if ( ModInfo().IsSinglePlayerOnly() )
	{
		DrawBox( x, y, barWide, barTall, Color( 0, 0, 0, 255 ), 1.0f );
	}

	DrawBox( x+2, y+2, barWide-4, barTall-4, Color( 100, 100, 100, 255 ), 1.0f );

	barWide = m_flProgressFraction * ( barWide - 4 );
	if ( barWide >= 12 )
	{
		// cannot draw a curved box smaller than 12 without artifacts
		DrawBox( x+2, y+2, barWide, barTall-4, Color( 200, 100, 0, 255 ), 1.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets up dialog layout
//-----------------------------------------------------------------------------
void CLoadingDialog::SetupControlSettings()
{
	if ( GameUI().IsConsoleUI() )
	{
		KeyValues *pControlSettings = BasePanel()->GetConsoleControlSettings()->FindKey( "LoadingDialogNoBanner.res" );
		LoadControlSettings( "null", NULL, pControlSettings );
		return;
	}

	LoadControlSettings("Resource/LoadingDialogNoBanner.res");
}

//-----------------------------------------------------------------------------
// Purpose: Activates the loading screen, initializing and making it visible
//-----------------------------------------------------------------------------
void CLoadingDialog::Open()
{
	if ( !m_bConsoleStyle )
	{
		SetTitle( "#GameUI_Loading", true );
	}


	HideOtherDialogs( true );
	BaseClass::Activate();

	SetAlpha(0);
	vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 255.0f, 0.0f, 0.4f, vgui::AnimationController::INTERPOLATOR_LINEAR);

	if ( !m_bConsoleStyle )
	{
		m_pProgress->SetVisible( true );
		if ( !ModInfo().IsSinglePlayerOnly() )
		{
			m_pInfoLabel->SetVisible( true );
		}
		m_pInfoLabel->SetText("");
		
		m_pCancelButton->SetText("#GameUI_Cancel");
		m_pCancelButton->SetCommand("Cancel");
	}
	if (m_pTipPanel)
        {
            m_pTipPanel->SetVisible(true);
        }
}


//-----------------------------------------------------------------------------
// Purpose: error display file
//-----------------------------------------------------------------------------
void CLoadingDialog::SetupControlSettingsForErrorDisplay( const char *settingsFile )
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	m_bCenter = true;
	SetTitle("#GameUI_Disconnected", true);
	m_pInfoLabel->SetText("");
	LoadControlSettings( settingsFile );
	HideOtherDialogs( true );

	BaseClass::Activate();
	
	m_pProgress->SetVisible(false);
	m_pMapNameLabel->SetVisible(false);
	m_pMapImage->SetVisible(false);
	m_pMapImageBackground->SetVisible(false);
	m_pMapIconImage->SetVisible(false);
	m_pGameModeNameLabel->SetVisible(false);
	m_pGameModeDescriptionLabel->SetVisible(false);

	m_pInfoLabel->SetVisible(true);
	m_pCancelButton->SetText("#GameUI_Close");
	m_pCancelButton->SetCommand("Close");
	m_pInfoLabel->InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: shows or hides other top-level dialogs
//-----------------------------------------------------------------------------
void CLoadingDialog::HideOtherDialogs( bool bHide )
{
	if ( bHide )
	{
		if ( GameUI().HasLoadingBackgroundDialog() )
		{
			// if we have a loading background dialog, hide any other dialogs by moving the full-screen background dialog to the
			// front, then moving ourselves in front of it
			GameUI().ShowLoadingBackgroundDialog();
			vgui::ipanel()->MoveToFront( GetVPanel() );
			vgui::input()->SetAppModalSurface( GetVPanel() );
		}
		else
		{
			// if there is no loading background dialog, use VGUI paint restrictions to hide other dialogs
			vgui::surface()->RestrictPaintToSinglePanel(GetVPanel());
		}
	}
	else
	{
		if ( GameUI().HasLoadingBackgroundDialog() )
		{
			GameUI().HideLoadingBackgroundDialog();
			vgui::input()->SetAppModalSurface( NULL );
		}
		else
		{
			// remove any rendering restrictions
			vgui::surface()->RestrictPaintToSinglePanel(NULL);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turns dialog into error display
//-----------------------------------------------------------------------------
void CLoadingDialog::DisplayGenericError(const char *failureReason, const char *extendedReason)
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	// In certain race conditions, DisplayGenericError can get called AFTER OnClose() has been called.
	// If that happens and we don't call Activate(), then it'll continue closing when we don't want it to.
	Activate(); 
	
	SetupControlSettingsForErrorDisplay("Resource/LoadingDialogError.res");

	if ( extendedReason && strlen( extendedReason ) > 0 ) 
	{
		wchar_t compositeReason[256], finalMsg[512], formatStr[256];
		if ( extendedReason[0] == '#' )
		{
			wcsncpy(compositeReason, g_pVGuiLocalize->Find(extendedReason), sizeof( compositeReason ) / sizeof( wchar_t ) );
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(extendedReason, compositeReason, sizeof( compositeReason ));
		}

		if ( failureReason[0] == '#' )
		{
			wcsncpy(formatStr, g_pVGuiLocalize->Find(failureReason), sizeof( formatStr ) / sizeof( wchar_t ) );
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(failureReason, formatStr, sizeof( formatStr ));
		}

		g_pVGuiLocalize->ConstructString(finalMsg, sizeof( finalMsg ), formatStr, 1, compositeReason);
		m_pInfoLabel->SetText(finalMsg);
	}
	else
	{
		m_pInfoLabel->SetText(failureReason);
	}

	int wide, tall;
	int x,y;
	m_pInfoLabel->GetContentSize( wide, tall );
	m_pInfoLabel->GetPos( x, y );
	SetTall( tall + y + 50 );

	int buttonX, buttonY;
	m_pCancelButton->GetPos( buttonX, buttonY );
	m_pCancelButton->SetPos( buttonX, tall + y + 6 );

	m_pCancelButton->RequestFocus();
}


//-----------------------------------------------------------------------------
// Purpose: explain to the user they can't join secure servers due to a VAC ban
//-----------------------------------------------------------------------------
void CLoadingDialog::DisplayVACBannedError()
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	SetupControlSettingsForErrorDisplay("Resource/LoadingDialogErrorVACBanned.res");
	SetTitle("#VAC_ConnectionRefusedTitle", true);
}


//-----------------------------------------------------------------------------
// Purpose: explain to the user they can't connect to public servers due to 
//			not having a valid connection to Steam
//			this should only happen if they are a pirate
//-----------------------------------------------------------------------------
void CLoadingDialog::DisplayNoSteamConnectionError()
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	SetupControlSettingsForErrorDisplay("Resource/LoadingDialogErrorNoSteamConnection.res");
}


//-----------------------------------------------------------------------------
// Purpose: explain to the user they got kicked from a server due to that same account 
//			logging in from another location. This also triggers the refresh login dialog on OK 
//			being pressed.
//-----------------------------------------------------------------------------
void CLoadingDialog::DisplayLoggedInElsewhereError()
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	SetupControlSettingsForErrorDisplay("Resource/LoadingDialogErrorLoggedInElsewhere.res");
	m_pCancelButton->SetText("#GameUI_RefreshLogin_Login");
	m_pCancelButton->SetCommand("Login");
}


//-----------------------------------------------------------------------------
// Purpose: sets status info text
//-----------------------------------------------------------------------------
void CLoadingDialog::SetStatusText(const char *statusText)
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	m_pInfoLabel->SetText(statusText);
}

//-----------------------------------------------------------------------------
// Purpose: returns the previous state
//-----------------------------------------------------------------------------
bool CLoadingDialog::SetShowProgressText( bool show )
{
	if ( m_bConsoleStyle )
	{
		return false;
	}

	bool bret = m_pInfoLabel->IsVisible();
	if ( bret != show )
	{
		SetupControlSettings();
		m_pInfoLabel->SetVisible( show );
	}
	return bret;
}

//-----------------------------------------------------------------------------
// Purpose: updates time remaining
//-----------------------------------------------------------------------------
void CLoadingDialog::OnThink()
{
	BaseClass::OnThink();

	if ( !m_bConsoleStyle && m_bShowingSecondaryProgress )
	{
		// calculate the time remaining string
		wchar_t unicode[512];
		if (m_flSecondaryProgress >= 1.0f)
		{
			m_pTimeRemainingLabel->SetText("complete");
		}
		else if (ContinuousProgressBar::ConstructTimeRemainingString(unicode, sizeof(unicode), m_flSecondaryProgressStartTime, (float)system()->GetFrameTime(), m_flSecondaryProgress, m_flLastSecondaryProgressUpdateTime, true))
		{
			m_pTimeRemainingLabel->SetText(unicode);
		}
		else
		{
			m_pTimeRemainingLabel->SetText("");
		}
	}
	
	if (!m_bConsoleStyle && m_pTipPanel)
	{
	    m_pTipPanel->NextTip();
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLoadingDialog::PerformLayout()
{
	if ( m_bConsoleStyle )
	{
		// place in lower center
		int screenWide, screenTall;
		surface()->GetScreenSize( screenWide, screenTall );
		int wide,tall;
		GetSize( wide, tall );
		int x = 0;
		int y = 0;

		if ( ModInfo().IsSinglePlayerOnly() )
		{
			x = ( screenWide - wide ) * 0.50f;
			y = ( screenTall - tall ) * 0.86f;
		}
		else
		{
			x = ( screenWide - ( wide * 1.30f ) );
			y = ( ( screenTall * 0.875f ) );
		}

		SetPos( x, y );
	}
	else if ( m_bCenter )
	{
		MoveToCenterOfScreen();
	}
	else
	{
		// if we're not supposed to be centered, move ourselves to the lower right hand corner of the screen
		int x, y, screenWide, screenTall;
		surface()->GetWorkspaceBounds( x, y, screenWide, screenTall );
		int wide,tall;
		GetSize( wide, tall );

		if ( IsPC() )
		{
			x = screenWide - ( wide + 10 );
			y = screenTall - ( tall + 10 );
		}
		else
		{
			// Move farther in so we're title safe
			x = screenWide - wide - (screenWide * 0.05);
			y = screenTall - tall - (screenTall * 0.05);
		}

		x -= m_iAdditionalIndentX;
		y -= m_iAdditionalIndentY;

		SetPos( x, y );
	}
	
	if (!m_bConsoleStyle)
    {
        vgui::HScheme scheme = vgui::scheme()->GetScheme("ClientScheme");
        vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(scheme);
    }
    
    if ( m_pMapOverviewPanel && m_pMapOverviewPanel->IsVisible() )
    {
        PositionMapIcons();
    }
	
	BaseClass::PerformLayout();
	
	vgui::ipanel()->MoveToFront( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the number of ticks has changed
//-----------------------------------------------------------------------------
bool CLoadingDialog::SetProgressPoint( float fraction )
{
	if ( m_bConsoleStyle )
	{
		if ( fraction >= 0.99f )
		{
			// show the progress artifically completed to fill in 100%
			fraction = 1.0f;
		}
		fraction = clamp( fraction, 0.0f, 1.0f );
		if ( (int)(fraction * 25) != (int)(m_flProgressFraction * 25) )
		{
			m_flProgressFraction = fraction;
			return true;
		}
		return IsX360();
	}

	int nOldDrawnSegments = m_pProgress->GetDrawnSegmentCount();
	m_pProgress->SetProgress( fraction );
	int nNewDrawSegments = m_pProgress->GetDrawnSegmentCount();
	return (nOldDrawnSegments != nNewDrawSegments) || IsX360();
}

//-----------------------------------------------------------------------------
// Purpose: sets and shows the secondary progress bar
//-----------------------------------------------------------------------------
void CLoadingDialog::SetSecondaryProgress( float progress )
{
	if ( m_bConsoleStyle )
		return;

	// don't show the progress if we've jumped right to completion
	if (!m_bShowingSecondaryProgress && progress > 0.99f)
		return;

	// if we haven't yet shown secondary progress then reconfigure the dialog
	if (!m_bShowingSecondaryProgress)
	{
		m_bShowingSecondaryProgress = true;
		m_pProgress2->SetVisible(true);
		m_flSecondaryProgressStartTime = (float)system()->GetFrameTime();
	}

	// if progress has increased then update the progress counters
	if (progress > m_flSecondaryProgress)
	{
		m_pProgress2->SetProgress(progress);
		m_flSecondaryProgress = progress;
		m_flLastSecondaryProgressUpdateTime = (float)system()->GetFrameTime();
	}

	// if progress has decreased then reset progress counters
	if (progress < m_flSecondaryProgress)
	{
		m_pProgress2->SetProgress(progress);
		m_flSecondaryProgress = progress;
		m_flLastSecondaryProgressUpdateTime = (float)system()->GetFrameTime();
		m_flSecondaryProgressStartTime = (float)system()->GetFrameTime();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLoadingDialog::SetSecondaryProgressText(const char *statusText)
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	SetControlString( "SecondaryProgressLabel", statusText );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLoadingDialog::OnClose()
{
	// remove any rendering restrictions
	HideOtherDialogs( false );

	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: command handler
//-----------------------------------------------------------------------------
void CLoadingDialog::OnCommand(const char *command)
{
	if ( !stricmp(command, "Cancel") )
	{
		// disconnect from the server
		engine->ClientCmd_Unrestricted("disconnect\n");

		ResetExtendedServerInfo();

		// close
		Close();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CLoadingDialog::OnKeyCodeTyped(KeyCode code)
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	if ( code == KEY_ESCAPE )
	{
		OnCommand("Cancel");
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Maps ESC to quiting loading
//-----------------------------------------------------------------------------
void CLoadingDialog::OnKeyCodePressed(KeyCode code)
{
	if ( m_bConsoleStyle )
	{
		return;
	}

	ButtonCode_t nButtonCode = GetBaseButtonCode( code );

	if ( nButtonCode == KEY_XBUTTON_B || nButtonCode == KEY_XBUTTON_A )
	{
		OnCommand("Cancel");
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

ConVar ui_loading_tip_refresh( "ui_loading_tip_refresh", "5", FCVAR_DEVELOPMENTONLY );
ConVar ui_loading_tip_f1( "ui_loading_tip_f1", "0.05", FCVAR_DEVELOPMENTONLY );
ConVar ui_loading_tip_f2( "ui_loading_tip_f2", "0.40", FCVAR_DEVELOPMENTONLY );

//--------------------------------------------------------------------------------------------------------
CLoadingTipPanel::CLoadingTipPanel( Panel *pParent ) : EditablePanel( pParent, "loadingtippanel" )
{
	m_flLastTipTime = 0.f;
	m_iCurrentTip = 0;
	m_pTipIcon = NULL;

	m_smearColor = Color( 0, 0, 0, 255 );

	SetupTips();
}

//--------------------------------------------------------------------------------------------------------
CLoadingTipPanel::~CLoadingTipPanel()
{
}

//--------------------------------------------------------------------------------------------------------
void CLoadingTipPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_smearColor = pScheme->GetColor( "Frame.SmearColor", Color( 0, 0, 0, 225 ) );

	ReloadScheme();
}

//--------------------------------------------------------------------------------------------------------
void CLoadingTipPanel::ReloadScheme( void )
{
	LoadControlSettings( "Resource/UI/loadingtippanel.res" );

	m_pTipIcon = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "TipIcon" ) );

	NextTip();
}

//--------------------------------------------------------------------------------------------------------
void CLoadingTipPanel::SetupTips( void )
{
	KeyValues *pKV = new KeyValues( "Tips" );
	KeyValues::AutoDelete autodelete( pKV );
	if ( !pKV->LoadFromFile( g_pFullFileSystem, "scripts/tips.txt", "GAME" ) )
	{
		AssertMsg( false, "failed to load tips!" );
		return;
	}

	for ( KeyValues *pKey = pKV->FindKey( "SurvivorTips" )->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
	{
		sTipInfo info;
		V_strncpy( info.szTipTitle, "", MAX_TIP_LENGTH );
		V_strncpy( info.szTipString, pKey->GetName(), MAX_TIP_LENGTH );
		V_strncpy( info.szTipImage, "achievements/ACH_SURVIVE_BRIDGE", MAX_TIP_LENGTH );
		m_Tips.AddToTail( info );
	}
#ifdef ACHIEVEMENT
	TitleAchievementsDescription_t const *desc = g_pMatchFramework->GetMatchTitle()->DescribeTitleAchievements();
	for ( ; desc->m_szAchievementName; ++desc )
	{
		sTipInfo info;
		V_snprintf( info.szTipTitle, MAX_TIP_LENGTH, "#%s_NAME", desc->m_szAchievementName );
		V_snprintf( info.szTipString, MAX_TIP_LENGTH, "#%s_DESC", desc->m_szAchievementName );
		V_snprintf( info.szTipImage, MAX_TIP_LENGTH, "achievements/%s", desc->m_szAchievementName );
		m_Tips.AddToTail( info );
	}
#endif
}

//--------------------------------------------------------------------------------------------------------
void CLoadingTipPanel::NextTip( void )
{
	if ( !IsEnabled() )
		return;

	if ( !m_Tips.Count() )
		return;

	if ( !m_flLastTipTime )
	{
		// Initialize timer on first render
		m_flLastTipTime = Plat_FloatTime();
		return;
	}

	if ( Plat_FloatTime() - m_flLastTipTime < ui_loading_tip_refresh.GetFloat() )
		return;

	m_flLastTipTime = Plat_FloatTime();

	m_iCurrentTip = RandomInt( 0, m_Tips.Count() - 1 );
	if ( !m_Tips.IsValidIndex( m_iCurrentTip ) )
		return;

	sTipInfo info = m_Tips[m_iCurrentTip];

	if ( m_pTipIcon )
	{
		m_pTipIcon->SetImage( info.szTipImage );
	}
	SetControlString( "TipTitle", info.szTipTitle );
	SetControlString( "TipText", info.szTipString );

	// Set our control visible
	SetVisible( true );
}


#define TOP_BORDER_HEIGHT		21
#define BOTTOM_BORDER_HEIGHT	21
int CLoadingTipPanel::DrawSmearBackgroundFade( int x0, int y0, int x1, int y1 )
{
	int wide = x1 - x0;
	int tall = y1 - y0;

	int topTall = scheme()->GetProportionalScaledValue( TOP_BORDER_HEIGHT );
	int bottomTall = scheme()->GetProportionalScaledValue( BOTTOM_BORDER_HEIGHT );

	float f1 = ui_loading_tip_f1.GetFloat();
	float f2 = ui_loading_tip_f2.GetFloat();

	topTall  = 1.00f * topTall;
	bottomTall = 1.00f * bottomTall;

	int middleTall = tall - ( topTall + bottomTall );
	if ( middleTall < 0 )
	{
		middleTall = 0;
	}

	surface()->DrawSetColor( m_smearColor );

	y0 += topTall;

	if ( middleTall )
	{
		// middle
		surface()->DrawFilledRectFade( x0, y0, x0 + f1*wide, y0 + middleTall, 0, 255, true );
		surface()->DrawFilledRectFade( x0 + f1*wide, y0, x0 + f2*wide, y0 + middleTall, 255, 255, true );
		surface()->DrawFilledRectFade( x0 + f2*wide, y0, x0 + wide, y0 + middleTall, 255, 0, true );
		y0 += middleTall;
	}

	return topTall + middleTall + bottomTall;
}

//--------------------------------------------------------------------------------------------------------
void CLoadingTipPanel::PaintBackground( void )
{
	BaseClass::PaintBackground();

	DrawSmearBackgroundFade( 
		0, 
		-scheme()->GetProportionalScaledValue( 20 ), 
		GetWide(), 
		GetTall() ); 

}

/*void PrecacheLoadingTipIcons()
{
	TitleAchievementsDescription_t const *desc = g_pMatchFramework->GetMatchTitle()->DescribeTitleAchievements();
	for ( ; desc->m_szAchievementName; ++desc )
	{
		CFmtStr imageString( "vgui/achievements/%s", desc->m_szAchievementName );
		int nImageId = vgui::surface()->DrawGetTextureId( imageString );
		if ( nImageId == -1 )
		{
			nImageId = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( nImageId, imageString, true, false );	
		}
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: Singleton accessor
//-----------------------------------------------------------------------------
extern vgui::DHANDLE<CLoadingDialog> g_hLoadingDialog;
CLoadingDialog *LoadingDialog()
{
	return g_hLoadingDialog.Get();
}
