
#ifndef PRG_VER
#define PRG_VER
#include <QString>
void getPrgVers(QString prgFile, QString &prgVers);
#ifndef _MSC_VER
#include <qstring.h>
inline
void getPrgVers(QString, QString &prgVers) {
    prgVers=QString( "1.0.1" );
}
#endif
#endif
