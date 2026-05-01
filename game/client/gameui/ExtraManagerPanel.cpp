#include "ExtraManagerPanel.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "tier1/checksum_crc.h" // 用于路径哈希
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"

#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"
#include "EngineInterface.h"
#include "ModInfo.h"
#include "GameUI_Interface.h"
#include "vgui/ILocalize.h"
#include "gametypes.h"

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

extern IFileSystem *g_pFullFileSystem;

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

// ========
// 辅助函数：加载 PNG 并返回 TextureID
// ========
static int CreatePNGTextureHelper(const char *szPath) {
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(szPath, "MOD", buf)) return -1;

    int width, height, channels;
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;

    int targetW = 128; // 统一缩放大小
    int targetH = 128;
    unsigned char *resizedData = (unsigned char *)malloc(targetW * targetH * 4);
    int textureID = -1;

    if (resizedData) {
        if (stbir_resize_uint8(data, width, height, width * 4, resizedData, targetW, targetH, targetW * 4, 4)) {
            textureID = vgui::surface()->CreateNewTextureID(true);
            vgui::surface()->DrawSetTextureRGBA(textureID, resizedData, targetW, targetH, true, false);
        }
        free(resizedData);
    }

    stbi_image_free(data);
    return textureID;
}

// =========================================================
// ImageUrlButton
// =========================================================
class ImageUrlButton : public vgui::Panel
{
public:
    ImageUrlButton(Panel *parent, const char *name, const char *imagePath, const char *url) : Panel(parent, name)
    {
        m_szUrl = url;
        m_bSelected = false;
        m_textureID = CreatePNGTextureHelper(imagePath);

        SetMouseInputEnabled(true);
        SetPaintBackgroundEnabled(false);
    }

    virtual ~ImageUrlButton() {
        if (vgui::surface()->IsTextureIDValid(m_textureID)) {
            vgui::surface()->DeleteTextureByID(m_textureID);
        }
    }

    virtual void Paint()
    {
        if (m_textureID == -1) return;
        int alpha = m_bSelected ? 150 : 255;
        vgui::surface()->DrawSetColor(255, 255, 255, alpha);
        vgui::surface()->DrawSetTexture(m_textureID);
        vgui::surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
    }

    virtual void OnMousePressed(MouseCode code) {
        if (code == MOUSE_LEFT) { m_bSelected = true; input()->SetMouseCapture(GetVPanel()); }
    }

    virtual void OnMouseReleased(MouseCode code) {
        if (code == MOUSE_LEFT) {
            m_bSelected = false;
            input()->SetMouseCapture(NULL);
            if (IsCursorOver() && m_szUrl) {
#ifdef ANDROID
                SDL_OpenURL(m_szUrl);
#else
                vgui::system()->ShellExecute("open", m_szUrl);
#endif
            }
        }
    }

private:
    bool m_bSelected;
    int m_textureID;
    const char *m_szUrl;
};



// =========================================================
// ExtraListPage 实现 (含纹理缓存)
// =========================================================
ExtraListPage::ExtraListPage(vgui::Panel *parent, const char *panelName) : BaseClass(parent, panelName) {
    // 创建过滤控件
    m_pFilterLabel = new vgui::Label(this, "FilterLabel", "#GameUI_Filtering_Mode");
    m_pFilterLabel->SetContentAlignment(vgui::Label::a_west);
    
    m_pGameTypeCombo = new vgui::ComboBox(this, "GameTypeCombo", 10, false);
    m_pGameTypeCombo->AddActionSignalTarget(this);
    
    m_pGameModeCombo = new vgui::ComboBox(this, "GameModeCombo", 10, false);
    m_pGameModeCombo->AddActionSignalTarget(this);
    
    m_pAllMapsCheck = new vgui::CheckButton(this, "AllMapsCheck", "#GameUI_AllMaps");
    m_pAllMapsCheck->AddActionSignalTarget(this);

    // 注册tick信号以驱动异步加载队列
    vgui::ivgui()->AddTickSignal(GetVPanel());
    
    // 初始化游戏类型列表
    int iGameTypeCount = g_pGameTypes->GetGameTypesCount();
    m_pGameTypeCombo->AddItem("#GameUI_AllMaps", new KeyValues("data", "game_type", -1));
    for (int i = 0; i < iGameTypeCount; i++) {
        const char* pszGameTypeNameID = g_pGameTypes->GetGameTypeNameID(i);
        if (pszGameTypeNameID) {
            m_pGameTypeCombo->AddItem(pszGameTypeNameID, new KeyValues("data", "game_type", i));
        }
    }
    m_pGameTypeCombo->ActivateItem(0);
    
    // 初始化游戏模式列表
    UpdateGameModeList();
    
    // 地图列表面板
    m_pMapListPanel = new vgui::PanelListPanel(this, "MapListPanel");
    m_pMapListPanel->SetFirstColumnWidth(0);
    
    int screenWidth, screenHeight;
    vgui::surface()->GetScreenSize(screenWidth, screenHeight);
    float scaleX = screenWidth / 1920.0f;
    float scaleY = screenHeight / 1080.0f;
    float scale = (scaleX + scaleY) / 2.0f;

    m_iItemWidth = 180 * scale;
    m_iItemHeight = 180 * scale;
    m_iItemSpacing = 12 * scale;
    m_iItemsPerRow = 4;

    m_pMapListPanel->SetNumColumns(m_iItemsPerRow);
    m_pMapListPanel->SetVerticalBufferPixels(m_iItemSpacing);

    m_TextureCache.SetLessFunc(DefLessFunc(unsigned int));
}

ExtraListPage::~ExtraListPage() {
    vgui::ivgui()->RemoveTickSignal(GetVPanel());
    CleanUpTextures();
}

void ExtraListPage::CleanUpTextures() {
    FOR_EACH_MAP(m_TextureCache, i) {
        int id = m_TextureCache[i];
        if (vgui::surface()->IsTextureIDValid(id)) { vgui::surface()->DeleteTextureByID(id); }
    }
    m_TextureCache.RemoveAll();
}

//-------------------------------------------------------------------------
// Purpose: 更新游戏模式列表
//-------------------------------------------------------------------------
void ExtraListPage::UpdateGameModeList()
{
    m_pGameModeCombo->DeleteAllItems();
    
    int nSelectedGameType = -1;
    KeyValues *pkvData = m_pGameTypeCombo->GetActiveItemUserData();
    if (pkvData) {
        nSelectedGameType = pkvData->GetInt("game_type", -1);
    }
    
    m_pGameModeCombo->AddItem("#GameUI_AllMaps", new KeyValues("data", "game_mode", -1));
    
    if (nSelectedGameType >= 0) {
        int iGameModeCount = g_pGameTypes->GetGameModesCount(nSelectedGameType);
        for (int i = 0; i < iGameModeCount; i++) {
            const char* pszGameModeNameID = g_pGameTypes->GetGameModeNameID(nSelectedGameType, i);
            if (pszGameModeNameID) {
                m_pGameModeCombo->AddItem(pszGameModeNameID, new KeyValues("data", "game_mode", i));
            }
        }
    }
    
    m_pGameModeCombo->ActivateItem(0);
}

int ExtraListPage::GetTextureForPath(const char *fullPath) {
    if (!fullPath || !fullPath[0]) return -1;

    // 使用 CRC 计算路径哈希作为 Key
    CRC32_t hash;
    CRC32_Init(&hash);
    CRC32_ProcessBuffer(&hash, fullPath, Q_strlen(fullPath));
    CRC32_Final(&hash);

    int index = m_TextureCache.Find(hash);
    if (index != m_TextureCache.InvalidIndex()) { return m_TextureCache[index]; }

    // 缓存中没有，执行实时加载
    int newID = CreateTextureFromPNG(fullPath);
    if (newID != -1) { m_TextureCache.Insert(hash, newID); }
    return newID;
}

int ExtraListPage::CreateTextureFromPNG(const char *fullPath) {
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(fullPath, "MOD", buf)) return -1;

    int width, height, channels;
    // 使用 stb_image 解码
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;

    // 性能优化：统一缩放到 128x128 节省显存
    int targetW = 128;
    int targetH = 128;
    unsigned char *resizedData = (unsigned char *)malloc(targetW * targetH * 4);
    if (resizedData) {
        if (stbir_resize_uint8(data, width, height, width * 4, resizedData, targetW, targetH, targetW * 4, 4)) {
            int textureID = vgui::surface()->CreateNewTextureID(true);
            vgui::surface()->DrawSetTextureRGBA(textureID, resizedData, targetW, targetH, true, false);
            stbi_image_free(data);
            free(resizedData);
            return textureID;
        }
        free(resizedData);
    }

    stbi_image_free(data);
    return -1;
}

void ExtraListPage::RefreshList() {
    // 1. 清空所有状态和现有的加载队列
    m_LoadQueue.RemoveAll();
    m_PendingMaps.RemoveAll();
    m_pMapListPanel->DeleteAllItems();
    
    // 向上寻找 ExtraManagerPanel 以获取 ServerPage 引用
    vgui::Panel *pTarget = GetParent();
    while (pTarget && !dynamic_cast<ExtraManagerPanel *>(pTarget)) { 
        pTarget = pTarget->GetParent(); 
    }
    
    if (!pTarget) return;
    ExtraManagerPanel *pMain = static_cast<ExtraManagerPanel *>(pTarget);
    CCreateMultiplayerGameServerPage *pServerPage = pMain->GetServerPage();
    if (!pServerPage) return;

    // 获取过滤条件
    int nFilterGameType = -1;
    int nFilterGameMode = -1;
    bool bShowAllMaps = m_pAllMapsCheck->IsSelected();
    
    KeyValues *pkvGameTypeData = m_pGameTypeCombo->GetActiveItemUserData();
    if (pkvGameTypeData) {
        nFilterGameType = pkvGameTypeData->GetInt("game_type", -1);
    }
    
    KeyValues *pkvGameModeData = m_pGameModeCombo->GetActiveItemUserData();
    if (pkvGameModeData) {
        nFilterGameMode = pkvGameModeData->GetInt("game_mode", -1);
    }
    
    const char *pszFilterGameType = (nFilterGameType >= 0) ? g_pGameTypes->GetGameTypeFromInt(nFilterGameType) : NULL;
    const char *pszFilterGameMode = (nFilterGameType >= 0 && nFilterGameMode >= 0) ? g_pGameTypes->GetGameModeFromInt(nFilterGameType, nFilterGameMode) : NULL;

    // 2. 快速扫描地图文件并存入 Pending 队列
    FileFindHandle_t findHandle = NULL;
    KeyValues *hiddenMaps = ModInfo().GetHiddenMaps();
    
    // 使用 "GAME" 路径搜索
    const char *pszFilename = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "GAME", &findHandle);
    
    while (pszFilename)
    {
        char mapname[256];
        char *ext, *str;

        str = Q_strstr(pszFilename, "maps");
        if (str) {
            Q_strncpy(mapname, str + 5, sizeof(mapname) - 1);
        } else {
            Q_strncpy(mapname, pszFilename, sizeof(mapname) - 1);
        }
        
        ext = Q_strstr(mapname, ".bsp");
        if (ext) {
            *ext = 0;
        }

        // 过滤逻辑 (保持原有逻辑不变)
        if (hiddenMaps && hiddenMaps->GetInt(mapname, 0))
            goto nextFile;

        if (!bShowAllMaps) 
        {
            if (pszFilterGameType) {
                if (!g_pGameTypes->IsValidMapForTypeAndMode(mapname, pszFilterGameType, pszFilterGameMode))
                    goto nextFile;
            } else {
                if (Q_stristr(mapname, "background") || Q_stristr(mapname, "vactest"))
                    goto nextFile;
            }
        }

        // 3. 将符合条件的地图加入待处理队列，而不是立即创建 Panel
        {
            int idx = m_PendingMaps.AddToTail();
            PendingMap_t &item = m_PendingMaps[idx];
            
            Q_strncpy(item.mapname, mapname, sizeof(item.mapname));
            
            const char *szUIMapName = g_pGameTypes->GetMapNameID(mapname);
            Q_strncpy(item.szUIMapName, (szUIMapName && szUIMapName[0]) ? szUIMapName : mapname, sizeof(item.szUIMapName));
            
            // 预生成图标路径，但不在这里检查文件是否存在（检查文件也是 IO 操作）
            Q_snprintf(item.szIconPath, sizeof(item.szIconPath), "materials/vgui/maps/%s.png", mapname);
        }

    nextFile:
        pszFilename = g_pFullFileSystem->FindNext(findHandle);
    }
    
    g_pFullFileSystem->FindClose(findHandle);
    
    // 扫描完成后，OnTick 会接手剩下的 UI 创建工作
}

// --- OnTick: 每帧处理加载队列 ---
void ExtraListPage::OnTick() {
    BaseClass::OnTick();

    // ---- 第一部分：分帧创建 MapCardPanel (每帧创建 4 个) ----
    if (!m_PendingMaps.IsEmpty()) {
        int cardsCreatedThisFrame = 0;
        const int MAX_CARDS_PER_FRAME = 4; // 这是一个平衡点，既不卡顿也能快速填充列表

        // 获取主面板指针以传递 ActionSignalTarget
        vgui::Panel *pTarget = GetParent();
        while (pTarget && !dynamic_cast<ExtraManagerPanel *>(pTarget)) { 
            pTarget = pTarget->GetParent(); 
        }

        while (!m_PendingMaps.IsEmpty() && cardsCreatedThisFrame < MAX_CARDS_PER_FRAME) {
            PendingMap_t &info = m_PendingMaps[0];

            // 创建卡片
            MapCardPanel *pCard = new MapCardPanel(m_pMapListPanel, info.mapname, info.szUIMapName);
            pCard->SetSize(m_iItemWidth, m_iItemHeight);
            
            // 直接设置路径并加入加载队列
            // 注意：我们将 FileExists 的检查推迟到 ExecuteLoad 中执行，进一步减少主线程负担
            pCard->SetImagePath(info.szIconPath);
            pCard->QueueForLoad();

            if (pTarget) {
                pCard->AddActionSignalTarget(pTarget);
            }

            m_pMapListPanel->AddItem(nullptr, pCard);

            // 从待处理队列中移除
            m_PendingMaps.Remove(0);
            cardsCreatedThisFrame++;
        }
    }

    // ---- 第二部分：处理纹理加载队列 (原有逻辑) ----
    ProcessLoadQueue();
}

// --- 将卡片加入加载队列 ---
void ExtraListPage::QueueCardForLoad(MapCardPanel *pCard) {
    if (!pCard) return;
    m_LoadQueue.AddToTail(pCard);
}

// --- 处理加载队列：每帧只加载少量 ---
void ExtraListPage::ProcessLoadQueue() {
    if (m_LoadQueue.IsEmpty()) return;

    // 每帧只处理少量，避免卡顿
    int loadsThisFrame = 0;
    while (!m_LoadQueue.IsEmpty() && loadsThisFrame < MAX_LOADS_PER_FRAME) {
        MapCardPanel *pCard = m_LoadQueue[0];
        m_LoadQueue.Remove(0);

        if (pCard && !pCard->IsLoadComplete() && pCard->IsQueuedForLoad()) {
            pCard->ExecuteLoad();
        }
        loadsThisFrame++;
    }
}

void ExtraListPage::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    
    if (m_pFilterLabel) m_pFilterLabel->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pGameTypeCombo) m_pGameTypeCombo->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pGameModeCombo) m_pGameModeCombo->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pAllMapsCheck) m_pAllMapsCheck->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
}

void ExtraListPage::PerformLayout() {
    BaseClass::PerformLayout();
    int w, h;
    GetSize(w, h);
    int margin = PROPVAL(8);
    int iFilterHeight = PROPVAL(24);
    int iSpacing = PROPVAL(6);
    
    // 布局过滤控件
    int currentY = margin;
    m_pFilterLabel->SetBounds(margin, currentY, PROPVAL(50), iFilterHeight);
    
    int iComboX = margin + PROPVAL(55);
    int iComboWidth = (w - iComboX - margin - iSpacing) / 2;
    m_pGameTypeCombo->SetBounds(iComboX, currentY, iComboWidth, iFilterHeight);
    m_pGameModeCombo->SetBounds(iComboX + iComboWidth + iSpacing, currentY, iComboWidth, iFilterHeight);
    
    currentY += iFilterHeight + iSpacing;
    m_pAllMapsCheck->SetBounds(margin, currentY, w - margin * 2, iFilterHeight);
    
    currentY += iFilterHeight + iSpacing;
    int iListTop = currentY;
    
    // 计算响应式布局
    int availableWidth = w - (margin * 2);
    m_iItemsPerRow = availableWidth / (m_iItemWidth + m_iItemSpacing);
    if (m_iItemsPerRow < 1) m_iItemsPerRow = 1;
    
    m_pMapListPanel->SetNumColumns(m_iItemsPerRow);
    m_pMapListPanel->SetVerticalBufferPixels(m_iItemSpacing);
    m_pMapListPanel->SetBounds(margin, iListTop, availableWidth, h - iListTop - margin);
    
    // 更新已有卡片的大小
    for (int itemID = m_pMapListPanel->FirstItem(); itemID != m_pMapListPanel->InvalidItemID(); itemID = m_pMapListPanel->NextItem(itemID))
    {
        vgui::Panel *pItem = m_pMapListPanel->GetItemPanel(itemID);
        if (pItem)
        {
            pItem->SetSize(m_iItemWidth, m_iItemHeight);
        }
    }
}

void ExtraListPage::UpdateSelection(const char *pPanelName)
{
    if (!m_pMapListPanel) return;

    for (int itemID = m_pMapListPanel->FirstItem(); itemID != m_pMapListPanel->InvalidItemID(); itemID = m_pMapListPanel->NextItem(itemID))
    {
        MapCardPanel *pCard = dynamic_cast<MapCardPanel *>(m_pMapListPanel->GetItemPanel(itemID));
        if (pCard)
        {
            pCard->SetSelected(!Q_stricmp(pCard->GetName(), pPanelName));
        }
    }
}

// =========================================================
// ExtraManagerPanel 实现
// =========================================================
ExtraManagerPanel::ExtraManagerPanel(vgui::Panel *parent) : BaseClass(parent, "ExtraManagerPanel") {

    int screenW, screenH;
    vgui::surface()->GetScreenSize(screenW, screenH);
    SetSize(screenW, screenH);

    SetTitle("", false);
    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);
    SetMoveable(false);
    SetSizeable(false);
    SetCloseButtonVisible(false);
    
    // create KeyValues object to load/save config options
	m_pSavedData = new KeyValues( "ServerConfig" );
	
	int nGameType = 0;
	int nGameMode = 0;
	bool bAllMaps = false;
	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile( g_pFullFileSystem, "ServerConfig.vdf", "GAME" ); // this is game-specific data, so it should live in GAME, not CONFIG
		
		nGameType = m_pSavedData->GetInt( "game_type" );
		nGameMode = m_pSavedData->GetInt( "game_mode" );
		bAllMaps = m_pSavedData->GetBool( "all_maps" );
	}

    m_pLeftPanel = new BlurredEditablePanel(this, "LeftFloatingPanel");
    m_pTabSheet = new PropertySheet(m_pLeftPanel, "ExtraTabs");
    m_pMapListPage = new ExtraListPage(m_pTabSheet, "MapListPage");
    
    m_pServerPage = new CCreateMultiplayerGameServerPage(this, "ServerPage", nGameType, nGameMode, bAllMaps);
    m_pGameplayPage = new CCreateMultiplayerGameGameplayPage(this, "GameplayPage");
    m_pBotPage = NULL;
    
    m_pServerPage->UpdateGameplayPage(); // do it AFTER m_pGameplayPage has been added

    if ( m_pSavedData )
	{
		const char *startMap = m_pSavedData->GetString("map", "");
		if (startMap[0])
		{
			m_pServerPage->SetMap(startMap);
		}
		const char *hostname = m_pSavedData->GetString("hostname", "");
		if (hostname[0])
		{
			m_pServerPage->SetHostName(hostname);
		}
		const char *maxplayers = m_pSavedData->GetString("maxplayers", "");
		if (maxplayers[0])
		{
			m_pServerPage->SetMaxPlayers(maxplayers);
		}
		const char *sv_password = m_pSavedData->GetString("sv_password", "");
		if (sv_password[0])
		{
			m_pServerPage->SetPassword(sv_password);
		}
	}

    m_pTabSheet->AddPage(m_pServerPage, "#GameUI_Server");
    m_pTabSheet->AddPage(m_pMapListPage, "#GameUI_Map");
    
    if ( ModInfo().UseBots() )
	{
		m_pBotPage = new CCreateMultiplayerGameBotPage( m_pTabSheet, "BotPage", m_pSavedData );
		m_pTabSheet->AddPage( m_pBotPage, "#GameUI_CPUPlayerOptions" );
		m_pServerPage->EnableBots( m_pSavedData );
	}
    
    m_pTabSheet->AddPage(m_pGameplayPage, "#GameUI_Game");

    m_pRightPanel = new BlurredEditablePanel(this, "RightFloatingPanel");
    m_pDetailsLabel = new vgui::Label(m_pRightPanel, "DetailsLabel", "Information");
    
    m_pRefreshButton = new vgui::Button(m_pRightPanel, "RefreshBtn", "#GameUI_Refresh", this, "RefreshList");
    m_pStartButton = new vgui::Button(m_pRightPanel, "StartBtn", "#GameUI_Start", this, "StartGame");
    m_pCloseButton = new Button(this, "CloseBtn", "#GameUI_Close", this, "Close");
}

ExtraManagerPanel::~ExtraManagerPanel() {
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

void ExtraManagerPanel::OnMapCardSelected(KeyValues *data) {
    if (!data) return;
    const char *pPanelName = data->GetString("panelName", "");

    if (m_pServerPage) {
        m_pServerPage->SetMap(pPanelName);
    }

    if (m_pMapListPage) {
        m_pMapListPage->UpdateSelection(pPanelName);
    }
}

void ExtraManagerPanel::StartGame() {
    // 1. 先重置所有被修改的 ConVars，这样 ApplyChanges 才能覆盖它们
    if (g_pCVar) {
        g_pCVar->RevertFlaggedConVars(FCVAR_REPLICATED);
        g_pCVar->RevertFlaggedConVars(FCVAR_CHEAT);
    }

    DevMsg("FCVAR_CHEAT cvars reverted to defaults.\n");

    // 2. 调用选项卡的 ApplyChanges，这会触发所有 Page 的 OnApplyChanges
    if (m_pTabSheet) {
        m_pTabSheet->ApplyChanges();
    }

    // get these values from m_pServerPage and store them temporarily
    char szMapName[64], szHostName[64], szPassword[64];
    int iGameTypeID = m_pServerPage->GetGameTypeID();
    int iGameModeID = m_pServerPage->GetGameModeID();
    int iMaxPlayers = m_pServerPage->GetMaxPlayers();
    Q_strncpy(szMapName, m_pServerPage->GetMapName(), sizeof(szMapName));
    Q_strncpy(szHostName, m_pServerPage->GetHostName(), sizeof(szHostName));
    Q_strncpy(szPassword, m_pServerPage->GetPassword(), sizeof(szPassword));

    int iBotQuota = 0;

    // save the config data
    if (m_pSavedData) {
        if (m_pServerPage->IsRandomMapSelected()) {
            m_pSavedData->SetString("map", "");
        } else {
            m_pSavedData->SetString("map", szMapName);
        }

        m_pSavedData->SetInt("game_type", iGameTypeID);
        m_pSavedData->SetInt("game_mode", iGameModeID);
        m_pSavedData->SetBool("all_maps", m_pServerPage->IsAllMaps());
        m_pSavedData->SetString("hostname", szHostName);
        m_pSavedData->SetInt("maxplayers", iMaxPlayers);
        m_pSavedData->SetString("sv_password", szPassword);

        // 获取机器人数量并保存
        iBotQuota = m_pSavedData->GetInt("bot_quota", 0);
        // 如果难度选择为 0 (通常是 "无机器人" 选项)，则强制数量为 0
        if (m_pSavedData->GetInt("custom_bot_difficulty", 0) == 0)
            iBotQuota = 0;

        // save config to a file
        m_pSavedData->SaveToFile(g_pFullFileSystem, "ServerConfig.vdf", "GAME");
    }

    char szMapCommand[1024];

    // create the command to execute
    // 增加一些必要的等待和初始化命令，确保 ConVars 已经应用
    // 显式在命令中设置 bot_quota 以确保生效
    Q_snprintf(szMapCommand, sizeof(szMapCommand),
               "disconnect\nwait\nwait\nsv_lan 1\nsetmaster enable\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nbot_quota %i\ngame_type %d\ngame_mode %d\ngame_online 0\nprogress_enable\nexec listenserver.cfg\nmap %s\n",
               iMaxPlayers, szPassword, szHostName, iBotQuota, iGameTypeID, iGameModeID, szMapName);

    // exec
    engine->ClientCmd_Unrestricted(szMapCommand);

    Close();
}





void ExtraManagerPanel::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);

    if (m_pLeftPanel) {
        m_pLeftPanel->SetPaintBackgroundEnabled(true);
        m_pLeftPanel->SetPaintBorderEnabled(true);
        m_pLeftPanel->SetBorder(pScheme->GetBorder("FrameBorder"));
        Color bgColor = pScheme->GetColor("Frame.BgColor", Color(0, 0, 0, 200));
        m_pLeftPanel->SetDarkness((float)bgColor.a());
    }

    if (m_pRightPanel) {
        m_pRightPanel->SetPaintBackgroundEnabled(true);
        m_pRightPanel->SetPaintBorderEnabled(true);
        m_pRightPanel->SetBorder(pScheme->GetBorder("FrameBorder"));
        Color bgColor = pScheme->GetColor("Frame.BgColor", Color(0, 0, 0, 150));
        m_pRightPanel->SetDarkness((float)bgColor.a());
    }

    if (m_pDetailsLabel) m_pDetailsLabel->SetFont(pScheme->GetFont("DefaultLarge", IsProportional()));
    if (m_pCloseButton) m_pCloseButton->SetFont(pScheme->GetFont("DefaultLarge", IsProportional()));
}

void ExtraManagerPanel::PerformLayout() {
    BaseClass::PerformLayout();
    int sw, sh;
    GetSize(sw, sh);
    int iPadding = PROPVAL(0), iGap = PROPVAL(10);
    int leftW = (sw * 0.75) - (iPadding + iGap / 2);
    int rightW = sw - leftW - (iPadding * 2) - iGap;
    int panelH = sh - (iPadding * 2);

    m_pLeftPanel->SetBounds(iPadding, iPadding, leftW, panelH);
    m_pRightPanel->SetBounds(iPadding + leftW + iGap, iPadding, rightW, panelH);

    int tPadding = PROPVAL(12);
    m_pTabSheet->SetBounds(tPadding, tPadding, leftW - (tPadding * 2), panelH - (tPadding * 2));

    int rInnerPad = PROPVAL(15);
    m_pDetailsLabel->SetBounds(rInnerPad, rInnerPad, rightW - (rInnerPad * 2), PROPVAL(30));

    // Arrange buttons vertically at the bottom right
    int btnW = PROPVAL(110), btnH = PROPVAL(28);
    int btnGap = PROPVAL(10);
    int totalBtnHeight = (btnH * 3) + (btnGap * 2);
    int startY = panelH - rInnerPad - totalBtnHeight;
    
    // Vertical arrangement: Refresh at top, Start in middle, Close at bottom
    m_pRefreshButton->SetBounds(rInnerPad, startY, btnW, btnH);
    m_pStartButton->SetBounds(rInnerPad, startY + btnH + btnGap, btnW, btnH);
    m_pCloseButton->SetBounds(sw - iPadding - rInnerPad - btnW, sh - iPadding - rInnerPad - btnH, btnW, btnH);
}

void ExtraManagerPanel::OnCommand(const char *command) {
    if (!Q_stricmp(command, "Close"))
        Close();
    else if (!Q_stricmp(command, "RefreshList") && m_pMapListPage)
        m_pMapListPage->RefreshList();
    else if (!Q_stricmp(command, "StartGame"))
        StartGame();
    else
        BaseClass::OnCommand(command);
}

void ExtraManagerPanel::Activate() {
    BaseClass::Activate();
    if (m_pMapListPage) m_pMapListPage->RefreshList();
}

void ExtraManagerPanel::OnKeyCodePressed( vgui::KeyCode code )
{
	// Handle close here, CBasePanel parent doesn't support "DialogClosing" command
	ButtonCode_t nButtonCode = GetBaseButtonCode( code );

	if ( nButtonCode == KEY_XBUTTON_B )
	{
		OnCommand( "Close" );
	}
	else if ( nButtonCode == KEY_XBUTTON_A || nButtonCode == STEAMCONTROLLER_A )
	{
		StartGame();
	}
	else if ( nButtonCode == KEY_XBUTTON_UP || 
			  nButtonCode == KEY_XSTICK1_UP ||
			  nButtonCode == KEY_XSTICK2_UP ||
			  nButtonCode == STEAMCONTROLLER_DPAD_UP ||
			  nButtonCode == KEY_UP )
	{
		if (m_pServerPage && m_pServerPage->GetMapList())
		{
			int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) - 1;
			if ( nItem < 0 )
			{
				nItem = m_pServerPage->GetMapList()->GetItemCount() - 1;
			}
			m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
		}
	}
	else if ( nButtonCode == KEY_XBUTTON_DOWN || 
			  nButtonCode == KEY_XSTICK1_DOWN ||
			  nButtonCode == KEY_XSTICK2_DOWN || 
			  nButtonCode == STEAMCONTROLLER_DPAD_DOWN ||
			  nButtonCode == KEY_DOWN )
	{
		if (m_pServerPage && m_pServerPage->GetMapList())
		{
			int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) + 1;
			if ( nItem >= m_pServerPage->GetMapList()->GetItemCount() )
			{
				nItem = 0;
			}
			m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void ExtraManagerPanel::OnClose() {
    BaseClass::OnClose();
    MarkForDeletion();
}

//-------------------------------------------------------------------------
// Purpose: ExtraListPage 消息处理
//-------------------------------------------------------------------------
void ExtraListPage::OnTextChanged(Panel *panel)
{
    if (panel == m_pGameTypeCombo)
    {
        UpdateGameModeList();
        RefreshList();
    }
    else if (panel == m_pGameModeCombo)
    {
        RefreshList();
    }
}

void ExtraListPage::OnCheckButtonChecked(Panel *panel)
{
    if (panel == m_pAllMapsCheck)
    {
        RefreshList();
    }
}
