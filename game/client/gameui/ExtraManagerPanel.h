#pragma once

#include "vgui/ISurface.h"
#include "GameUI_Interface.h"
#include "vgui/ISystem.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/PanelListPanel.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/RichText.h"
#include "vgui_controls/ComboBox.h"

#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"

#include "BlurUnified.h"

#ifdef ANDROID
#include <SDL_misc.h>
#endif

#include "utlvector.h"
#include "utlmap.h"

#include "CardPanel.h"
#include "CustomMenu.h"

// ---------------------------------------------------------
// BlurredEditablePanel - 带模糊背景的编辑面板
// ---------------------------------------------------------
class BlurredEditablePanel : public vgui::EditablePanel {
	DECLARE_CLASS_SIMPLE(BlurredEditablePanel, vgui::EditablePanel);
public:
	BlurredEditablePanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name) {
		m_fDarkness = 160.0f;
	}

	virtual void PaintBackground() override {
		int x, y, w, h;
		GetBounds(x, y, w, h);
		m_BlurHelper.ApplyBlur(x, y, w, h, m_fDarkness / 255.0f, true);
	}

	void SetDarkness(float fDarkness) { m_fDarkness = fDarkness; }

private:
	CUnifiedBlurHelper m_BlurHelper;
	float m_fDarkness;
};

// ---------------------------------------------------------
// 列表页面：管理纹理生命周期
// ---------------------------------------------------------
class ExtraListPage : public vgui::PropertyPage {
    DECLARE_CLASS_SIMPLE(ExtraListPage, vgui::PropertyPage);
public:
    ExtraListPage(vgui::Panel *parent, const char *panelName);
    virtual ~ExtraListPage(); 

    virtual void PerformLayout() override;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void OnTick() override;
    void RefreshList(); 

    // 提供给 MapCardPanel 调用的纹理加载接口
    int GetTextureForPath(const char *fullPath);
    // 异步加载队列管理
    void QueueCardForLoad(MapCardPanel *pCard);
    void ProcessLoadQueue();

    void UpdateSelection(const char *pPanelName);

    // 存储待处理的地图数据结构
struct PendingMap_t {
    char mapname[256];
    char szUIMapName[256];
    char szIconPath[MAX_PATH];
};

CUtlVector<PendingMap_t> m_PendingMaps; // 待创建 UI 的队列
bool m_bIsScanning = false;             

protected:
    int m_iItemWidth;
    int m_iItemHeight;
    int m_iItemSpacing;
    int m_iItemsPerRow;

private:
    void UpdateGameModeList();
    void ApplyMapFilters();
    int CreateTextureFromPNG(const char *fullPath);
    void CleanUpTextures();

    // 过滤控件
    vgui::Label *m_pFilterLabel;
    vgui::ComboBox *m_pGameTypeCombo;
    vgui::ComboBox *m_pGameModeCombo;
    vgui::CheckButton *m_pAllMapsCheck;
    
    vgui::PanelListPanel *m_pMapListPanel; 

    // 纹理缓存：Key 是路径哈希或字符串，Value 是 TextureID
    CUtlMap<unsigned int, int> m_TextureCache; 

    // 异步加载队列：每帧只加载少量纹理避免卡顿
    CUtlVector<MapCardPanel*> m_LoadQueue;
    static const int MAX_LOADS_PER_FRAME = 2; // 每帧最多加载2个纹理
    
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
    MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);
};

// ---------------------------------------------------------
// 主窗口
// ---------------------------------------------------------
class ExtraManagerPanel : public vgui::Frame {
    DECLARE_CLASS_SIMPLE(ExtraManagerPanel, vgui::Frame);
public:
    ExtraManagerPanel(vgui::Panel *parent);
    virtual ~ExtraManagerPanel();

    virtual void Activate() override;
    virtual void OnCommand(const char *command) override;
    virtual void OnClose() override;
    virtual void PerformLayout() override;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void OnKeyCodePressed(vgui::KeyCode code) override;

    void StartGame();
    MESSAGE_FUNC_PARAMS( OnMapCardSelected, "MapCardSelected", data );

    CCreateMultiplayerGameServerPage *GetServerPage() { return m_pServerPage; }

    BlurredEditablePanel *m_pLeftPanel;   
    vgui::PropertySheet *m_pTabSheet;
    ExtraListPage       *m_pMapListPage;

    BlurredEditablePanel *m_pRightPanel;  
    vgui::Label         *m_pDetailsLabel;
    
    vgui::Button        *m_pRefreshButton;
    vgui::Button        *m_pCloseButton;
    vgui::Button        *m_pStartButton;

    // tabs
    CCreateMultiplayerGameServerPage   *m_pServerPage;
    CCreateMultiplayerGameGameplayPage *m_pGameplayPage;
    CCreateMultiplayerGameBotPage      *m_pBotPage;

    // for loading/saving game config
    KeyValues *m_pSavedData;
};


