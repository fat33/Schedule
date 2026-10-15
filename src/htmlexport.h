#ifndef SCHEDULE_HTMLEXPORT_H
#define SCHEDULE_HTMLEXPORT_H

#include <QString>

class ScheduleModel;

/// Renders the schedule as a self-contained HTML table.
/// Used for printing and PDF export (QTextDocument understands this subset of HTML).
QString scheduleToHtml(const ScheduleModel &model, const QString &title);

#endif // SCHEDULE_HTMLEXPORT_H
