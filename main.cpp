#include "mainwindow.h"
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator translator;
    translator.load(":/transfer/schedule_ru");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
