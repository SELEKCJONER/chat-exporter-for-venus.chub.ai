#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <sstream> 
#include <iomanip> 
#include <random>
#include <ctime>
#include <chrono>

#pragma comment(lib, "wininet.lib")

using ordered_json = nlohmann::basic_json<std::map>;

std::string escape_json(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= *c && *c <= '\x1f') {
                    o << "\\u"
                      << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
                } else {
                    o << *c;
                }
        }
    }
    return o.str();
}

std::string current_date_time() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    std::ostringstream oss;
    oss << 1900 + now_tm.tm_year << "-" 
        << std::setw(2) << std::setfill('0') << 1 + now_tm.tm_mon << "-" 
        << std::setw(2) << std::setfill('0') << now_tm.tm_mday << " @" 
        << std::setw(2) << std::setfill('0') << now_tm.tm_hour << "h " 
        << std::setw(2) << std::setfill('0') << now_tm.tm_min << "m " 
        << std::setw(2) << std::setfill('0') << now_tm.tm_sec << "s "
        << std::setw(3) << std::setfill('0') << millis << "ms";
    return oss.str();
}

int main() {
    HINTERNET hInternet, hConnect;
    DWORD bytesRead;

    hInternet = InternetOpen("User Agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        printf("InternetOpen failed (%d)\n", GetLastError());
        return 1;
    }

    std::string idczatu;
    std::cout << "IMPORTANT NOTES!!! Your chat MUST be public otherwise exporter wont be able to access it!\nChat id is that number at the end of link that appears in search bar when you are in chat on venus\n\nNow please input chat id\n" << std::endl;
    std::cin >> idczatu;

    std::string url = "https://api.characterhub.org/api/venus/chats/" + idczatu;

    hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        printf("InternetOpenUrl failed (%d)\n", GetLastError());
        return 1;
    }

    char buffer[4096];
    std::ofstream sourceFile("source.json");

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        sourceFile.write(buffer, bytesRead);
    }

    sourceFile.close();
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    std::ifstream inputFile("source.json");
    nlohmann::json wholeFileJson;
    inputFile >> wholeFileJson;
    inputFile.close();

    std::string botnejm;
    std::cout << "\nPlease input name of the character that you have been chatting with\n\n";
    std::cin >> botnejm;
    std::string juzernejm;
    std::cout << "\nPlease input the name that you were using in chat\n\n";
    std::cin >> juzernejm;
    
    std::string currentDateTime = current_date_time();

    nlohmann::json inputJson = wholeFileJson["chatMessages"];

    // Losowy generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> distr(1000, 10000);

    long long last_send_date = 1000000000000; // Startowy send_date

    std::string outputFileName = juzernejm + "'s chat with " + botnejm + ".jsonl";
    std::ofstream outputFile(outputFileName);

    outputFile << "{\"user_name\":\"" << juzernejm << "\",\"character_name\":\"" << botnejm << "\",\"create_date\":\"" << currentDateTime << "\",\"chat_metadata\":{\"note_prompt\":\"\",\"note_interval\":1,\"note_position\":1,\"note_depth\":4}}\n";

    for (const auto& item : inputJson) {
        last_send_date += distr(gen); // Losowy przyrost do send_date
        std::string outputString = "{\"name\":\"" + std::string(item["is_bot"].get<bool>() ? botnejm : juzernejm) +
            "\",\"is_user\":" + std::string(item["is_bot"].get<bool>() ? "false" : "true") +
            ",\"is_name\":" + std::string(item["is_main"].get<bool>() ? "true" : "false") +
            ",\"send_date\":" + std::to_string(last_send_date) + // Losowy send_date
            ",\"mes\":\"" + escape_json(item["message"].get<std::string>()) +
            "\",\"chid\":" + std::to_string(item["chat_id"].get<int>()) + "}";

        outputFile << outputString << '\n';
    }
    outputFile.close();
std::cout << "\nSuccess! now just import the chat file or paste it into chats directory in silly\n\n";
system("pause");
    return 0;
}
