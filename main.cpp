#include "mainwindow.h"
//#include "widget.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    /*국제화*/
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CSApp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    /*메인윈도우를 띄우는 부분*/
    MainWindow w;
    w.show();
    return a.exec();
}
