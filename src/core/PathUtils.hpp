#pragma once

#include <QString>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace treasure::core {

inline QString getAppVersion() {
    QString ver = QCoreApplication::applicationVersion();
    if (ver.isEmpty()) {
#ifdef APP_VERSION
        ver = QString::fromUtf8(APP_VERSION);
#else
        ver = "0.2.1";
#endif
    }
    return ver;
}

inline std::string resolveStorePath(const QString& filename) {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    QString targetPath = configDir + "/" + filename;

    if (QFile::exists(targetPath)) {
        return targetPath.toStdString();
    }

    QString appConfig = QCoreApplication::applicationDirPath() + "/configs/" + filename;
    QString cwdConfig = "configs/" + filename;

    if (QFile::exists(appConfig)) {
        QFile::copy(appConfig, targetPath);
        if (QFile::exists(targetPath)) return targetPath.toStdString();
        return appConfig.toStdString();
    } else if (QFile::exists(cwdConfig)) {
        QFile::copy(cwdConfig, targetPath);
        if (QFile::exists(targetPath)) return targetPath.toStdString();
        return cwdConfig.toStdString();
    }

    return targetPath.toStdString();
}

inline std::string resolveServerConfigPath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString userServers = configDir + "/servers.yaml";
    if (QFile::exists(userServers)) {
        return userServers.toStdString();
    }

    QString appServers = QCoreApplication::applicationDirPath() + "/configs/servers.yaml";
    if (QFile::exists(appServers)) {
        return appServers.toStdString();
    }

    if (QFile::exists("configs/servers.yaml")) {
        return "configs/servers.yaml";
    }

    return userServers.toStdString();
}

inline QString resolveResourcePath(const QString& relativePath) {
    QString appDir = QCoreApplication::applicationDirPath();
    QString path1 = appDir + "/" + relativePath;
    if (QFile::exists(path1)) return path1;

    QString path2 = relativePath;
    if (QFile::exists(path2)) return path2;

    QString path3 = appDir + "/../" + relativePath;
    if (QFile::exists(path3)) return path3;

    return relativePath;
}

inline std::vector<std::filesystem::path> getMapSearchDirectories() {
    namespace fs = std::filesystem;
    std::vector<fs::path> dirs;

    QString appDir = QCoreApplication::applicationDirPath();
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    std::vector<QString> candidates = {
        appDir + "/assets/maps",
        appDir + "/../assets/maps",
        appDir + "/assets",
        appData + "/maps",
        appData + "/assets/maps",
        "assets/maps",
        "../assets/maps",
        "assets",
        "svrMaps",
        "../svrMaps"
    };

    for (const auto& c : candidates) {
        fs::path p(c.toStdString());
        if (fs::exists(p) && fs::is_directory(p)) {
            if (std::find(dirs.begin(), dirs.end(), p) == dirs.end()) {
                dirs.push_back(p);
            }
        }
    }
    return dirs;
}

} // namespace treasure::core
