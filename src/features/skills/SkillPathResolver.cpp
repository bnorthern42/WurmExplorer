#include "SkillPathResolver.hpp"
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace skills {

std::string SkillPathResolver::getDefaultPlayersDir() {
    const char* home = std::getenv("HOME");
    if (!home) return "";
    return std::string(home) + "/.config/wurm/players";
}

std::vector<std::string> SkillPathResolver::getAvailablePlayers(const std::string& playersDir) {
    std::vector<std::string> players;
    if (!std::filesystem::exists(playersDir) || !std::filesystem::is_directory(playersDir)) {
        return players;
    }

    for (const auto& entry : std::filesystem::directory_iterator(playersDir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (!name.empty() && name[0] != '.') {
                players.push_back(name);
            }
        }
    }
    std::sort(players.begin(), players.end());
    return players;
}

std::string SkillPathResolver::getLogsDir(const std::string& playersDir, const std::string& playerName) {
    return playersDir + "/" + playerName + "/logs";
}

std::string SkillPathResolver::getMonthLogFilename(int year, int month) {
    std::ostringstream ss;
    ss << "_Skills." << year << "-" << std::setw(2) << std::setfill('0') << month << ".txt";
    return ss.str();
}

std::string SkillPathResolver::getCurrentMonthLogFilename() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    return getMonthLogFilename(now->tm_year + 1900, now->tm_mon + 1);
}

std::string SkillPathResolver::resolveCurrentLogPath(const std::string& playersDir, const std::string& playerName) {
    return getLogsDir(playersDir, playerName) + "/" + getCurrentMonthLogFilename();
}

std::vector<std::string> SkillPathResolver::getAvailableLogFiles(const std::string& logsDir) {
    std::vector<std::string> files;
    if (!std::filesystem::exists(logsDir) || !std::filesystem::is_directory(logsDir)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(logsDir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            bool hasPrefix = (filename.rfind("_Skills.", 0) == 0);
            bool hasSuffix = (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".txt") == 0);
            if (hasPrefix && hasSuffix) {
                files.push_back(filename);
            }


        }
    }

    // Sort descending so newest is first
    std::sort(files.begin(), files.end(), std::greater<std::string>());
    return files;
}

} // namespace skills
