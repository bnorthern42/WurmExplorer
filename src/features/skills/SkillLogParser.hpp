#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace skills {

struct SkillLogEntry {
    std::string time_str; // "HH:MM:SS"
    int64_t seconds_of_day = 0;
    std::string skill_name;
    double gain = 0.0;
    double level = 0.0;
};

class SkillLogParser {
public:
    static std::optional<SkillLogEntry> parseLine(const std::string& line);
    static int64_t parseTimeToSeconds(const std::string& time_str);
};

} // namespace skills
