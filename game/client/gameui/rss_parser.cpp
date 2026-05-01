#include "rss_parser.h"
#include "tier0/dbg.h"
#include <sstream>

RSSParser::RSSParser() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

RSSParser::~RSSParser() {
    curl_global_cleanup();
}

size_t RSSParser::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool RSSParser::FetchFeed(const char* url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        Msg("[RSS] Failed to initialize CURL\n");
        return false;
    }
    
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        Msg("[RSS] CURL error: %s\n", curl_easy_strerror(res));
        return false;
    }
    
    ParseFeed(readBuffer);
    return true;
}

std::string RSSParser::ExtractTag(const std::string& xml, const std::string& tag, size_t& pos) {
    std::string openTag = "<" + tag + ">";
    std::string closeTag = "</" + tag + ">";
    
    size_t start = xml.find(openTag, pos);
    if (start == std::string::npos) {
        return "";
    }
    
    start += openTag.length();
    size_t end = xml.find(closeTag, start);
    
    if (end == std::string::npos) {
        return "";
    }
    
    pos = end + closeTag.length();
    
    std::string content = xml.substr(start, end - start);
    
    size_t cdataStart = content.find("<![CDATA[");
    if (cdataStart != std::string::npos) {
        size_t cdataEnd = content.find("]]>");
        if (cdataEnd != std::string::npos) {
            content = content.substr(cdataStart + 9, cdataEnd - cdataStart - 9);
        }
    }
    
    return content;
}

void RSSParser::ParseFeed(const std::string& xmlData) {
    m_items.clear();
    
    // ОТЛАДКА: Выводим первые 1000 символов
    Msg("[RSS] === RAW XML START ===\n");
    if (xmlData.length() > 1000) {
        Msg("%s...\n", xmlData.substr(0, 1000).c_str());
    } else {
        Msg("%s\n", xmlData.c_str());
    }
    Msg("[RSS] === RAW XML END ===\n");
    Msg("[RSS] Total XML size: %d bytes\n\n", xmlData.length());
    
    // Ищем что есть в XML
    bool hasItem = (xmlData.find("<item>") != std::string::npos || xmlData.find("<item ") != std::string::npos);
    bool hasEntry = (xmlData.find("<entry>") != std::string::npos || xmlData.find("<entry ") != std::string::npos);
    
    Msg("[RSS] Found <item>: %s\n", hasItem ? "YES" : "NO");
    Msg("[RSS] Found <entry>: %s\n", hasEntry ? "YES" : "NO");
    
    if (hasEntry) {
        Msg("[RSS] Using Atom parser\n");
        ParseAtomFeed(xmlData);
    } else if (hasItem) {
        Msg("[RSS] Using RSS parser\n");
        ParseRSSFeed(xmlData);
    } else {
        Msg("[RSS] WARNING: No <item> or <entry> tags found!\n");
    }
    
    Msg("[RSS] Parsed %d items\n", m_items.size());
}

void RSSParser::ParseRSSFeed(const std::string& xmlData) {
    size_t itemPos = 0;
    
    while (true) {
        size_t itemStart = xmlData.find("<item>", itemPos);
        if (itemStart == std::string::npos) {
            itemStart = xmlData.find("<item ", itemPos);
        }
        if (itemStart == std::string::npos) {
            break;
        }
        
        size_t itemEnd = xmlData.find("</item>", itemStart);
        if (itemEnd == std::string::npos) {
            break;
        }
        
        itemEnd += 7;
        std::string itemBlock = xmlData.substr(itemStart, itemEnd - itemStart);
        
        RSSItem item;
        size_t tagPos = 0;
        
        item.title = ExtractTag(itemBlock, "title", tagPos);
        tagPos = 0;
        item.link = ExtractTag(itemBlock, "link", tagPos);
        tagPos = 0;
        item.description = ExtractTag(itemBlock, "description", tagPos);
        tagPos = 0;
        item.pubDate = ExtractTag(itemBlock, "pubDate", tagPos);
        
        m_items.push_back(item);
        itemPos = itemEnd;
    }
}

void RSSParser::ParseAtomFeed(const std::string& xmlData) {
    size_t entryPos = 0;
    
    while (true) {
        size_t entryStart = xmlData.find("<entry>", entryPos);
        if (entryStart == std::string::npos) {
            entryStart = xmlData.find("<entry ", entryPos);
        }
        if (entryStart == std::string::npos) {
            break;
        }
        
        size_t entryEnd = xmlData.find("</entry>", entryStart);
        if (entryEnd == std::string::npos) {
            break;
        }
        
        entryEnd += 8;
        std::string entryBlock = xmlData.substr(entryStart, entryEnd - entryStart);
        
        RSSItem item;
        size_t tagPos = 0;
        
        item.title = ExtractTag(entryBlock, "title", tagPos);
        tagPos = 0;
        item.link = ExtractAtomLink(entryBlock);
        tagPos = 0;
        item.description = ExtractTag(entryBlock, "summary", tagPos);
        if (item.description.empty()) {
            tagPos = 0;
            item.description = ExtractTag(entryBlock, "content", tagPos);
        }
        tagPos = 0;
        item.pubDate = ExtractTag(entryBlock, "updated", tagPos);
        if (item.pubDate.empty()) {
            tagPos = 0;
            item.pubDate = ExtractTag(entryBlock, "published", tagPos);
        }
        
        m_items.push_back(item);
        entryPos = entryEnd;
    }
}

std::string RSSParser::ExtractAtomLink(const std::string& entryBlock) {
    size_t linkStart = entryBlock.find("<link");
    if (linkStart == std::string::npos) {
        return "";
    }
    
    size_t hrefStart = entryBlock.find("href=\"", linkStart);
    if (hrefStart == std::string::npos) {
        hrefStart = entryBlock.find("href='", linkStart);
        if (hrefStart == std::string::npos) {
            return "";
        }
        hrefStart += 6;
    } else {
        hrefStart += 6;
    }
    
    size_t hrefEnd = entryBlock.find("\"", hrefStart);
    if (hrefEnd == std::string::npos) {
        hrefEnd = entryBlock.find("'", hrefStart);
    }
    
    if (hrefEnd == std::string::npos) {
        return "";
    }
    
    return entryBlock.substr(hrefStart, hrefEnd - hrefStart);
}

void RSSParser::PrintToConsole() {
    Msg("=== RSS Feed ===\n");
    Msg("Total items: %d\n\n", m_items.size());
    
    for (size_t i = 0; i < m_items.size(); i++) {
        Msg("--- Item %d ---\n", i + 1);
        Msg("Title: %s\n", m_items[i].title.c_str());
        Msg("Link: %s\n", m_items[i].link.c_str());
        Msg("Date: %s\n", m_items[i].pubDate.c_str());
        
        std::string desc = m_items[i].description;
        if (desc.length() > 200) {
            desc = desc.substr(0, 197) + "...";
        }
        Msg("Description: %s\n", desc.c_str());
        Msg("\n");
    }
}