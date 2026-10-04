#include "CliOptions.hpp"
#include <QRegularExpression>

namespace treasure::core {

int parseTabIdentifier(const QString& str) {
    bool ok = false;
    int idx = str.toInt(&ok);
    if (ok && idx >= 0 && idx <= 8) return idx;

    QString s = str.trimmed().toLower();
    if (s == "locator" || s == "treasure") return 0;
    if (s == "drawing" || s == "mapdrawing" || s == "draw") return 1;
    if (s == "annotations" || s == "annotation" || s == "annot") return 2;
    if (s == "artifact" || s == "artifacts" || s == "clues") return 3;
    if (s == "data" || s == "imported" || s == "mapdata") return 4;
    if (s == "sailing" || s == "routes" || s == "cluster") return 5;
    if (s == "livestock" || s == "granger" || s == "animals" || s == "breeding") return 6;
    if (s == "skills" || s == "skill" || s == "tracker") return 7;
    if (s == "tools" || s == "simulators" || s == "workbench" || s == "imp" || s == "bridge" || s == "grinder") return 8;

    return -1;
}

WindowLaunchOptions parseCommandLine(const QStringList& arguments,
                                     bool* helpOrVersionRequested,
                                     QString* errorMsg) {
    WindowLaunchOptions opts;
    if (helpOrVersionRequested) *helpOrVersionRequested = false;

    for (int i = 1; i < arguments.size(); ++i) {
        const QString& arg = arguments[i];

        if (arg == "--help" || arg == "-?" || arg == "--help-all" || arg == "--version" || arg == "-v") {
            if (helpOrVersionRequested) *helpOrVersionRequested = true;
            continue;
        }

        if (arg == "--fullscreen" || arg == "-f") {
            opts.fullscreen = true;
            continue;
        }

        if (arg == "--maximized" || arg == "-m") {
            opts.maximized = true;
            continue;
        }

        if (arg == "--half-screen") {
            opts.width = 1280;
            opts.height = 1440;
            opts.hasCustomSize = true;
            continue;
        }

        if ((arg == "--width" || arg == "-w") && i + 1 < arguments.size()) {
            bool ok = false;
            int val = arguments[++i].toInt(&ok);
            if (ok && val > 0) {
                opts.width = val;
                opts.hasCustomSize = true;
            } else if (errorMsg) {
                *errorMsg = QString("Invalid width: %1").arg(arguments[i]);
            }
            continue;
        }

        if ((arg == "--height" || arg == "-H") && i + 1 < arguments.size()) {
            bool ok = false;
            int val = arguments[++i].toInt(&ok);
            if (ok && val > 0) {
                opts.height = val;
                opts.hasCustomSize = true;
            } else if (errorMsg) {
                *errorMsg = QString("Invalid height: %1").arg(arguments[i]);
            }
            continue;
        }

        if ((arg == "--size" || arg == "-s") && i + 1 < arguments.size()) {
            QString sVal = arguments[++i].trimmed().toLower();
            if (sVal == "half") {
                opts.width = 1280;
                opts.height = 1440;
                opts.hasCustomSize = true;
            } else if (sVal == "full" || sVal == "fullscreen") {
                opts.fullscreen = true;
            } else {
                static const QRegularExpression sizeRe(R"(^(\d+)[xX,](\d+)$)");
                auto match = sizeRe.match(sVal);
                if (match.hasMatch()) {
                    opts.width = match.captured(1).toInt();
                    opts.height = match.captured(2).toInt();
                    opts.hasCustomSize = true;
                } else if (errorMsg) {
                    *errorMsg = QString("Invalid size format: %1. Expected WIDTHxHEIGHT (e.g. 1920x1080)").arg(sVal);
                }
            }
            continue;
        }

        if (arg == "--capture-docs") {
            opts.captureDocs = true;
            if (i + 1 < arguments.size() && !arguments[i + 1].startsWith("-")) {
                opts.captureDocsDir = arguments[++i];
            }
            continue;
        }

        if ((arg == "--tab" || arg == "-t") && i + 1 < arguments.size()) {
            int tab = parseTabIdentifier(arguments[++i]);
            if (tab >= 0) {
                opts.initialTab = tab;
            } else if (errorMsg) {
                *errorMsg = QString("Unknown tab: %1").arg(arguments[i]);
            }
            continue;
        }
    }

    return opts;
}

QString getCommandLineHelp() {
    return QString(R"(
WurmExplorer - Cartography, Sailing, Livestock & Workbench Suite for Wurm Online

Usage:
  wurm_explorer [options]

Window Sizing & Display Modes:
  -s, --size <WIDTHxHEIGHT|half|full>  Set window dimensions (e.g. 1920x1080, 2560x1440, 1280x720, half)
  -w, --width <pixels>                Set window width in pixels
  -H, --height <pixels>               Set window height in pixels
  -f, --fullscreen                    Launch in borderless fullscreen mode
  -m, --maximized                     Launch maximized
      --half-screen                   Shortcut for half-screen width (1280x1440)

Navigation & Automation:
  -t, --tab <id|name>                 Open directly to a tab (locator, drawing, annot, artifacts,
                                      data, sailing, livestock, skills, tools)
      --capture-docs [output_dir]     Run automated headless documentation screenshot suite and exit
                                      (default: assets/docs)

General Options:
  -h, --help                          Show this help message and exit
  -v, --version                       Display application version information
)");
}

} // namespace treasure::core
