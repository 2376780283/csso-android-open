//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Custom menu components implementation
//
//=============================================================================

#include "cbase.h"
#include "CustomMenu.h"
#include "BasePanel.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"
#include "filesystem.h"

#include <materialsystem/itexture.h>
#include <materialsystem/imaterialvar.h>
#include "stb/stb_image.h"

#include "stb/stb_image_resize.h"
#include "tier0/memdbgon.h"

// External functions and variables needed
extern CBaseModPanel *BasePanel();

// ====================================
// 辅助函数：加载 PNG 并返回 TextureID
// ====================================
static int CreatePNGTextureHelper(const char *szPath)
{
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(szPath, "MOD", buf)) return -1;
    
    int width, height, channels;
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;
    
    int targetW = 128; // 统一缩放大小
    int targetH = 128;
    unsigned char *resizedData = (unsigned char *)malloc(targetW * targetH * 4);
    int textureID = -1;
    
    if (resizedData)
    {
        if (stbir_resize_uint8(data, width, height, width * 4, resizedData, targetW, targetH, targetW * 4, 4))
        {
            textureID = vgui::surface()->CreateNewTextureID(true);
            vgui::surface()->DrawSetTextureRGBA(textureID, resizedData, targetW, targetH, true, false);
        }
        free(resizedData);
    }
    
    stbi_image_free(data);
    return textureID;
}

// ====================================
// ImageButton 实现
// ====================================
ImageButton::ImageButton(vgui::Panel *parent, CBaseModPanel* pBasePanel, const char *name, const char *imagePath)
    : BaseClass(parent, name)
{
    m_pBasePanel = pBasePanel;
    m_bSelected = false;
    m_bIsMarked = false;
    m_textureID = CreatePNGTextureHelper(imagePath);
    m_textureID_Marked = -1;

    SetMouseInputEnabled(true);
    SetPaintBackgroundEnabled(false);
}

void ImageButton::SetActions(PanelActionFunc_t defaultAction, PanelActionFunc_t markedAction)
{
    m_pDefaultAction = defaultAction;
    m_pMarkedAction = markedAction;
}

void ImageButton::SetMarkedInfo(const char *imagePath)
{
    if (m_textureID_Marked != -1)
    {
        vgui::surface()->DeleteTextureByID(m_textureID_Marked);
    }
    m_textureID_Marked = CreatePNGTextureHelper(imagePath);
}

void ImageButton::SetMarked(bool bMarked)
{
    if (m_bIsMarked != bMarked)
    {
        m_bIsMarked = bMarked;
        Repaint();
    }
}

void ImageButton::Paint()
{
    int currentTexture = (m_bIsMarked && m_textureID_Marked != -1) ? m_textureID_Marked : m_textureID;
    if (currentTexture == -1) return;
    
    vgui::surface()->DrawSetColor(255, 255, 255, m_bSelected ? 150 : 255);
    vgui::surface()->DrawSetTexture(currentTexture);
    vgui::surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
}

void ImageButton::OnMousePressed(vgui::MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        m_bSelected = true;
        vgui::input()->SetMouseCapture(GetVPanel());
    }
}

void ImageButton::OnMouseReleased(vgui::MouseCode code)
{
    if (code == MOUSE_LEFT && m_bSelected)
    {
        if (m_pBasePanel)
        {
            PanelActionFunc_t action = (m_bIsMarked && m_pMarkedAction) ? m_pMarkedAction : m_pDefaultAction;
            if (action)
            {
                (m_pBasePanel->*action)();
            }
        }
    }
    m_bSelected = false;
    vgui::input()->SetMouseCapture(NULL);
}

void ImageButton::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
}

// ====================================
// CGUIBlurHelper 实现
// ====================================
CGUIBlurHelper::CGUIBlurHelper()
{
    m_bInitialized = false;
}

void CGUIBlurHelper::InitResources()
{
    if (m_bInitialized) return;

    materials->BeginRenderTargetAllocation();

    m_HelperRT.Init(materials->CreateNamedRenderTargetTextureEx2(
        "_rt_GUI_BlurTemp0", 256, 256, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888,
        MATERIAL_RT_DEPTH_NONE, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 0
    ));

    m_CaptureRT.Init(materials->CreateNamedRenderTargetTextureEx2(
        "_rt_GUI_BlurCapture", 256, 256, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888,
        MATERIAL_RT_DEPTH_NONE, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 0
    ));

    m_BlurX.Init("vgui/blur_x", TEXTURE_GROUP_OTHER);
    m_BlurY.Init("vgui/blur_y", TEXTURE_GROUP_OTHER);

    materials->EndRenderTargetAllocation();
    m_bInitialized = true;
}

void CGUIBlurHelper::DrawBlur(vgui::Panel *pPanel, float fDarkness)
{
    if (!pPanel) return;

    int x, y, w, h;
    vgui::ipanel()->GetAbsPos(pPanel->GetVPanel(), x, y);
    w = pPanel->GetWide();
    h = pPanel->GetTall();

    if (w <= 0 || h <= 0) return;

    InitResources();

    CMatRenderContextPtr pRenderContext(materials);

    if (m_BlurX.IsValid() && m_BlurY.IsValid() && m_HelperRT.IsValid() && m_CaptureRT.IsValid())
    {
        // 1. Capture screen area behind this panel to m_CaptureRT
        pRenderContext->Flush();
        Rect_t srcRect = { x, y, w, h };
        Rect_t dstRect = { 0, 0, 256, 256 };
        pRenderContext->CopyRenderTargetToTextureEx(m_CaptureRT, 0, &srcRect, &dstRect);

        // 2. Pass 1: Horizontal Blur (m_CaptureRT -> m_HelperRT)
        IMaterialVar *pBaseTextureVar = m_BlurX->FindVar("$basetexture", NULL);
        if (pBaseTextureVar)
        {
            pBaseTextureVar->SetTextureValue(m_CaptureRT);
        }

        pRenderContext->PushRenderTargetAndViewport(m_HelperRT);
        pRenderContext->ClearColor4ub(0, 0, 0, 255);
        pRenderContext->ClearBuffers(true, false);

        pRenderContext->DrawScreenSpaceRectangle(m_BlurX, 0, 0, 256, 256, 
                                                0, 0, 255, 255, 
                                                256, 256);
        pRenderContext->PopRenderTargetAndViewport();

        // 3. Pass 2: Vertical Blur (m_HelperRT -> Screen)
        pBaseTextureVar = m_BlurY->FindVar("$basetexture", NULL);
        if (pBaseTextureVar)
        {
            pBaseTextureVar->SetTextureValue(m_HelperRT);
        }

        pRenderContext->DrawScreenSpaceRectangle(m_BlurY, x, y, w, h, 
                                                0, 0, 255, 255, 
                                                256, 256);
    }
    else
    {
        // Fallback
        vgui::surface()->DrawSetColor(30, 30, 30, 200);
        vgui::surface()->DrawFilledRect(0, 0, w, h);
    }

    // Overlay dark layer
    if (fDarkness > 0)
    {
        vgui::surface()->DrawSetColor(30, 30, 30, (int)fDarkness);
        vgui::surface()->DrawFilledRect(0, 0, w, h);
    }
}

// ====================================
// NvgBarPanel 实现
// ====================================
NvgBarPanel::NvgBarPanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
    SetPaintBackgroundEnabled(true);
    SetPaintBackgroundType(2);
    SetVisible(true);
}

void NvgBarPanel::PaintBackground()
{
    m_BlurHelper.DrawBlur(this);
}

void NvgBarPanel::UpdateLayout()
{
    if (BasePanel())
    {
        BasePanel()->UpdateCustomMenuUI();
    }
}

// ====================================
// NewsListItem 实现
// ====================================
NewsListItem::NewsListItem(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
    m_pIcon = new vgui::ImagePanel(this, "NewsIcon");
    m_pText = new vgui::Label(this, "NewsText", "");
    
    m_pIcon->SetShouldScaleImage(true);
    m_pIcon->SetDrawColor(Color(255, 255, 255, 255));
    
    m_pText->SetContentAlignment(vgui::Label::a_west);
    m_pText->SetTextInset(5, 0);
    
    SetMouseInputEnabled(true);
    SetPaintBackgroundEnabled(false);
    
    m_iTextureID = -1;
}

void NewsListItem::SetNewsInfo(const char *iconPath, const wchar_t *text)
{
    if (iconPath && iconPath[0])
    {
        m_iTextureID = CreatePNGTextureHelper(iconPath);
        if (m_iTextureID != -1)
        {
            char textureName[64];
            Q_snprintf(textureName, sizeof(textureName), "news_icon_%d", m_iTextureID);
            m_pIcon->SetImage(textureName);
        }
    }
    
    if (text)
    {
        m_pText->SetText(text);
    }
}

void NewsListItem::SetNewsInfo(const char *iconPath, const char *text)
{
    wchar_t wtext[256];
    if (text)
    {
        g_pVGuiLocalize->ConvertANSIToUnicode(text, wtext, sizeof(wtext));
        SetNewsInfo(iconPath, wtext);
    }
    else
    {
        SetNewsInfo(iconPath, L"");
    }
}

void NewsListItem::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    
    m_pText->SetFgColor(GetSchemeColor("MainMenu.TextColor", pScheme));
    
    vgui::HFont hFont = pScheme->GetFont("GameMenuFont", IsProportional());
    if (hFont)
    {
        m_pText->SetFont(hFont);
    }
}

void NewsListItem::PerformLayout()
{
    BaseClass::PerformLayout();
    
    int wide, tall;
    GetSize(wide, tall);
    
    int iconSize = tall - 4;
    int iconX = 2;
    int iconY = (tall - iconSize) / 2;
    
    m_pIcon->SetBounds(iconX, iconY, iconSize, iconSize);
    
    int textX = iconX + iconSize + 5;
    int textWidth = wide - textX - 2;
    
    m_pText->SetBounds(textX, 0, textWidth, tall);
}

// ====================================
// NewsListPanel 实现
// ====================================
NewsListPanel::NewsListPanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
    SetPaintBackgroundEnabled(true);
    SetPaintBackgroundType(2);
    SetBgColor(Color(90, 90, 90, 169));
}

void NewsListPanel::AddNewsItem(const char *iconPath, const wchar_t *text)
{
    char itemName[64];
    Q_snprintf(itemName, sizeof(itemName), "NewsItem%d", m_Items.Count());
    
    NewsListItem *pItem = new NewsListItem(this, itemName);
    pItem->SetNewsInfo(iconPath, text);
    
    m_Items.AddToTail(pItem);
    
    InvalidateLayout();
}

void NewsListPanel::AddNewsItem(const char *iconPath, const char *text)
{
    wchar_t wtext[256];
    if (text)
    {
        g_pVGuiLocalize->ConvertANSIToUnicode(text, wtext, sizeof(wtext));
        AddNewsItem(iconPath, wtext);
    }
    else
    {
        AddNewsItem(iconPath, L"");
    }
}

void NewsListPanel::ClearItems()
{
    for (int i = 0; i < m_Items.Count(); i++)
    {
        m_Items[i]->MarkForDeletion();
    }
    m_Items.Purge();
}

void NewsListPanel::PerformLayout()
{
    BaseClass::PerformLayout();
    
    int wide, tall;
    GetSize(wide, tall);
    
    int itemHeight = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 24);
    int yOffset = 5;
    
    for (int i = 0; i < m_Items.Count(); i++)
    {
        if (m_Items[i]->IsVisible())
        {
            m_Items[i]->SetBounds(2, yOffset, wide - 4, itemHeight);
            yOffset += itemHeight + 2;
        }
    }
}

#include "tier0/memdbgoff.h"
