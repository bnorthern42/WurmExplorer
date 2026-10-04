#pragma once

#include <QString>
#include <QStringList>

namespace treasure::core {

struct WindowLaunchOptions {
    int width = 2560;
    int height = 1440;
    bool hasCustomSize = false;
    bool fullscreen = false;
    bool maximized = false;
    bool captureDocs = false;
    QString captureDocsDir = "assets/docs";
    int initialTab = -1; // -1 = default / saved tab
};

WindowLaunchOptions parseCommandLine(const QStringList& arguments,
                                     bool* helpOrVersionRequested = nullptr,
                                     QString* errorMsg = nullptr);

QString getCommandLineHelp();

} // namespace treasure::core
