#include <QApplication>
#include <QIcon>
#include <vips/vips8>
#include "ui/MainWindow.hpp"
#include "ui/Theme.hpp"
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    if (VIPS_INIT(argv[0])) {
        vips_error_exit(NULL);
    }

    QApplication app(argc, argv);
    
    // Set up application metadata
    QApplication::setApplicationName("WurmExplorer");
    QApplication::setOrganizationName("WurmMods");
    
    QIcon appIcon("resources/wurmexplorer.svg");
    if (appIcon.isNull()) {
        appIcon = QIcon("resources/icon.png");
    }
    app.setWindowIcon(appIcon);
    
    QFontDatabase::addApplicationFont("resources/fonts/MaterialIcons-Regular.ttf");
    
    Theme::apply(app);
    
    MainWindow window;
    window.resize(1200, 800);
    window.show();
    
    int result = app.exec();
    vips_shutdown();
    return result;
}
