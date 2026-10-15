#include "htmlexport.h"

#include "schedulemodel.h"
#include "timetable.h"

#include <QCoreApplication>

namespace {

QString cell(const QString &text, const QString &attributes = QString())
{
    const QString content = text.isEmpty() ? QStringLiteral("&nbsp;") : text.toHtmlEscaped();
    return attributes.isEmpty() ? QStringLiteral("<td>%1</td>").arg(content)
                                : QStringLiteral("<td %1>%2</td>").arg(attributes, content);
}

} // namespace

QString scheduleToHtml(const ScheduleModel &model, const QString &title)
{
    const QStringList groups = model.groups();

    QString html;
    html += QStringLiteral("<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n");
    html += QStringLiteral("<title>%1</title>\n").arg(title.toHtmlEscaped());
    html +=
        QStringLiteral("<style>\n"
                       "table { border-collapse: collapse; }\n"
                       "td, th { border: 1px solid #000; padding: 3px 6px; text-align: center; }\n"
                       "th { background: #e8e8e8; }\n"
                       "</style>\n</head>\n<body>\n");
    html += QStringLiteral("<h2>%1</h2>\n").arg(title.toHtmlEscaped());
    html +=
        QStringLiteral("<table width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"4\">\n");

    // Header row
    html += QStringLiteral("<tr><th colspan=\"2\">%1</th><th>%2</th>")
                .arg(QCoreApplication::translate("HtmlExport", "Pair / Lesson").toHtmlEscaped(),
                     QCoreApplication::translate("HtmlExport", "Time").toHtmlEscaped());
    for (const QString &group : groups)
        html += QStringLiteral("<th>%1</th>").arg(group.toHtmlEscaped());
    html += QStringLiteral("</tr>\n");

    // Lesson rows
    for (int lesson = 0; lesson < Timetable::LessonCount; ++lesson) {
        html += QStringLiteral("<tr>");
        if (Timetable::lessonInPair(lesson) == 1) {
            html += cell(QString::number(Timetable::pairNumber(lesson)),
                         QStringLiteral("rowspan=\"%1\"").arg(Timetable::LessonsPerPair));
        }
        html += cell(QString::number(lesson + 1));
        html += cell(Timetable::timeRange(lesson));
        for (int column = 0; column < groups.size(); ++column)
            html += cell(model.lesson(column, lesson));
        html += QStringLiteral("</tr>\n");
    }

    html += QStringLiteral("</table>\n</body>\n</html>\n");
    return html;
}
