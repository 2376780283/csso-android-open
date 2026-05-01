#ifndef VGUI_SKIN_EDITOR_H
#define VGUI_SKIN_EDITOR_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

namespace vgui
{
    class Button;
    class ComboBox;
    class CInventoryPanel;
}

class CSkinEditorPanel : public vgui::PropertyPage
{
    DECLARE_CLASS_SIMPLE(CSkinEditorPanel, vgui::PropertyPage);

public:
    CSkinEditorPanel(vgui::Panel* parent);
    ~CSkinEditorPanel();

    virtual void OnThink();
    virtual void PerformLayout();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void OnCommand(const char *command);
    
    MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
    MESSAGE_FUNC_PARAMS(OnSkinSelected, "SkinSelected", data);

public:
    vgui::CInventoryPanel *m_pInventoryPanel;
    vgui::Button *m_pShowAllButton;
    vgui::ComboBox *m_pWeaponFilter;
    vgui::ComboBox *m_pRarityFilter;

private:
    void CreateControls();
    bool m_bControlsCreated;
};

extern CSkinEditorPanel* g_pSkinEditor;

#endif // VGUI_SKIN_EDITOR_H
