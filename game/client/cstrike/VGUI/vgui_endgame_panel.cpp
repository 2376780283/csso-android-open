#include "cbase.h"
#include "vgui_endgame_panel.h"
#include "c_cs_playerresource.h"
#include "cs_gamerules.h"
#include <vgui/IVGui.h>
#include <ienginevgui.h>
#include "c_team.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include "GameEventListener.h"
#include "viewpostprocess.h"
#include "c_cs_player.h"
#include "cs_loadout.h"
#include "weapon_csbase.h"

using namespace vgui;

CGameResultPanel *g_pGameResultPanel = NULL;

static CGameResultPanel* GetEndgamePanel()
{
    static CGameResultPanel *s_pPanel = NULL;
    
    if (!s_pPanel)
    {
        s_pPanel = new CGameResultPanel(enginevgui->GetPanel(PANEL_CLIENTDLL));
        
        if (s_pPanel)
        {
            vgui::ivgui()->AddTickSignal(s_pPanel->GetVPanel(), 100);
            Msg("[ENDGAME] Panel created successfully!\n");
        }
        else
        {
            Warning("[ENDGAME] Failed to create panel!\n");
        }
    }
    
    return s_pPanel;
}

CGameResultPanel::CGameResultPanel(vgui::VPANEL parent) : BaseClass(NULL, "GameResultPanel")
{
    SetParent(parent);
    int wide, tall;
    surface()->GetScreenSize(wide, tall);
    SetSize(wide, tall);
    SetTitle("", true);
    SetMoveable(false);
    SetSizeable(false);
    SetCloseButtonVisible(false);
    
    m_pResultLabel = new Label(this, "ResultLabel", "");
    m_pContinueButton = new Button(this, "ContinueButton", "Continue", this, "continue");
    
    // Инициализация панелей игроков
    for (int i = 0; i < 5; i++)
    {
        m_pPlayerPanels[i] = NULL;
        m_pPlayerLabels[i] = NULL;
    }
    
    SetupPlayerPanels();
    
    SetVisible(false);
    SetScheme("ClientScheme");
    LoadControlSettings("resource/UI/GameResultPanel.res");
    
    ListenForGameEvent( "announce_phase_end" );
    
    g_pGameResultPanel = this;
    m_gameOver = false;
    
    Msg("[ENDGAME] Panel created\n");
}

CGameResultPanel::~CGameResultPanel()
{
    // Удаление панелей игроков
    for (int i = 0; i < 5; i++)
    {
        if (m_pPlayerPanels[i])
        {
            m_pPlayerPanels[i]->MarkForDeletion();
            m_pPlayerPanels[i] = NULL;
        }
        if (m_pPlayerLabels[i])
        {
            m_pPlayerLabels[i]->MarkForDeletion();
            m_pPlayerLabels[i] = NULL;
        }
    }
    
    g_pGameResultPanel = NULL;
    m_gameOver = false;
}

void CGameResultPanel::SetupPlayerPanels()
{
    for (int i = 0; i < 5; i++)
    {
        m_pPlayerPanels[i] = new CBasePlayerModelPanel(this, VarArgs("PlayerPanel%d", i));
        m_pPlayerPanels[i]->SetVisible(false);
        
        m_pPlayerLabels[i] = new Label(this, VarArgs("PlayerLabel%d", i), "");
        m_pPlayerLabels[i]->SetContentAlignment(Label::a_center);
        m_pPlayerLabels[i]->SetVisible(false);
    }
}

C_CSPlayer* CGameResultPanel::GetPlayerByOffset(int offset)
{
    C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocalPlayer)
        return NULL;
    
    int localTeam = pLocalPlayer->GetTeamNumber();
    int localIndex = pLocalPlayer->entindex();
    
    // Собираем всех игроков команды
    CUtlVector<C_CSPlayer*> teamPlayers;
    
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        C_CSPlayer* pPlayer = ToCSPlayer(UTIL_PlayerByIndex(i));
        if (pPlayer && pPlayer->GetTeamNumber() == localTeam)
        {
            teamPlayers.AddToTail(pPlayer);
        }
    }
    
    // Сортируем так, чтобы локальный был в центре
    int localPosInArray = -1;
    for (int i = 0; i < teamPlayers.Count(); i++)
    {
        if (teamPlayers[i]->entindex() == localIndex)
        {
            localPosInArray = i;
            break;
        }
    }
    
    if (localPosInArray == -1)
        return NULL;
    
    int targetIndex = localPosInArray + offset;
    
    if (targetIndex >= 0 && targetIndex < teamPlayers.Count())
        return teamPlayers[targetIndex];
    
    return NULL;
}

void CGameResultPanel::UpdatePlayerModels()
{
    C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocalPlayer)
        return;
    
    // Центральный игрок - локальный (индекс 2)
    // Слева: -2, -1
    // Справа: +1, +2
    int offsets[5] = {-2, -1, 0, 1, 2};
    
    for (int i = 0; i < 5; i++)
    {
        C_CSPlayer* pPlayer = GetPlayerByOffset(offsets[i]);
        
        if (pPlayer && m_pPlayerPanels[i])
        {
            // Получаем модель агента
            const char* pszPlayerModel = NULL;
            bool bUseAgent = CSLoadout()->HasAgentSet( pPlayer, pPlayer->GetTeamNumber() );
            
            if (bUseAgent)
            {
                pszPlayerModel = GetCSAgentInfoT( CSLoadout()->GetAgentForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->m_szModel;
            }
            
            if (pszPlayerModel && pszPlayerModel[0])
            {
                m_pPlayerPanels[i]->SetMDL(pszPlayerModel);
            }
            
            // Устанавливаем модель перчаток
            if ( CSLoadout()->HasGlovesSet( pPlayer, pPlayer->GetTeamNumber() ) )
            {
                const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szViewModel;
                const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szWorldModel;
                const char* pszDefaultGlovesModel = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel )->szAssociatedGloveModel;
                
                if ( pszGlovesViewModel && pszDefaultGlovesModel && pszGlovesViewModel[0] && pszDefaultGlovesModel[0] )
                {
                    // Проверяем поддержку перчаток (если у вас есть такая функция)
                    // Иначе просто устанавливаем модель
                    if (pszGlovesWorldModel && pszGlovesWorldModel[0])
                    {
                        m_pPlayerPanels[i]->SetMergeMDL(pszGlovesWorldModel);
                    }
                }
            }
            
            // Получаем последнее оружие
            C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
            if (!pWeapon)
            {
                // Пытаемся найти последнее использованное оружие
                for (int w = 0; w < MAX_WEAPONS; w++)
                {
                    C_BaseCombatWeapon* pWep = pPlayer->GetWeapon(w);
                    if (pWep)
                    {
                        pWeapon = pWep;
                        break;
                    }
                }
            }
            
            if (pWeapon)
            {
                const char* pszWeaponName = pWeapon->GetClassname();
                if (pszWeaponName)
                {
                    WEAPON_FILE_INFO_HANDLE hWpnInfo = LookupWeaponInfoSlot( pszWeaponName );
                    if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
                    {
                        CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
                        if ( pWeaponInfo && pWeaponInfo->szWorldModel && pWeaponInfo->szWorldModel[0] )
                        {
                            CMDL* pWeaponMDL = m_pPlayerPanels[i]->SetMergeMDL(pWeaponInfo->szWorldModel);
                            
                            // Можно установить скин оружия если нужно
                            if (pWeaponMDL)
                            {
                                C_WeaponCSBase* pCSWeapon = dynamic_cast<C_WeaponCSBase*>(pWeapon);
                                if (pCSWeapon)
                                {
                                    // pWeaponMDL->m_nSkin = pCSWeapon->GetSkin();
                                }
                            }
                        }
                    }
                }
            }
            
            // Устанавливаем имя игрока
            if (m_pPlayerLabels[i])
            {
                player_info_t playerInfo;
                if (engine->GetPlayerInfo(pPlayer->entindex(), &playerInfo))
                {
                    m_pPlayerLabels[i]->SetText(playerInfo.name);
                }
                m_pPlayerLabels[i]->SetVisible(true);
            }
            
            // Делаем панель видимой
            m_pPlayerPanels[i]->SetVisible(true);
        }
        else if (m_pPlayerPanels[i])
        {
            // Скрываем пустые панели
            m_pPlayerPanels[i]->SetVisible(false);
            if (m_pPlayerLabels[i])
                m_pPlayerLabels[i]->SetVisible(false);
        }
    }
}

void CGameResultPanel::ShowResult(bool bWin)
{
    Msg("[ENDGAME] ShowResult called: %s\n", bWin ? "WIN" : "LOSE");
    
    if (bWin)
    {
        m_pResultLabel->SetText("#Game_Win");
        SetBgColor(Color(0, 128, 0, 220));
    }
    else
    {
        m_pResultLabel->SetText("#Game_Lose");
        SetBgColor(Color(128, 0, 0, 220));
    }
    
    // Обновляем модели игроков
    UpdatePlayerModels();
    
    SetVisible(true);
    MoveToFront();
    RequestFocus();
    
    int wide, tall;
    surface()->GetScreenSize(wide, tall);
    SetPos((wide - GetWide()) / 2, (tall - GetTall()) / 2);
    
    m_bShownResult = true;
    m_gameOver = true;
    
    Msg("[ENDGAME] Panel should be visible now\n");
}

void CGameResultPanel::OnCommand(const char *command)
{
    if (!Q_strcmp(command, "continue"))
    {
        SetVisible(false);
        
        // Скрываем панели игроков
        for (int i = 0; i < 5; i++)
        {
            if (m_pPlayerPanels[i])
                m_pPlayerPanels[i]->SetVisible(false);
            if (m_pPlayerLabels[i])
                m_pPlayerLabels[i]->SetVisible(false);
        }
        
        engine->ClientCmd("disconnect");
    }
    
    BaseClass::OnCommand(command);
}

void CGameResultPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    
    if (m_pResultLabel)
    {
        m_pResultLabel->SetFont(pScheme->GetFont("HudNumbers", true));
        m_pResultLabel->SetFgColor(Color(255, 255, 255, 255));
    }
    
    // Применяем шрифт для имен игроков
    for (int i = 0; i < 5; i++)
    {
        if (m_pPlayerLabels[i])
        {
            m_pPlayerLabels[i]->SetFont(pScheme->GetFont("Default", true));
            m_pPlayerLabels[i]->SetFgColor(Color(255, 255, 255, 255));
        }
    }
}

void CGameResultPanel::OnThink()
{
    BaseClass::OnThink();
}

void CGameResultPanel::UpdateWinLose()
{
    BaseClass::OnThink();
    
    if (m_bShownResult)
        return;
    
    if (!CSGameRules())
        return;
    
    if (CSGameRules()->GetGamePhase() == GAMEPHASE_MATCH_ENDED)
    {
        C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
        if (!pLocalPlayer)
            return;
        
        bool bLocalPlayerWon = false;
        
        int iLocalTeam = pLocalPlayer->GetTeamNumber();
        
        C_Team *pLocalTeam = GetGlobalTeam(iLocalTeam);
        if (pLocalTeam)
        {
            int iLocalTeamScore = pLocalTeam->Get_Score();
            
            int iEnemyTeam = (iLocalTeam == TEAM_CT) ? TEAM_TERRORIST : TEAM_CT;
            C_Team *pEnemyTeam = GetGlobalTeam(iEnemyTeam);
            
            if (pEnemyTeam)
            {
                int iEnemyScore = pEnemyTeam->Get_Score();
                bLocalPlayerWon = (iLocalTeamScore > iEnemyScore);
            }
        }
        
        ShowResult(bLocalPlayerWon);
    }
}

void CGameResultPanel::FireGameEvent( IGameEvent *event )
{
    if ( event == NULL )
        return;

    const char *pEventName = event->GetName();
    if ( pEventName == NULL )
        return;
        
    if ( Q_strcmp( pEventName, "game_newmap" ) == 0 )
    {
        m_gameOver = false;
    }
    else if ( Q_strcmp( pEventName, "cs_win_panel_match" ) == 0 )
    {
        m_gameOver = true;
        if ( m_gameOver && Q_strcmp( pEventName, "announce_phase_end" ) == 0 )
        {
            UpdateWinLose();
            ShowResult(true);
        }
    }
    else
    {
        m_gameOver = false;
        ShowResult(false);
    }
}

extern ConVar mat_blur_strength;
extern ConVar mat_blur_desaturate;
void CGameResultPanel::PaintBackground()
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

void CGameResultPanel::Reset()
{
    m_gameOver = false;
    m_bShownResult = false;
    SetVisible(false);
    
    for (int i = 0; i < 5; i++)
    {
        if (m_pPlayerPanels[i])
            m_pPlayerPanels[i]->SetVisible(false);
        if (m_pPlayerLabels[i])
            m_pPlayerLabels[i]->SetVisible(false);
    }
}

CON_COMMAND(show_endgame_win, "Test endgame panel - WIN")
{
    CGameResultPanel *pPanel = GetEndgamePanel();
    if (g_pGameResultPanel)
    {
        g_pGameResultPanel->ShowResult(true);
    }
    else
    {
        Warning("Panel not created!\n");
    }
}

CON_COMMAND(show_endgame_lose, "Test endgame panel - LOSE")
{
    CGameResultPanel *pPanel = GetEndgamePanel();
    if (g_pGameResultPanel)
    {
        g_pGameResultPanel->ShowResult(false);
    }
    else
    {
        Warning("Panel not created!\n");
    }
}

CON_COMMAND(hide_endgame, "Hide endgame panel")
{
    CGameResultPanel *pPanel = GetEndgamePanel();
    if (g_pGameResultPanel)
    {
        g_pGameResultPanel->SetVisible(false);
    }
}