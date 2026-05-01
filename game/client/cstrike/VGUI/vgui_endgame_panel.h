#ifndef VGUI_ENDGAME_PANEL_H
#define VGUI_ENDGAME_PANEL_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include "PlayerModelPanel.h"
#include "GameEventListener.h"

class CGameResultPanel : public vgui::Frame, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CGameResultPanel, vgui::Frame);

public:
    CGameResultPanel(vgui::VPANEL parent);
    virtual ~CGameResultPanel();

    void ShowResult(bool bWin);
    void Reset(); // Сброс для нового матча
    
protected:
    virtual void OnCommand(const char *command);
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void OnThink(); 
    void UpdateWinLose();
    virtual void FireGameEvent( IGameEvent *event );
    virtual void PaintBackground();
private:
    CBasePlayerModelPanel* m_pPlayerPanels[5];
    vgui::Label* m_pPlayerLabels[5];
    
    void SetupPlayerPanels();
    void UpdatePlayerModels();
    C_CSPlayer* GetPlayerByOffset(int offset);
    vgui::Label *m_pResultLabel;
    vgui::Button *m_pContinueButton;
    bool m_bShownResult; 
    bool m_gameOver;
};

#endif // VGUI_ENDGAME_PANEL_H
