#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class AppSettings : public QObject {
    Q_OBJECT
public:
    static AppSettings& instance();

    QString loginBackgroundPath() const;
    QString mainBackgroundPath() const;

    void setLoginBackgroundPath(const QString& path);
    void setMainBackgroundPath(const QString& path);
    void resetLoginBackground();
    void resetMainBackground();

signals:
    void loginBackgroundChanged(const QString& path);
    void mainBackgroundChanged(const QString& path);

private:
    AppSettings();

    QSettings settings;
};
