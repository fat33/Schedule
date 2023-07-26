#include <qstring.h>
#include "prgver.h"
#ifdef _MSC_VER
#include <windows.h>
#include <strsafe.h>
/***************************************************************//**
 * Функция возвращает версию программы
 * \param[in] &prgFile Адрес класса строки с именем файла программы
 * \param[out] &prgVer Адрес класса строки с версией программы
 *****************************************************************/
void getPrgVers(QString prgFile, QString &prgVers) {
	QString vers;
	char  *prgName;
	prgName = (char *) prgFile.data();

	DWORD  dwHandle=0;
	DWORD  dwLen=0;
	char   *buf;
	prgVers = "";
	if (dwLen = GetFileVersionInfoSizeA(prgName, &dwHandle)) {

		buf = (char *) malloc( dwLen );
		if( buf != NULL ) {
			memset((void *) buf, 0, dwLen);
			if (GetFileVersionInfoA(prgName, dwHandle, dwLen, (LPVOID) buf)) {

				HRESULT hr;

				struct LANGANDCODEPAGE {
					WORD wLanguage;
					WORD wCodePage;
				} *lpTranslate;

				UINT cbTranslate=0;

				VerQueryValue(buf, TEXT("\\VarFileInfo\\Translation"), (LPVOID *)&lpTranslate, &cbTranslate);

				WCHAR SubBlock[50];
				WCHAR *Buffer;
				UINT dwBytes;

				for (UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ ) {
					hr = StringCchPrintf(SubBlock, 50,
							 TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),
							 lpTranslate[i].wLanguage,
							 lpTranslate[i].wCodePage);
					if (FAILED(hr)) {
						// TODO: write error handler.
					} else {
						// Retrieve file description for language and code page "i".
						VerQueryValue(buf, SubBlock, (LPVOID *) &Buffer, &dwBytes);
						prgVers += QString::fromStdWString(Buffer);
						prgVers = prgVers.replace(", ",".");
					}
				}
			}
		}

		free(buf);
	}
}
#endif
