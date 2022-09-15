#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "yfel_config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qSetMessagePattern("[%{type}] %{time yyyy-MM-dd hh:mm:ss.zzz}  %{function}:%{line} %{message}");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "YFEL_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.setWindowTitle(PROJECT_NAME + QString(" - ") + PROJECT_DESCRIPTION);
    w.setWindowIcon(QIcon(":/assets/img/icon.png"));
    w.show();
    return a.exec();
}
