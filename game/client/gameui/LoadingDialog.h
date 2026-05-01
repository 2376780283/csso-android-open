//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/VectorImagePanel.h>
#include "GameEventListener.h"

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying level loading status
//-----------------------------------------------------------------------------
#define MAX_TIP_LENGTH 64

struct sTipInfo
{
	char szTipTitle[MAX_TIP_LENGTH];
	char szTipString[MAX_TIP_LENGTH];
	char szTipImage[MAX_TIP_LENGTH];
};

struct MapIconInfo
{
    vgui::ImagePanel *pIconPanel;
    float x;
    float y;
    bool bVisible;
    
    MapIconInfo()
    {
        pIconPanel = NULL;
        x = 0.0f;
        y = 0.0f;
        bVisible = false;
    }
};


enum eTipMode
{
	TIP_MODE_SURVIVOR,
	TIP_MODE_INFECTED,
	TIP_MODE_ACHIEVEMENTS,

	TIP_MODE_COUNT,
};

class CLoadingTipPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CLoadingTipPanel, vgui::EditablePanel )

public:
	CLoadingTipPanel( Panel *pParent );
	~CLoadingTipPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PaintBackground( void );
	void ReloadScheme( void );

	void NextTip( void );
	void SetupTips( void );

private:
	int	 DrawSmearBackgroundFade( int x, int y, int wide, int tall );

	Color m_smearColor;

	vgui::ImagePanel *m_pTipIcon;

	CUtlVector< sTipInfo > m_Tips;

	float m_flLastTipTime;
	int m_iCurrentTip;
};
class CLoadingDialog : public vgui::Frame, public CGameEventListener
{
        DECLARE_CLASS_SIMPLE( CLoadingDialog, vgui::Frame ); 
public:
        CLoadingDialog( vgui::Panel *parent );
        ~CLoadingDialog();

        void Open();
        bool SetProgressPoint(float fraction);
        void SetStatusText(const char *statusText);
        void SetSecondaryProgress(float progress);
        void SetSecondaryProgressText(const char *statusText);
        bool SetShowProgressText( bool show );
        void UpdateHintLabel();

        void DisplayGenericError(const char *failureReason, const char *extendedReason = NULL);
        void DisplayVACBannedError();
        void DisplayNoSteamConnectionError();
        void DisplayLoggedInElsewhereError();

        // IGameEventListener
        virtual void FireGameEvent( IGameEvent* event );

        void SetExtendedServerInfo( KeyValues* pExtendedServerInfo );
        void ResetExtendedServerInfo();

protected:
        virtual void OnCommand(const char *command);
        virtual void PerformLayout();
        virtual void OnThink();
        virtual void OnClose();
        virtual void OnKeyCodeTyped(vgui::KeyCode code);
        virtual void OnKeyCodePressed(vgui::KeyCode code);
        virtual void PaintBackground( void );
        void SetupMapIcons();
        void LoadMapOverviewData( const char *mapName );
        void SetIconPosition( const char *iconName, float x, float y );
        void UpdateMapIconsVisibility( int iGameType, int iGameMode );
        void PositionMapIcons();
        void ClearMapIcons();

private:
        void SetupControlSettings();
        void SetupControlSettingsForErrorDisplay( const char *settingsFile );
        void HideOtherDialogs( bool bHide );
        
        CUtlDict< MapIconInfo, int > m_MapIcons;
        vgui::Panel              *m_pMapOverviewPanel;

        CLoadingTipPanel *m_pTipPanel;
        vgui::ProgressBar        *m_pProgress;
        vgui::ProgressBar        *m_pProgress2;
        vgui::Label                        *m_pInfoLabel;
        vgui::Label                        *m_pTimeRemainingLabel;
        vgui::Button                *m_pCancelButton;
        vgui::Panel                        *m_pLoadingBackground;
        vgui::Label                        *m_pMapNameLabel;
        vgui::VectorImagePanel* m_pGameModeIcon;
        vgui::ImagePanel        *m_pMapImage;
        vgui::ImagePanel        *m_pMapIconImage;
        vgui::ImagePanel        *m_pMapImageBackground;
        vgui::Label                        *m_pGameModeNameLabel;
        vgui::Label                        *m_pGameModeDescriptionLabel;

        bool        m_bShowingSecondaryProgress;
        float        m_flSecondaryProgress;
        float        m_flLastSecondaryProgressUpdateTime;
        float        m_flSecondaryProgressStartTime;
        bool        m_bCenter;
        bool        m_bConsoleStyle;
        float        m_flProgressFraction;
        bool        m_bExtendedServerInfoLoaded;

        CPanelAnimationVar( int, m_iAdditionalIndentX, "AdditionalIndentX", "0" );
        CPanelAnimationVar( int, m_iAdditionalIndentY, "AdditionalIndentY", "0" );
};

// singleton accessor
CLoadingDialog *LoadingDialog();


#endif // LOADINGDIALOG_H