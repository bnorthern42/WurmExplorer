#include <QApplication>
#include <QIcon>
#include <QFontDatabase>
#include <QFile>
#include <iostream>
#include "core/PathUtils.hpp"
#include "core/CliOptions.hpp"
#include "ui/MainWindow.hpp"
#include "ui/Theme.hpp"
#pragma push_macro("signals")
#undef signals
#include <vips/vips8>
#pragma pop_macro("signals")

int main(int argc, char *argv[]) {
    if (VIPS_INIT(argv[0])) {
        vips_error_exit(NULL);
    }

    QApplication app(argc, argv);
    
    // Set up application metadata
    QApplication::setApplicationName("WurmExplorer");
    QApplication::setOrganizationName("WurmMods");
    QApplication::setApplicationVersion(treasure::core::getAppVersion());

    bool helpOrVersion = false;
    QString cliError;
    auto launchOpts = treasure::core::parseCommandLine(app.arguments(), &helpOrVersion, &cliError);

    if (helpOrVersion) {
        std::cout << treasure::core::getCommandLineHelp().toStdString() << std::endl;
        vips_shutdown();
        return 0;
    }

    if (!cliError.isEmpty()) {
        std::cerr << "Error: " << cliError.toStdString() << std::endl;
        std::cerr << "Run 'wurm_explorer --help' for available options." << std::endl;
        vips_shutdown();
        return 1;
    }
    
    QString iconSvg = treasure::core::resolveResourcePath("assets/icons/wurm_explorer.svg");
    if (!QFile::exists(iconSvg)) {
        iconSvg = QCoreApplication::applicationDirPath() + "/assets/icons/wurm_explorer.svg";
    }
    if (!QFile::exists(iconSvg)) {
        iconSvg = treasure::core::resolveResourcePath("resources/wurmexplorer.svg");
    }
    QIcon appIcon(iconSvg);
    if (appIcon.isNull()) {
        QString iconPng = treasure::core::resolveResourcePath("resources/icon.png");
        appIcon = QIcon(iconPng);
    }
    app.setWindowIcon(appIcon);
    
    QString fontPath = treasure::core::resolveResourcePath("resources/fonts/MaterialIcons-Regular.ttf");
    QFontDatabase::addApplicationFont(fontPath);
    
    Theme::apply(app);
    
    MainWindow window;

    if (launchOpts.initialTab >= 0) {
        window.selectInitialTab(launchOpts.initialTab);
    }

    if (launchOpts.captureDocs) {
        window.captureDocScreenshots(launchOpts.captureDocsDir);
        vips_shutdown();
        return 0;
    }

    if (launchOpts.fullscreen) {
        window.showFullScreen();
    } else if (launchOpts.maximized) {
        window.showMaximized();
    } else {
        window.resize(launchOpts.width, launchOpts.height);
        window.show();
    }
    
    int result = app.exec();
    vips_shutdown();
    return result;
}
