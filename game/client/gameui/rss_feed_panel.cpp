#include "rss_feed_panel.h"
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/ListPanel.h>
#include <KeyValues.h>
#include "tier0/dbg.h"

using namespace vgui;

#define RSS_FEED_URL "http://fetchrss.com/feed/1vnNWIFClA0w1vnNVtBy6134.rss"

RSSFeedPanel::RSSFeedPanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name) {
    SetProportional(false);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(true);
    
    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(true);
    
    m_pRSSParser = new RSSParser();
    m_bLoaded = false;
    
    m_pNewsList = new ListPanel(this, "NewsList");
    m_pNewsList->SetMultiselectEnabled(false);
    m_pNewsList->AddColumnHeader(0, "title", "CS:SO Plus News", 280, ListPanel::COLUMN_RESIZEWITHWINDOW);
    m_pNewsList->AddColumnHeader(1, "date", "Date", 120, ListPanel::COLUMN_FIXEDSIZE);
    m_pNewsList->SetSortFunc(0, NULL);
    m_pNewsList->SetSortFunc(1, NULL);
    m_pNewsList->SetSortColumn(-1);
    m_pNewsList->SetEmptyListText("Loading news...");
    
    LoadControlSettings("Resource/UI/RSSFeedPanel.res");
    
    ivgui()->AddTickSignal(GetVPanel(), 100);
    SetVisible(true);
}

RSSFeedPanel::~RSSFeedPanel() {
    if (m_pRSSParser) {
        delete m_pRSSParser;
        m_pRSSParser = NULL;
    }
}

void RSSFeedPanel::ApplySchemeSettings(IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    
    SetBgColor(pScheme->GetColor("Panel.BgColor", Color(0, 0, 0, 180)));
    SetBorder(pScheme->GetBorder("FrameBorder"));
}

void RSSFeedPanel::PerformLayout() {
    BaseClass::PerformLayout();
    
    int x, y, wide, tall;
    m_pNewsList->GetBounds(x, y, wide, tall);
    Msg("[RSS] NewsList from .res: x=%d, y=%d, wide=%d, tall=%d\n", x, y, wide, tall);
}

void RSSFeedPanel::OnThink() {
    BaseClass::OnThink();
    
    if (!m_bLoaded) {
        LoadAndPopulateFeed();
        m_bLoaded = true;
    }
}

void RSSFeedPanel::LoadAndPopulateFeed() {
    if (!m_pRSSParser || !m_pNewsList) {
        return;
    }
    
    Msg("[RSS Panel] Fetching feed from: %s\n", RSS_FEED_URL);
    
    if (!m_pRSSParser->FetchFeed(RSS_FEED_URL)) {
        m_pNewsList->SetEmptyListText("Failed to load feed");
        Msg("[RSS Panel] Failed to fetch feed\n");
        return;
    }
    
    const std::vector<RSSItem>& items = m_pRSSParser->GetItems();
    
    Msg("[RSS Panel] Loaded %d items\n", items.size());
    
    if (items.empty()) {
        m_pNewsList->SetEmptyListText("No news available");
        return;
    }
    
    m_pNewsList->RemoveAll();
    
    for (size_t i = 0; i < items.size(); i++) {
        KeyValues *kv = new KeyValues("item");
        
        std::string title;
        if (items[i].title.empty()) {
            title = items[i].description;
            size_t start = 0;
            while ((start = title.find("<", start)) != std::string::npos) {
                size_t end = title.find(">", start);
                if (end != std::string::npos) {
                    title.erase(start, end - start + 1);
                } else {
                    break;
                }
            }
            if (title.length() > 60) {
                title = title.substr(0, 57) + "...";
            }
        } else {
            title = items[i].title;
        }
        
        kv->SetString("title", title.c_str());
        
        std::string date = items[i].pubDate;
        if (date.length() > 16) {
            date = date.substr(0, 16);
        }
        kv->SetString("date", date.c_str());
        
        kv->SetString("link", items[i].link.c_str());
        
        m_pNewsList->AddItem(kv, 0, false, false);
        kv->deleteThis();
    }
    
    Msg("[RSS Panel] Feed loaded successfully\n");
}