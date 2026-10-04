#include "SkillTracker.hpp"
#include <algorithm>

namespace skills {

SkillTracker::SkillTracker(QObject* parent)
    : QObject(parent) {
    reset();
}

SkillTracker& SkillTracker::instance() {
    static SkillTracker s_instance;
    return s_instance;
}

void SkillTracker::reset() {
    m_skills.clear();
    m_cachedStats.clear();
    m_cachedMap.clear();
    m_globalLastTime = 0;
    m_currentDayOffset = 0;
    m_lastRawTimeSec = -1;
    m_dirty = false;
}

void SkillTracker::processLine(const std::string& line) {
    auto entryOpt = SkillLogParser::parseLine(line);
    if (!entryOpt) return;

    const auto& entry = *entryOpt;
    int64_t raw_time_sec = entry.seconds_of_day;

    // Midnight rollover: transition from late night (>= 12:00 PM / 43200s, typically >= 20:00) to early morning (< 12:00 PM)
    if (m_lastRawTimeSec >= 43200 && raw_time_sec < 43200) {
        m_currentDayOffset += 86400;
    }
    m_lastRawTimeSec = raw_time_sec;

    int64_t absolute_time = m_currentDayOffset + raw_time_sec;


    auto& data = m_skills[entry.skill_name];
    data.gains.push_back({absolute_time, entry.gain});
    data.total_gain += entry.gain;
    data.current_level = entry.level;
    if (data.first_time < 0 || absolute_time < data.first_time) {
        data.first_time = absolute_time;
    }
    if (data.last_time < 0 || absolute_time > data.last_time) {
        data.last_time = absolute_time;
    }

    if (absolute_time > m_globalLastTime) {
        m_globalLastTime = absolute_time;
    }

    m_dirty = true;
    emit skillUpdated(QString::fromStdString(entry.skill_name), entry.level);
}

void SkillTracker::recomputeStats() const {
    m_cachedStats.clear();
    m_cachedMap.clear();

    for (const auto& [name, data] : m_skills) {
        double past_15 = 0.0;
        double past_60 = 0.0;

        for (const auto& g : data.gains) {
            if (g.time >= m_globalLastTime - 900) {
                past_15 += g.inc;
            }
            if (g.time >= m_globalLastTime - 3600) {
                past_60 += g.inc;
            }
        }

        int64_t time_diff = (data.last_time >= 0 && data.first_time >= 0) ? (data.last_time - data.first_time) : 0;
        double hours = time_diff > 0 ? (static_cast<double>(time_diff) / 3600.0) : 0.0;
        double rate = hours > 0 ? (data.total_gain / hours) : 0.0;

        SkillStats s;
        s.skill_name = name;
        s.current_level = data.current_level;
        s.total_gain = data.total_gain;
        s.past_15m_gain = past_15;
        s.past_60m_gain = past_60;
        s.rate_per_hour = rate;
        s.gain_count = static_cast<int>(data.gains.size());
        s.first_time = data.first_time;
        s.last_time = data.last_time;

        m_cachedStats.push_back(s);
        m_cachedMap[name] = s;
    }

    std::sort(m_cachedStats.begin(), m_cachedStats.end(), [](const SkillStats& a, const SkillStats& b) {
        return a.total_gain > b.total_gain;
    });

    m_dirty = false;
}

std::vector<SkillStats> SkillTracker::getStats() const {
    if (m_dirty) {
        recomputeStats();
    }
    return m_cachedStats;
}

const SkillStats* SkillTracker::getSkillStats(const std::string& skillName) const {
    if (m_dirty) {
        recomputeStats();
    }
    auto it = m_cachedMap.find(skillName);
    if (it != m_cachedMap.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace skills
