#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

namespace {

void installTranslations(QApplication &app)
{
    const QLocale locale;

    auto *qtTranslator = new QTranslator(&app);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QString qtTranslations = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const QString qtTranslations = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    if (qtTranslator->load(locale, QStringLiteral("qtbase"), QStringLiteral("_"), qtTranslations))
        QApplication::installTranslator(qtTranslator);

    auto *appTranslator = new QTranslator(&app);
    if (appTranslator->load(locale, QStringLiteral("schedule"), QStringLiteral("_"),
                            QStringLiteral(":/i18n")))
        QApplication::installTranslator(appTranslator);
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("Schedule"));
    QApplication::setApplicationName(QStringLiteral("Schedule"));
    QApplication::setApplicationVersion(QStringLiteral(SCHEDULE_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/schedule.ico")));

    installTranslations(app);
    QApplication::setApplicationDisplayName(QApplication::translate("main", "Schedule"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("main", "Class timetable editor"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"),
                                 QApplication::translate("main", "Schedule file to open."));
    parser.process(app);

    MainWindow window;
    window.show();
    const QStringList files = parser.positionalArguments();
    window.openInitialFile(files.isEmpty() ? QString() : files.first());

    return app.exec();
}
