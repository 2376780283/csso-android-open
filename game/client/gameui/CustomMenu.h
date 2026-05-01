//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Custom menu components for the main menu
//
//=============================================================================

#ifndef CUSTOMMENU_H
#define CUSTOMMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "utlvector.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/MaterialSystemUtil.h"

#include <materialsystem/itexture.h>
#include <materialsystem/imaterialvar.h>

class CBaseModPanel;

// ====================================
// ImageButton - 图片按钮
// ====================================
class ImageButton : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(ImageButton, vgui::Panel);
public:
    ImageButton(vgui::Panel *parent, CBaseModPanel* pBasePanel, const char *name, const char *imagePath);
    
    typedef void (CBaseModPanel::*PanelActionFunc_t)();
    void SetActions(PanelActionFunc_t defaultAction, PanelActionFunc_t markedAction = NULL);
    void SetMarkedInfo(const char *imagePath);
    void SetMarked(bool bMarked);
    
protected:
    virtual void Paint() override;
    virtual void OnMousePressed(vgui::MouseCode code) override;
    virtual void OnMouseReleased(vgui::MouseCode code) override;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;

private:
    CBaseModPanel* m_pBasePanel;
    PanelActionFunc_t m_pDefaultAction;
    PanelActionFunc_t m_pMarkedAction;
    
    bool m_bSelected;
    bool m_bIsMarked;
    int m_textureID;
    int m_textureID_Marked;
};

// ====================================
// CGUIBlurHelper - 模糊背景辅助类
// ====================================
class CGUIBlurHelper
{
public:
    CGUIBlurHelper();
    void DrawBlur(vgui::Panel *pPanel, float fDarkness = 160.0f);

private:
    void InitResources();

    CTextureReference   m_CaptureRT;
    CTextureReference   m_HelperRT;
    CMaterialReference  m_BlurX;
    CMaterialReference  m_BlurY;
    bool                m_bInitialized;
};

// ====================================
// NvgBarPanel - 导航栏背景面板
// ====================================
class NvgBarPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(NvgBarPanel, vgui::Panel);
public:
    NvgBarPanel(vgui::Panel *parent, const char *name);
    
protected:
    virtual void PaintBackground() override;

public:
    void UpdateLayout();
    
private:
    CGUIBlurHelper m_BlurHelper;
};

// ====================================
// NewsListItem - 新闻列表项
// ====================================
class NewsListItem : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(NewsListItem, vgui::Panel);
public:
    NewsListItem(vgui::Panel *parent, const char *name);
    
    void SetNewsInfo(const char *iconPath, const wchar_t *text);
    void SetNewsInfo(const char *iconPath, const char *text);
    
protected:
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void PerformLayout() override;
    
private:
    vgui::ImagePanel *m_pIcon;
    vgui::Label *m_pText;
    int m_iTextureID;
};

// ====================================
// NewsListPanel - 新闻列表面板
// ====================================
class NewsListPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(NewsListPanel, vgui::Panel);
public:
    NewsListPanel(vgui::Panel *parent, const char *name);
    
    void AddNewsItem(const char *iconPath, const wchar_t *text);
    void AddNewsItem(const char *iconPath, const char *text);
    void ClearItems();
    
protected:
    virtual void PerformLayout() override;
    
private:
    CUtlVector<NewsListItem*> m_Items;
};

#endif // CUSTOMMENU_H
