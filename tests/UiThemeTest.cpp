#include <QtTest/QtTest>

#include "src/gui/UiTheme.h"

class UiThemeTest : public QObject {
    Q_OBJECT

private slots:
    void stylesAreNotEmpty() {
        QVERIFY(!ui::dialogBackground().isEmpty());
        QVERIFY(ui::primaryButtonStyle().contains("QPushButton"));
        QVERIFY(ui::headerFrameStyle().contains("QFrame"));
    }

    void cachedStylesReturnIdenticalPointers() {
        const QString& first = ui::tableStyle();
        const QString& second = ui::tableStyle();
        QCOMPARE(&first, &second);
    }
};

QTEST_APPLESS_MAIN(UiThemeTest)
#include "UiThemeTest.moc"
