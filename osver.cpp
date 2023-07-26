
#include <qstring.h>
#include "osver.h"
#if defined( __WIN32 )
#	include <windows.h>
#	ifdef _MSC_VER
#		include <VersionHelpers.h>
#	endif
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#	ifndef PRODUCT_BUSINESS
#define PRODUCT_BUSINESS						 0x00000006 // Business Edition
#define PRODUCT_BUSINESS_N						 0x00000010 // Business Edition
#define PRODUCT_CLUSTER_SERVER					 0x00000012 // Cluster Server Edition
#define PRODUCT_DATACENTER_SERVER				 0x00000008 // Server Datacenter Edition (full installation)
#define PRODUCT_DATACENTER_SERVER_CORE			 0x0000000C // Server Datacenter Edition (core installation)
#define PRODUCT_DATACENTER_SERVER_CORE_V		 0x00000027 // Server Datacenter Edition without Hyper-V (core installation)
#define PRODUCT_DATACENTER_SERVER_V 			 0x00000025 // Server Datacenter Edition without Hyper-V (full installation)
#define PRODUCT_ENTERPRISE						 0x00000004 //Enterprise Edition
#define PRODUCT_ENTERPRISE_N					 0x0000001B // Enterprise Edition
#define PRODUCT_ENTERPRISE_SERVER				 0x0000000A // Server Enterprise Edition (full installation)
#define PRODUCT_ENTERPRISE_SERVER_CORE			 0x0000000E // Server Enterprise Edition (core installation)
#define PRODUCT_ENTERPRISE_SERVER_CORE_V		 0x00000029 // Server Enterprise Edition without Hyper-V (core installation)
#define PRODUCT_ENTERPRISE_SERVER_IA64			 0x0000000F // Server Enterprise Edition for Itanium-based Systems
#define PRODUCT_ENTERPRISE_SERVER_V 			 0x00000026 //Server Enterprise Edition without Hyper-V (full installation)
#define PRODUCT_HOME_BASIC						 0x00000002 // Home Basic Edition
#define PRODUCT_HOME_BASIC_N					 0x00000005 // Home Basic Edition
#define PRODUCT_PROFESSIONAL					 0x00000030 // Profesional
#define PRODUCT_PROFESSIONAL_N					 0x00000031 // Profesional
#define PRODUCT_HOME_PREMIUM					 0x00000003 // Home Premium Edition
#define PRODUCT_HOME_PREMIUM_N					 0x0000001A // Home Premium Edition
#define PRODUCT_HOME_SERVER 					 0x00000013 // Home Server Edition
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT 0x0000001E // Windows Essential Business Server Management Server
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING  0x00000020 // Windows Essential Business Server Messaging Server
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY	 0x0000001F // Windows Essential Business Server Security Server
#define PRODUCT_SERVER_FOR_SMALLBUSINESS		 0x00000018 // Server for Small Business Edition
#define PRODUCT_SMALLBUSINESS_SERVER			 0x00000009 // Small Business Server
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM	 0x00000019 // Small Business Server Premium Edition
#define PRODUCT_STANDARD_SERVER 				 0x00000007 // Server Standard Edition (full installation)
#define PRODUCT_STANDARD_SERVER_CORE			 0x0000000D // Server Standard Edition (core installation)
#define PRODUCT_STANDARD_SERVER_CORE_V			 0x00000028 // Server Standard Edition without Hyper-V (core installation)
#define PRODUCT_STANDARD_SERVER_V				 0x00000024 // Server Standard Edition without Hyper-V (full installation)
#define PRODUCT_STARTER 						 0x0000000B // Starter Edition
#define PRODUCT_STORAGE_ENTERPRISE_SERVER		 0x00000017 // Storage Server Enterprise Edition
#define PRODUCT_STORAGE_EXPRESS_SERVER			 0x00000014 // Storage Server Express Edition
#define PRODUCT_STORAGE_STANDARD_SERVER 		 0x00000015 // Storage Server Standard Edition
#define PRODUCT_STORAGE_WORKGROUP_SERVER		 0x00000016 // Storage Server Workgroup Edition
#define PRODUCT_UNDEFINED						 0x00000000 // An unknown product
#define PRODUCT_ULTIMATE						 0x00000001 // Ultimate Edition
#define PRODUCT_ULTIMATE_N						 0x0000001C // Ultimate Edition
#define PRODUCT_WEB_SERVER						 0x00000011 // Web Server Edition (full installation)
#define PRODUCT_WEB_SERVER_CORE 				 0x0000001D // Web Server Edition (core installation)
#	endif
#define SM_SERVERR2 89 // The build number if the system is Windows Server 2003 R2;
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTextCodec>

/**
 * Определение версии ОС для Microsoft Windows 10 и выше
 * @brief GetOsVer10
 * @return Строку с описанием версии ОС
 */
QString GetOsVers10()
{
	QString versOS;
	QString fn = QDir::tempPath () + "\\getOsVers.vbs";
	QFile fileScript(fn);
	fileScript.open (QIODevice::WriteOnly);
	QFile file(":/getOsVers.vbs");
	file.open(QIODevice::ReadOnly);
	QByteArray line;
	while (!file.atEnd()) {
		line = file.readLine();
		if (!line.startsWith ('\'')) fileScript.write(line.data ());
	}
	file.close();
	fileScript.close();

	QProcess cscript;
	QStringList arguments;
	arguments << fn << "//NoLogo";
	cscript.start("cscript", arguments);
	if (cscript.waitForStarted()) {
		if (cscript.waitForFinished()) {
			QTextCodec *codec = QTextCodec::codecForName("CP866");
			versOS = codec->toUnicode(cscript.readAll());
		}
	}
	QFile::remove (fn);
	return versOS;
}
#else
#include <sys/utsname.h>
#endif

/********************************************//**
 * Формирование наименования и версии ОС
 * \param[out] &os адрес класса выходной строки
 ***********************************************/
void GetOsVers(QString &os) {

#if defined( __WIN32 )
/*=============================================
 * для среды MS Windows
 *============================================*/
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;
	DWORD dwType;
	QString OS10;
	os = "";

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ) return;

   // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

	pGNSI = (PGNSI) GetProcAddress(	GetModuleHandle(TEXT("kernel32.dll")),	"GetNativeSystemInfo");
	if	(NULL != pGNSI) pGNSI(&si);
	else GetSystemInfo(&si);

	if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion > 4 ) {
		os = "Microsoft ";

	  // Test for the specific product.
		if ( osvi.dwMajorVersion == 10 ) {
			   if( osvi.wProductType == VER_NT_WORKSTATION )	os += "Windows 10 ";
		} else if ( osvi.dwMajorVersion == 6 ) {
			if( osvi.dwMinorVersion == 0 ) {
			   if( osvi.wProductType == VER_NT_WORKSTATION )	os += "Windows Vista ";
			   else												os += "Windows Server 2008 ";
			} else if ( osvi.dwMinorVersion == 1 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION )	os += "Windows 7 ";
				else											os += "Windows Server 2008 R2 ";
			} else if (osvi.dwMinorVersion == 2) {
				OS10 = GetOsVers10();
				if (OS10.isEmpty ()) {
					if (osvi.wProductType == VER_NT_WORKSTATION) 	os += "Windows 8 ";
					else											os += "Windows Server 2012 ";
				} else os = OS10;
			} else if (osvi.dwMinorVersion == 3) {
				if (osvi.wProductType == VER_NT_WORKSTATION)
#ifdef _MSC_VER
					if (IsWindows8OrGreater())					os += "Windows 10.0 ";
					else
#endif
																os += "Windows 8.1 ";
				else											os += "Windows Server 2012 R2 ";
			}

			if (OS10.isEmpty ()) {
				pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetProductInfo");
				pGPI( 6, 0, 0, 0, &dwType);

				switch( dwType ) {
					case PRODUCT_ULTIMATE:					os += "Ultimate Edition"; break;
					case PRODUCT_HOME_PREMIUM:				os += "Home Premium Edition"; break;
					case PRODUCT_PROFESSIONAL:				os += "Professional"; break;
					case PRODUCT_HOME_BASIC:				os += "Home Basic Edition"; break;
					case PRODUCT_ENTERPRISE:				os +="Enterprise Edition"; 	break;
					case PRODUCT_BUSINESS:					os += "Business Edition"; 	break;
					case PRODUCT_STARTER:					os += "Starter Edition"; 	break;
					case PRODUCT_CLUSTER_SERVER:			os += "Cluster Server Edition"; break;
					case PRODUCT_DATACENTER_SERVER:			os += "Datacenter Edition"; break;
					case PRODUCT_DATACENTER_SERVER_CORE:	os += "Datacenter Edition (core installation)"; break;
					case PRODUCT_ENTERPRISE_SERVER: 		os += "Enterprise Edition"; break;
					case PRODUCT_ENTERPRISE_SERVER_CORE: 	os += "Enterprise Edition (core installation)"; break;
					case PRODUCT_ENTERPRISE_SERVER_IA64: 	os += "Enterprise Edition for Itanium-based Systems"; break;
					case PRODUCT_SMALLBUSINESS_SERVER: 		os += "Small Business Server"; break;
					case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM: 	os += "Small Business Server Premium Edition"; break;
					case PRODUCT_STANDARD_SERVER: 			os += "Standard Edition"; break;
				case PRODUCT_STANDARD_SERVER_CORE: 			os += "Standard Edition (core installation)"; break;
				case PRODUCT_WEB_SERVER:					os += "Web Server Edition"; break;
				}
			}
		} else if ( osvi.dwMajorVersion == 5) {
			if (osvi.dwMinorVersion == 2 ) {
				if( GetSystemMetrics(SM_SERVERR2) )							os += "Windows Server 2003 R2, ";
				else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )		os += "Windows Storage Server 2003";
				else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
																		os += "Windows XP Professional x64 Edition";
				else														os += "Windows Server 2003, ";
				// Test for the server type.
				if ( osvi.wProductType != VER_NT_WORKSTATION ) {
					if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 ) {
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) 		os += "Datacenter Edition for Itanium-based Systems";
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	os += "Enterprise Edition for Itanium-based Systems";
					} else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ) {
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )		os += "Datacenter x64 Edition";
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	os += "Enterprise x64 Edition";
						else												os += "Standard x64 Edition";
					} else {
						if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )	os += "Compute Cluster Edition";
						else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )	os += "Datacenter Edition";
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	os += "Enterprise Edition";
						else if ( osvi.wSuiteMask & VER_SUITE_BLADE )		os += "Web Edition";
						else												os += "Standard Edition";
					}
				}
			} else if ( osvi.dwMinorVersion == 1 ) {
																			os += "Windows XP ";
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )					os += "Home Edition";
				else														os += "Professional";
			} else if (osvi.dwMinorVersion == 0 ) {
																			os += "Windows 2000 ";
				if ( osvi.wProductType == VER_NT_WORKSTATION ) {			os += "Professional";
				} else {
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )			os += "Datacenter ";
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )		os += "Advanced ";
																			os += "Server";
				}
			}
		}
	   // Include service pack (if any) and build number.
//	  if( _tcslen(osvi.szCSDVersion) > 0 )
//	  os += QString(" %1").arg(osvi.szCSDVersion);
		if (OS10.isEmpty ()) os += QString(" (build %1)").arg(osvi.dwBuildNumber);
		if ( osvi.dwMajorVersion >= 6 ) {
			if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )		os += ", 64-bit";
			else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )	os += ", 32-bit";
		}
	} else {
		os = "Microsoft Windows";
	}

#else
/*=============================================
 * для Линукс
 *============================================*/
	struct utsname u_name;	/* Creates an instance of the utsname structure */
	/* Uses the C function 'uname' and stores the
	** results in the structure we just created.
	** You could add some error checking, by testing the value of z -
	** But I cut this out, though you should add it.
	*/
	uname(&u_name);
	os += QString::fromLocal8Bit(u_name.sysname);
//	os += QString::fromLocal8Bit(u_name.nodename);
	os += " " + QString::fromLocal8Bit(u_name.release);
	os += " " + QString::fromLocal8Bit(u_name.version);
//	s += QString::fromLocal8Bit(u_name.machine);

#endif

}
