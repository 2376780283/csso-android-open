#include "CardPanel.h"
#include "ExtraManagerPanel.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "tier1/checksum_crc.h"
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

// 辅助函数：加载 PNG 并返回 TextureID
static int CreatePNGTextureHelper(const char *szPath) {
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(szPath, "MOD", buf)) return -1;

    int width, height, channels;
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;

    int targetW = 128; 
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
// MapCardPanel 实现 (80x80 固定布局)
// =========================================================
MapCardPanel::MapCardPanel(vgui::Panel *parent, const char *name, const char *title) : BaseClass(parent, name) {
    m_nTextureID = -1;
    m_bAttemptedLoad = false;
    m_bQueuedForLoad = false;
    m_szImagePath[0] = '\0';
    Q_strncpy(m_szUIMapName, title ? title : "", sizeof(m_szUIMapName));

    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetMouseInputEnabled(true);
    
    // 直接使用原生像素值，废弃 PROPVAL
    m_iMargin = 5; 
    m_bSelected = false;

    m_clrBgNormal = Color(0, 0, 0, 0);
    m_clrBgHover = Color(89, 221, 242, 150);

    m_pImagePanelPlaceholder = new vgui::ImagePanel(this, "MapImage");
    m_pImagePanelPlaceholder->SetShouldScaleImage(true);
    m_pImagePanelPlaceholder->SetMouseInputEnabled(false);
    m_pImagePanelPlaceholder->SetVisible(false);

    // 创建底部容器：在 80x80 内部下方
    m_pContainer = new vgui::Panel(this, "MapContainer");
    m_pContainer->SetPaintBackgroundEnabled(true);
    m_pContainer->SetPaintBorderEnabled(false);
    m_pContainer->SetBgColor(Color(0, 0, 0, 180)); // 略深一点的背景提高文字可读性
    m_pContainer->SetMouseInputEnabled(false);

    m_pTitle = new vgui::Label(m_pContainer, "MapTitle", title);
    m_pTitle->SetPaintBackgroundEnabled(false);
    m_pTitle->SetFgColor(Color(255, 255, 255, 255));
    m_pTitle->SetContentAlignment(vgui::Label::a_center);
    m_pTitle->SetMouseInputEnabled(false);
}

void MapCardPanel::SetImagePath(const char *path) {
    if (path) { Q_strncpy(m_szImagePath, path, sizeof(m_szImagePath)); }
}

void MapCardPanel::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    // 使用最小字体以适应宽度
    m_pTitle->SetFont(pScheme->GetFont("DefaultVerySmall", false));
}

void MapCardPanel::Paint() {
    BaseClass::Paint();

    int wide, tall;
    GetSize(wide, tall);
    
    int drawX = m_iMargin;
    int drawY = m_iMargin;
    int imgW = wide - (m_iMargin * 2);
    int imgH = tall - (m_iMargin * 2);

    if (m_nTextureID != -1 && vgui::surface()->IsTextureIDValid(m_nTextureID)) {
        vgui::surface()->DrawSetColor(255, 255, 255, 255);
        vgui::surface()->DrawSetTexture(m_nTextureID);
        vgui::surface()->DrawTexturedRect(drawX, drawY, drawX + imgW, drawY + imgH);
    } else {
        vgui::surface()->DrawSetColor(30, 30, 30, 255);
        vgui::surface()->DrawFilledRect(drawX, drawY, drawX + imgW, drawY + imgH);
    }

    if (m_bSelected) {
        vgui::surface()->DrawSetColor(89, 221, 242, 255);
        for (int i = 0; i < 2; i++) {
            vgui::surface()->DrawOutlinedRect(drawX + i, drawY + i, drawX + imgW - i, drawY + imgH - i);
        }
    }
}

void MapCardPanel::QueueForLoad() {
    if (m_bQueuedForLoad || m_bAttemptedLoad || m_szImagePath[0] == '\0') return;
    m_bQueuedForLoad = true;

    vgui::Panel *pPage = GetParent();
    while (pPage && !dynamic_cast<ExtraListPage *>(pPage)) { pPage = pPage->GetParent(); }

    if (pPage) {
        ExtraListPage *pListPage = static_cast<ExtraListPage *>(pPage);
        pListPage->QueueCardForLoad(this);
    }
}

void MapCardPanel::ExecuteLoad() {
    if (m_bAttemptedLoad || m_szImagePath[0] == '\0') return;

    vgui::Panel *pPage = GetParent();
    while (pPage && !dynamic_cast<ExtraListPage *>(pPage)) { pPage = pPage->GetParent(); }

    if (pPage) {
        ExtraListPage *pListPage = static_cast<ExtraListPage *>(pPage);
        m_nTextureID = pListPage->GetTextureForPath(m_szImagePath);
        m_bAttemptedLoad = true;
        m_bQueuedForLoad = false;
    }
}

void MapCardPanel::PerformLayout() {
    BaseClass::PerformLayout();
    
    int wide, tall;
    GetSize(wide, tall);
    
    m_pImagePanelPlaceholder->SetBounds(0, 0, wide, tall);

    // 布局底部容器：高度固定为 20，位于底端
    int labelH = 20;
    int labelY = tall - labelH;
    m_pContainer->SetBounds(0, labelY, wide, labelH);

    // 布局标题标签（充满容器）
    m_pTitle->SetBounds(0, 0, wide, labelH);
}

void MapCardPanel::OnCursorEntered() {
    SetBgColor(m_clrBgHover);
}
void MapCardPanel::OnCursorExited() {
    SetBgColor(m_clrBgNormal);
}

void MapCardPanel::OnMousePressed(vgui::MouseCode code) {
    if (code == MOUSE_LEFT) {
        KeyValues *msg = new KeyValues("MapCardSelected");
        msg->SetString("panelName", GetName());
        msg->SetString("uiMapName", m_szUIMapName);
        PostActionSignal(msg);
    }
}