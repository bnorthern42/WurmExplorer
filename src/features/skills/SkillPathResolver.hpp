#pragma once

#include <string>
#include <vector>

namespace skills {

class SkillPathResolver {
public:
    static std::string getDefaultPlayersDir();
    static std::vector<std::string> getAvailablePlayers(const std::string& playersDir);
    static std::string getLogsDir(const std::string& playersDir, const std::string& playerName);
    static std::string getMonthLogFilename(int year, int month);
    static std::string getCurrentMonthLogFilename();
    static std::string resolveCurrentLogPath(const std::string& playersDir, const std::string& playerName);
    static std::vector<std::string> getAvailableLogFiles(const std::string& logsDir);
};

} // namespace skills
