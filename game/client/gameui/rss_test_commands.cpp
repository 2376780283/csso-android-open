#include "convar.h"
#include "rss_parser.h"

static RSSParser g_RSSParser;

CON_COMMAND(rss_fetch, "Fetch RSS feed from URL") {
    if (args.ArgC() < 2) {
        Msg("Usage: rss_fetch <url>\n");
        Msg("Example: rss_fetch https://news.ycombinator.com/rss\n");
        return;
    }
    
    const char* urlArg = args.ArgS();
    std::string url = urlArg;
    
    // Автоматически добавляем https:// если протокол не указан
    if (url.find("://") == std::string::npos) {
        url = "http://" + url;
    }
    
    Msg("[RSS] Fetching feed from: %s\n", url.c_str());
    
    if (g_RSSParser.FetchFeed(url.c_str())) {
        g_RSSParser.PrintToConsole();
    } else {
        Msg("[RSS] Failed to fetch feed\n");
    }
}

CON_COMMAND(rss_print, "Print last fetched RSS feed") {
    g_RSSParser.PrintToConsole();
}