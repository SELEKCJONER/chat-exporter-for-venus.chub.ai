#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <sstream> 
#include <iomanip> 
#include <ctime>
#include <chrono>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

using ordered_json = nlohmann::basic_json<std::map>;
std::string replace_string(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

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
	int tavern_wersja;
    int idczatu;
    std::cout << "IMPORTANT NOTES!!! Your chat MUST be public otherwise exporter wont be able to access it\nthrough official www.chub.ai/ api endpoint! \n\nChat id is that number at the end of link that appears in search bar when you are in chat on venus\n\nNow please input the chat id\n\n";
    std::cin >> idczatu;

    HINTERNET hInternet, hConnect;
    DWORD bytesRead;

    hInternet = InternetOpen("User Agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        printf("InternetOpen failed (%d)\n", GetLastError());
        return 1;
    }

    std::string url = "https://api.chub.ai/api/venus/chats/" + std::to_string(idczatu); //using public api endpoint

    std::string botnejm;
    std::cout << "\nPlease input name of the character that you were chatting with\n\n";
    std::cin >> botnejm;
    std::string juzernejm;
    std::cout << "\nPlease input the name that you want to use in this chat\n\n";
    std::cin >> juzernejm;
	std::cout << "\n\nPlease specify tavern version\nType 1 for normal Tavern experimental i cant guarantee that it will work\n\nType anything else for silly tavern\n\n";
	std::cin >> tavern_wersja;
    std::string currentDateTime = current_date_time();

    std::multimap<int, nlohmann::json> sortedInputJson;
    int lowestMessageId = INT_MAX;
    ordered_json previousResponse;

    while (true) {
        hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect == NULL)
        {
            printf("InternetOpenUrl failed (%d)\n", GetLastError());
			std::cout << "something went wrong if u are 100% sure u typed id correctly try to disable firewall or run this program as admin and\nif this does not help well contat me pls dont spam\n\n\n discord selekcjoner."; 
            return 1;
        }

        std::ofstream sourceFile("source.txt");
        char buffer[4096];

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        {
            sourceFile.write(buffer, bytesRead);
        }
        sourceFile.close();
        InternetCloseHandle(hConnect);

        std::ifstream sourceFileToRead("source.txt");
        std::stringstream ss;
        ss << sourceFileToRead.rdbuf();
        sourceFileToRead.close();

        ordered_json wholeFileJson = ordered_json::parse(ss.str());
        ss.str(std::string());

        if (wholeFileJson == previousResponse || wholeFileJson["chatMessages"].empty())
        {
            break;
        }

        previousResponse = wholeFileJson;
        nlohmann::json inputJson = wholeFileJson["chatMessages"];

        for (const auto& item : inputJson) {
            sortedInputJson.insert({item["id"].get<int>(), item});
            if (item["id"].get<int>() < lowestMessageId)
            {
                lowestMessageId = item["id"].get<int>();
            }
        }

        url = "https://api.chub.ai/api/venus/chats/" + std::to_string(idczatu) + "/" + std::to_string(lowestMessageId); //why just why make pagination only isnt there a way to keep paginated response api and full chat api ?
    }

    std::string outputFileName = juzernejm + "'s chat with " + botnejm + ".jsonl";
    std::ofstream outputFile(outputFileName);
	if (tavern_wersja == 1)
	{
		outputFile << "{\"user_name\":\"" << juzernejm << "\",\"character_name\":\"" << botnejm << "\",\"create_date\":000000000001,\"notes\":\"\",\"notes_type\":\"discr\",\"mode\":\"chat\"}\n";
		long long last_send_date = 1000000000000; // Starting send_date

    for (const auto& pair : sortedInputJson) {
        const auto& item = pair.second;
        if (item["is_main"].get<bool>()) {  // Only messages with is_main == true
            last_send_date++; // Increment send_date by 1 for each message

            // Replace {{char}} and {{user}} with botnejm and juzernejm
            std::string message = replace_string(item["message"].get<std::string>(), "{{char}}", botnejm);
            message = replace_string(message, "{{user}}", juzernejm);

            std::string outputString = "{\"name\":\"" + std::string(item["is_bot"].get<bool>() ? botnejm : juzernejm) +
                "\",\"is_user\":" + std::string(item["is_bot"].get<bool>() ? "false" : "true") +
                ",\"is_name\":" + std::string(item["is_main"].get<bool>() ? "true" : "false") +
                ",\"send_date\":" + std::to_string(last_send_date) +
                ",\"mes\":\"" + escape_json(message) +
                "\",\"chid\":" + std::to_string(item["chat_id"].get<int>()) + "}";

            outputFile << outputString << '\n';
        }
    }

    outputFile.close();
    InternetCloseHandle(hInternet);

    std::cout << "\nSuccess! now just import the chat file or paste it into chats directory in Tavern\n";
    system("pause");
    return 0;
		
	}
	else if (tavern_wersja != 1)
	{
    outputFile << "{\"user_name\":\"" << juzernejm << "\",\"character_name\":\"" << botnejm << "\",\"create_date\":\"" << currentDateTime << "\",\"chat_metadata\":{\"note_prompt\":\"\",\"note_interval\":1,\"note_position\":1,\"note_depth\":4}}\n";

    long long last_send_date = 1000000000000; // Starting send_date

    for (const auto& pair : sortedInputJson) {
        const auto& item = pair.second;
        if (item["is_main"].get<bool>()) {  // Only messages with is_main == true
            last_send_date++; // Increment send_date by 1 for each message

            // Replace {{char}} and {{user}} with botnejm and juzernejm
            std::string message = replace_string(item["message"].get<std::string>(), "{{char}}", botnejm);
            message = replace_string(message, "{{user}}", juzernejm);

            std::string outputString = "{\"name\":\"" + std::string(item["is_bot"].get<bool>() ? botnejm : juzernejm) +
                "\",\"is_user\":" + std::string(item["is_bot"].get<bool>() ? "false" : "true") +
                ",\"is_name\":" + std::string(item["is_main"].get<bool>() ? "true" : "false") +
                ",\"send_date\":" + std::to_string(last_send_date) +
                ",\"mes\":\"" + escape_json(message) +
                "\",\"chid\":" + std::to_string(item["chat_id"].get<int>()) + "}";

            outputFile << outputString << '\n';
        }
    }

    outputFile.close();
    InternetCloseHandle(hInternet);

    std::cout << "\nSuccess! now just import the chat file or paste it into chats directory in Silly Tavern\n";
    system("pause");
    return 0;
	}
}