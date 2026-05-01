#pragma once
#include <string>
#include <vector>
#include <curl/curl.h>

struct RSSItem {
    std::string title;
    std::string link;
    std::string description;
    std::string pubDate;
};

class RSSParser {
public:
    RSSParser();
    ~RSSParser();
    
    bool FetchFeed(const char* url);
    void ParseFeed(const std::string& xmlData);
    void PrintToConsole();
    const std::vector<RSSItem>& GetItems() const { return m_items; }
    
private:
    std::vector<RSSItem> m_items;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string ExtractTag(const std::string& xml, const std::string& tag, size_t& pos);
    void ParseRSSFeed(const std::string& xmlData);
    void ParseAtomFeed(const std::string& xmlData);
    std::string ExtractAtomLink(const std::string& entryBlock);
};