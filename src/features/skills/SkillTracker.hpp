#pragma once

#include "SkillLogParser.hpp"
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace skills {

struct SingleGain {
    int64_t time = 0; // absolute seconds
    double inc = 0.0;
};

struct SkillStats {
    std::string skill_name;
    double current_level = 0.0;
    double total_gain = 0.0;
    double past_15m_gain = 0.0;
    double past_60m_gain = 0.0;
    double rate_per_hour = 0.0;
    int gain_count = 0;
    int64_t first_time = 0;
    int64_t last_time = 0;
};

struct InternalSkillData {
    std::vector<SingleGain> gains;
    double total_gain = 0.0;
    double current_level = 0.0;
    int64_t first_time = -1;
    int64_t last_time = -1;
};

class SkillTracker {
public:
    SkillTracker();

    void reset();
    void processLine(const std::string& line);

    std::vector<SkillStats> getStats() const;
    const SkillStats* getSkillStats(const std::string& skillName) const;
    int64_t getGlobalLastTime() const { return m_globalLastTime; }

private:
    void recomputeStats() const;

    int64_t m_globalLastTime = 0;
    int64_t m_currentDayOffset = 0;
    int64_t m_lastRawTimeSec = -1;

    std::map<std::string, InternalSkillData> m_skills;
    mutable std::vector<SkillStats> m_cachedStats;
    mutable std::map<std::string, SkillStats> m_cachedMap;
    mutable bool m_dirty = false;
};

} // namespace skills
