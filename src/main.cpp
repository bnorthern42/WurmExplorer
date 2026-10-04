#include <QApplication>
#include <QIcon>
#include <QFontDatabase>
#include <QFile>
#include "core/PathUtils.hpp"
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
    window.resize(1200, 800);

    if (argc > 1 && QString::fromUtf8(argv[1]) == "--capture-docs") {
        QString outDir = (argc > 2) ? QString::fromUtf8(argv[2]) : "assets/docs";
        window.captureDocScreenshots(outDir);
        vips_shutdown();
        return 0;
    }

    window.show();
    
    int result = app.exec();
    vips_shutdown();
    return result;
}
