#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include "../src/features/skills/SkillLogParser.hpp"
#include "../src/features/skills/SkillTracker.hpp"
#include "../src/features/skills/SkillPathResolver.hpp"

using namespace skills;

bool approxEqual(double a, double b, double eps = 0.00001) {
    return std::fabs(a - b) < eps;
}

void testSkillLogParser() {
    std::string line1 = "[21:12:15] Repairing increased by 0.0002 to 66.8953";
    auto entry1 = SkillLogParser::parseLine(line1);
    assert(entry1.has_value());
    assert(entry1->time_str == "21:12:15");
    assert(entry1->skill_name == "Repairing");
    assert(approxEqual(entry1->gain, 0.0002));
    assert(approxEqual(entry1->level, 66.8953));

    std::string line2 = "[21:12:16] Mind logic increased by 0.000008 to 67.319145";
    auto entry2 = SkillLogParser::parseLine(line2);
    assert(entry2.has_value());
    assert(entry2->time_str == "21:12:16");
    assert(entry2->skill_name == "Mind logic");
    assert(approxEqual(entry2->gain, 0.000008));
    assert(approxEqual(entry2->level, 67.319145));

    // Non-skill log lines should return nullopt
    std::string line3 = "[21:12:16] You start repairing the item.";
    auto entry3 = SkillLogParser::parseLine(line3);
    assert(!entry3.has_value());

    std::string line4 = "Some random chatter in local chat";
    auto entry4 = SkillLogParser::parseLine(line4);
    assert(!entry4.has_value());

    std::cout << "testSkillLogParser PASSED\n";
}

void testSkillTrackerStats() {
    SkillTracker tracker;

    tracker.processLine("[10:00:00] Carpentry increased by 0.0100 to 50.0100");
    tracker.processLine("[10:10:00] Carpentry increased by 0.0200 to 50.0300");
    tracker.processLine("[10:50:00] Carpentry increased by 0.0150 to 50.0450");
    tracker.processLine("[11:00:00] Carpentry increased by 0.0050 to 50.0500");

    // Also an unrelated skill
    tracker.processLine("[10:30:00] Blacksmithing increased by 0.0050 to 40.0050");

    auto stats = tracker.getStats();
    assert(stats.size() == 2);

    const auto* carp = tracker.getSkillStats("Carpentry");
    assert(carp != nullptr);
    assert(approxEqual(carp->total_gain, 0.0500));
    assert(approxEqual(carp->current_level, 50.0500));
    assert(carp->gain_count == 4);

    // Global last is 11:00:00 (sec = 39600)
    // Past 15m is from 10:45:00 onwards -> lines at 10:50 (0.0150) and 11:00 (0.0050) = 0.0200
    assert(approxEqual(carp->past_15m_gain, 0.0200));

    // Past 60m is from 10:00:00 onwards -> all 4 gains = 0.0500
    assert(approxEqual(carp->past_60m_gain, 0.0500));

    // Elapsed time from 10:00:00 to 11:00:00 is 1 hour (3600 sec). Rate = 0.0500 / 1.0 = 0.0500 / hr
    assert(approxEqual(carp->rate_per_hour, 0.0500));

    // Sorted descending by total_gain
    assert(stats[0].skill_name == "Carpentry");
    assert(stats[1].skill_name == "Blacksmithing");

    std::cout << "testSkillTrackerStats PASSED\n";
}

void testMidnightRollover() {
    SkillTracker tracker;

    tracker.processLine("[23:59:30] Digging increased by 0.0100 to 30.0100");
    tracker.processLine("[00:01:00] Digging increased by 0.0100 to 30.0200");

    const auto* dig = tracker.getSkillStats("Digging");
    assert(dig != nullptr);
    assert(approxEqual(dig->total_gain, 0.0200));

    // Time difference should be 90 seconds (1.5 minutes = 0.025 hours)
    int64_t diff = dig->last_time - dig->first_time;
    assert(diff == 90);

    // Rate = 0.0200 / (90 / 3600.0) = 0.0200 / 0.025 = 0.8
    assert(approxEqual(dig->rate_per_hour, 0.8));

    std::cout << "testMidnightRollover PASSED\n";
}

void testSkillPathResolver() {
    std::string defaultPath = SkillPathResolver::getDefaultPlayersDir();
    assert(!defaultPath.empty());

    // Test resolving logs dir
    std::string logsDir = SkillPathResolver::getLogsDir("/mock/wurm/players", "polarbear");
    assert(logsDir == "/mock/wurm/players/polarbear/logs");

    // Test month pattern
    std::string monthLog = SkillPathResolver::getMonthLogFilename(2026, 10);
    assert(monthLog == "_Skills.2026-10.txt");

    // Test finding polarbear in actual ~/.config/wurm/players if directory exists
    std::string realPlayersDir = SkillPathResolver::getDefaultPlayersDir();
    if (std::filesystem::exists(realPlayersDir)) {
        auto players = SkillPathResolver::getAvailablePlayers(realPlayersDir);
        assert(std::find(players.begin(), players.end(), "polarbear") != players.end());

        std::string polarbearLogs = SkillPathResolver::getLogsDir(realPlayersDir, "polarbear");
        assert(std::filesystem::exists(polarbearLogs));
    }

    std::cout << "testSkillPathResolver PASSED\n";
}

int main() {
    std::cout << "Running Skills Tests...\n";
    testSkillLogParser();
    testSkillTrackerStats();
    testMidnightRollover();
    testSkillPathResolver();
    std::cout << "All Skills tests passed!\n";
    return 0;
}
