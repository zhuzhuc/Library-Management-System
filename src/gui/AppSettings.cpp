#include "AppSettings.h"

AppSettings& AppSettings::instance() {
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings()
    : QObject(nullptr),
      settings("zhuzhuc", "library_gui") {}

QString AppSettings::loginBackgroundPath() const {
    return settings.value(QStringLiteral("ui/loginBackground")).toString();
}

QString AppSettings::mainBackgroundPath() const {
    return settings.value(QStringLiteral("ui/mainBackground")).toString();
}

void AppSettings::setLoginBackgroundPath(const QString& path) {
    if (settings.value(QStringLiteral("ui/loginBackground")).toString() == path) {
        return;
    }
    settings.setValue(QStringLiteral("ui/loginBackground"), path);
    emit loginBackgroundChanged(path);
}

void AppSettings::setMainBackgroundPath(const QString& path) {
    if (settings.value(QStringLiteral("ui/mainBackground")).toString() == path) {
        return;
    }
    settings.setValue(QStringLiteral("ui/mainBackground"), path);
    emit mainBackgroundChanged(path);
}

void AppSettings::resetLoginBackground() {
    settings.remove(QStringLiteral("ui/loginBackground"));
    emit loginBackgroundChanged(QString());
}

void AppSettings::resetMainBackground() {
    settings.remove(QStringLiteral("ui/mainBackground"));
    emit mainBackgroundChanged(QString());
}
