#include <qstring.h>
#include "compver.h"

/*******************************************************//**
 * Функция возвращает наименование и версию
 * \param[out] &vc Адрес класса строки с версией компилятора
 ***********************************************************/
void GetCompVers(QString &vc) {
	vc = "unknown compiler";
#ifdef _MSC_VER
	int vers = _MSC_VER;
	vc = "Microsoft Visual C++ ";
	switch (vers) {
		case 1200:
			vc += "6";
			break;
		case 1310:
			vc += ".NET 2003";
			break;
		case 1400:
			vc += "2005";
			break;
		case 1500:
			vc += "2008";
			break;
		case 1600:
			vc += "2010";
			break;
		case 1800:
			vc += "2013";
			break;
		default:
			vc = QString("unknown %1").arg(vers);
	}
#endif
#ifdef __GNUC__
#	ifdef __GNUC_PATCHLEVEL__
	    vc = QString("GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);	 
#	else 	 
	     vc = QString("GCC %1.%2").arg(__GNUC__).arg(__GNUC_MINOR__);	 
#	endif
#endif
}

