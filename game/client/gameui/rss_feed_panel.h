#pragma once

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ListPanel.h>
#include "rss_parser.h"

namespace vgui {
    class ListPanel;
}

class RSSFeedPanel : public vgui::EditablePanel {
    DECLARE_CLASS_SIMPLE(RSSFeedPanel, vgui::EditablePanel);

public:
    RSSFeedPanel(vgui::Panel *parent, const char *name);
    virtual ~RSSFeedPanel();

protected:
    virtual void PerformLayout();
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void OnThink();

private:
    void LoadAndPopulateFeed();
    
    vgui::ListPanel *m_pNewsList;
    RSSParser *m_pRSSParser;
    bool m_bLoaded;
};