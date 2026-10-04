#include "SkillLogParser.hpp"
#include <regex>

namespace skills {

int64_t SkillLogParser::parseTimeToSeconds(const std::string& time_str) {
    if (time_str.size() < 8) return 0;
    try {
        int h = std::stoi(time_str.substr(0, 2));
        int m = std::stoi(time_str.substr(3, 2));
        int s = std::stoi(time_str.substr(6, 2));
        return static_cast<int64_t>(h) * 3600 + static_cast<int64_t>(m) * 60 + s;
    } catch (...) {
        return 0;
    }
}

std::optional<SkillLogEntry> SkillLogParser::parseLine(const std::string& line) {
    // Regex matching: [HH:MM:SS] <skill_name> increased by <inc> to <level>
    static const std::regex skillRegex(R"(^\[(\d{1,2}:\d{2}:\d{2})\]\s+(.+?)\s+increased by\s+([0-9.]+)\s+to\s+([0-9.]+)\s*$)");
    std::smatch match;

    if (std::regex_match(line, match, skillRegex)) {
        try {
            SkillLogEntry entry;
            entry.time_str = match[1].str();
            entry.seconds_of_day = parseTimeToSeconds(entry.time_str);
            entry.skill_name = match[2].str();
            entry.gain = std::stod(match[3].str());
            entry.level = std::stod(match[4].str());
            return entry;
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

} // namespace skills
