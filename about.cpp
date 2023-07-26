#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#	include <QDesktopWidget>
#endif

#include "about.h"
#include "ui_about.h"
#include "osver.h"
#include "compver.h"
#include "prgver.h"

/***********************************************//**
 * Конструктор
 * \brief About(QWidget *parent)
 ***************************************************/
About::About(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::About) {
	m_ui->setupUi(this);

	QString osVers;
	QString vcVers;
	QString s;
    QString vers;
    getPrgVers ("", vers);
	QString platform;
#ifdef __WIN32__
	platform = "(x32)";
#endif
#ifdef __WIN64__
	platform = "(amd64)";
#endif
    m_ui->lblVers->setText (QString("v.%1 %2").arg(vers).arg(platform));
	GetOsVers(osVers);
	GetCompVers(vcVers);
	s = QString("<table><tr><td colspan=2>") + osVers + QString("</tr>") ;
#ifdef _MSC_VER
	s += "<tr><td>" + vcVers;
	s += "<td><a href=\"http://www.microsoft.com/\">http://www.microsoft.com/</a></td></tr>";
#endif
#ifdef __GNUC__
	s += QString("<tr><td>%1").arg(vcVers);
	s += "<td><a href=\"http://gcc.gnu.org/\">http://gcc.gnu.org/</a></td></tr>";
#endif
#ifdef __MINGW32__
	s += "<tr><td>MinGW v.";
	s += QString("%1.%2").arg(__MINGW32_MAJOR_VERSION).arg(__MINGW32_MINOR_VERSION);
	s += "<td><a href=\"http://www.mingw.org/\">http://www.mingw.org/</a></td></tr>";
#endif
	s += "<tr><td>QT Library v." + QString(qVersion()) + "<td><a href=\"http://www.qt-project.org/\">http://www.qt-project.org</a></td></tr></table>";
	m_ui->lblBuild->setText(s);
}

/**************************************************//**
 * Деструктор
 * \brief ~About()
 *****************************************************/
About::~About()
{
	delete m_ui;
}

/**************************************************//**
 * Обработчик выбора языка
 * \brief changeEvent(QEvent *e)
 *****************************************************/
void About::changeEvent(QEvent *e)
{
	switch (e->type()) {
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);
			break;
		default:
			break;
	}
}
