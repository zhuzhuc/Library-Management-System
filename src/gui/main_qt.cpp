#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QTranslator>

#include "MainWindow.h"

namespace {

bool installTranslator(QApplication& app) {
    auto* translator = new QTranslator(&app);
    const QString locale = QLocale::system().name();
    const QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/translations",
        QCoreApplication::applicationDirPath() + "/../translations"
    };
    for (const auto& path : searchPaths) {
        if (translator->load(QStringLiteral("app_%1").arg(locale), path)) {
            app.installTranslator(translator);
            return true;
        }
    }
    translator->deleteLater();
    return false;
}

} // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    installTranslator(app);

    MainWindow w;
    w.show();

    return app.exec();
}
