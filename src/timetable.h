#ifndef SCHEDULE_TIMETABLE_H
#define SCHEDULE_TIMETABLE_H

#include <QString>

/// Fixed bell schedule: 6 pairs, 2 lessons per pair.
/// Single source of truth for both the table header and the HTML/PDF export.
namespace Timetable {

constexpr int LessonsPerPair = 2;
constexpr int PairCount = 6;
constexpr int LessonCount = PairCount * LessonsPerPair;

struct Slot
{
    const char *start;
    const char *end;
};

constexpr Slot Slots[LessonCount] = {
    {"8:00", "8:45"},   {"8:50", "9:35"},   // pair 1
    {"9:55", "10:40"},  {"11:00", "11:45"}, // pair 2
    {"12:05", "12:50"}, {"12:55", "13:40"}, // pair 3
    {"14:00", "14:45"}, {"14:50", "15:35"}, // pair 4
    {"15:55", "16:40"}, {"17:00", "17:45"}, // pair 5
    {"17:55", "18:40"}, {"18:45", "19:30"}, // pair 6
};

/// 0-based lesson index -> "8:00-8:45"
inline QString timeRange(int lesson)
{
    return QStringLiteral("%1-%2").arg(QLatin1String(Slots[lesson].start),
                                       QLatin1String(Slots[lesson].end));
}

/// 0-based lesson index -> 1-based pair number
constexpr int pairNumber(int lesson)
{
    return lesson / LessonsPerPair + 1;
}

/// 0-based lesson index -> 1-based lesson number inside its pair
constexpr int lessonInPair(int lesson)
{
    return lesson % LessonsPerPair + 1;
}

} // namespace Timetable

#endif // SCHEDULE_TIMETABLE_H
