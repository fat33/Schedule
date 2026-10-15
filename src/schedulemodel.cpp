#include "schedulemodel.h"

#include "hierarchicalheaderview.h"
#include "timetable.h"

#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QStringList emptyLessons()
{
    QStringList lessons;
    lessons.reserve(Timetable::LessonCount);
    for (int i = 0; i < Timetable::LessonCount; ++i)
        lessons.append(QString());
    return lessons;
}

QStringList splitLegacyList(const QJsonValue &value)
{
    QStringList list;
    const QStringList parts = value.toString().split(QLatin1Char(','));
    for (const QString &part : parts) {
        const QString item = part.trimmed();
        if (!item.isEmpty() && !list.contains(item))
            list.append(item);
    }
    return list;
}

QStringList toStringList(const QJsonArray &array)
{
    QStringList list;
    for (const auto &value : array)
        list.append(value.toString());
    return list;
}

} // namespace

ScheduleModel::ScheduleModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    buildHeaderModel();
}

void ScheduleModel::buildHeaderModel()
{
    // Tree describing the vertical header:
    //   Schedule (rotated)
    //   ├── №      ── Time / Group
    //   ├── 1 ─┬── 1 ── 8:00-8:45
    //   │      └── 2 ── 8:50-9:35
    //   └── ...
    auto *root = new QStandardItem(tr("Schedule"));
    root->setData(1, Qt::UserRole); // rotate text in this cell

    auto *numberCell = new QStandardItem(QStringLiteral(" № "));
    numberCell->appendColumn({new QStandardItem(tr("Time / Group") + QStringLiteral("  "))});
    root->appendColumn({numberCell});

    for (int pair = 0; pair < Timetable::PairCount; ++pair) {
        auto *pairCell = new QStandardItem(QStringLiteral(" %1 ").arg(pair + 1));
        for (int i = 0; i < Timetable::LessonsPerPair; ++i) {
            const int lesson = pair * Timetable::LessonsPerPair + i;
            auto *lessonCell = new QStandardItem(QStringLiteral(" %1 ").arg(i + 1));
            lessonCell->appendColumn({new QStandardItem(Timetable::timeRange(lesson))});
            pairCell->appendColumn({lessonCell});
        }
        root->appendColumn({pairCell});
    }

    m_verticalHeaderModel.setItem(0, 0, root);
}

int ScheduleModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : Timetable::LessonCount + 1;
}

int ScheduleModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_groups.size());
}

QVariant ScheduleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        if (role == HierarchicalHeaderView::VerticalHeaderDataRole) {
            return QVariant::fromValue(
                static_cast<QObject *>(const_cast<QStandardItemModel *>(&m_verticalHeaderModel)));
        }
        return {};
    }

    const int column = index.column();
    const int row = index.row();
    if (column < 0 || column >= m_groups.size() || row < 0 || row >= rowCount())
        return {};

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return row == GroupRow ? m_groups.at(column) : m_lessons.at(column).at(row - 1);
    case Qt::TextAlignmentRole:
        return QVariant::fromValue<int>(Qt::AlignCenter);
    case Qt::FontRole:
        if (row == GroupRow) {
            QFont font;
            font.setBold(true);
            return font;
        }
        return {};
    case AllowedValuesRole:
        return row == GroupRow ? QStringList() : QStringList(QString()) + m_subjects;
    default:
        return {};
    }
}

bool ScheduleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || index.row() == GroupRow)
        return false;
    return setLesson(index.column(), index.row() - 1, value.toString());
}

Qt::ItemFlags ScheduleModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.row() != GroupRow)
        f |= Qt::ItemIsEditable;
    return f;
}

QString ScheduleModel::normalized(const QString &name)
{
    return name.simplified();
}

bool ScheduleModel::addGroup(const QString &rawName)
{
    const QString name = normalized(rawName);
    if (name.isEmpty() || m_groups.contains(name))
        return false;

    const int column = static_cast<int>(m_groups.size());
    beginInsertColumns(QModelIndex(), column, column);
    m_groups.append(name);
    m_lessons.append(emptyLessons());
    endInsertColumns();
    emit contentChanged();
    return true;
}

bool ScheduleModel::removeGroup(int column)
{
    if (column < 0 || column >= m_groups.size())
        return false;

    beginRemoveColumns(QModelIndex(), column, column);
    m_groups.removeAt(column);
    m_lessons.removeAt(column);
    endRemoveColumns();
    emit contentChanged();
    return true;
}

bool ScheduleModel::addSubject(const QString &rawName)
{
    const QString name = normalized(rawName);
    if (name.isEmpty() || m_subjects.contains(name))
        return false;
    m_subjects.append(name);
    emit contentChanged();
    return true;
}

bool ScheduleModel::removeSubject(const QString &name)
{
    if (!m_subjects.removeOne(name))
        return false;

    for (int column = 0; column < m_lessons.size(); ++column) {
        QStringList &lessons = m_lessons[column];
        for (int i = 0; i < lessons.size(); ++i) {
            if (lessons.at(i) == name) {
                lessons[i].clear();
                const QModelIndex idx = index(i + 1, column);
                emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
            }
        }
    }
    emit contentChanged();
    return true;
}

QString ScheduleModel::lesson(int column, int lessonIndex) const
{
    if (column < 0 || column >= m_lessons.size() || lessonIndex < 0 ||
        lessonIndex >= Timetable::LessonCount)
        return {};
    return m_lessons.at(column).at(lessonIndex);
}

bool ScheduleModel::setLesson(int column, int lessonIndex, const QString &subject)
{
    if (column < 0 || column >= m_lessons.size() || lessonIndex < 0 ||
        lessonIndex >= Timetable::LessonCount)
        return false;
    if (!subject.isEmpty() && !m_subjects.contains(subject))
        return false;

    QString &cell = m_lessons[column][lessonIndex];
    if (cell == subject)
        return true;

    cell = subject;
    const QModelIndex idx = index(lessonIndex + 1, column);
    emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
    emit contentChanged();
    return true;
}

void ScheduleModel::clear()
{
    beginResetModel();
    m_groups.clear();
    m_subjects.clear();
    m_lessons.clear();
    endResetModel();
    emit contentChanged();
}

// ---------------------------------------------------------------------------
// Persistence
//
// Format v2:
// {
//   "version": 2,
//   "subjects": ["Math", "Physics"],
//   "groups": [ { "name": "10A", "lessons": ["Math", "", "Physics", ...] } ]
// }
// ---------------------------------------------------------------------------

QByteArray ScheduleModel::toJson() const
{
    QJsonArray groups;
    for (int column = 0; column < m_groups.size(); ++column) {
        QJsonObject group;
        group.insert(QStringLiteral("name"), m_groups.at(column));
        group.insert(QStringLiteral("lessons"), QJsonArray::fromStringList(m_lessons.at(column)));
        groups.append(group);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), FormatVersion);
    root.insert(QStringLiteral("subjects"), QJsonArray::fromStringList(m_subjects));
    root.insert(QStringLiteral("groups"), groups);
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool ScheduleModel::fromJson(const QByteArray &json, QString *errorMessage)
{
    auto fail = [errorMessage](const QString &message) {
        if (errorMessage)
            *errorMessage = message;
        return false;
    };

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail(tr("Invalid JSON: %1 (offset %2)")
                        .arg(parseError.errorString())
                        .arg(parseError.offset));
    if (!doc.isObject())
        return fail(tr("Invalid file: a JSON object is expected"));

    const QJsonObject root = doc.object();
    if (!root.contains(QStringLiteral("version")))
        return loadLegacy(root);

    const int version = root.value(QStringLiteral("version")).toInt();
    if (version > FormatVersion)
        return fail(
            tr("The file was created by a newer version of the program (format %1)").arg(version));

    QStringList subjects;
    for (const QString &s : toStringList(root.value(QStringLiteral("subjects")).toArray())) {
        const QString name = normalized(s);
        if (!name.isEmpty() && !subjects.contains(name))
            subjects.append(name);
    }

    QStringList groups;
    QVector<QStringList> lessons;
    const QJsonArray storedGroups = root.value(QStringLiteral("groups")).toArray();
    for (const auto &value : storedGroups) {
        const QJsonObject group = value.toObject();
        const QString name = normalized(group.value(QStringLiteral("name")).toString());
        if (name.isEmpty() || groups.contains(name))
            return fail(tr("Invalid file: empty or duplicate group name \"%1\"").arg(name));

        QStringList column = emptyLessons();
        const QStringList stored = toStringList(group.value(QStringLiteral("lessons")).toArray());
        for (int i = 0; i < qMin(stored.size(), int(Timetable::LessonCount)); ++i) {
            if (stored.at(i).isEmpty() || subjects.contains(stored.at(i)))
                column[i] = stored.at(i);
        }
        groups.append(name);
        lessons.append(column);
    }

    beginResetModel();
    m_subjects = subjects;
    m_groups = groups;
    m_lessons = lessons;
    endResetModel();
    emit contentChanged();
    return true;
}

// Legacy v1 format written by Schedule 1.0:
// { "Object": "a,b", "Group": "g1,g2", "Array": [ row-major cell values ] }
// Row 0 of "Array" contains group names. Loaded on a best-effort basis.
bool ScheduleModel::loadLegacy(const QJsonObject &root)
{
    const QStringList subjects = splitLegacyList(root.value(QStringLiteral("Object")));
    const QStringList groups = splitLegacyList(root.value(QStringLiteral("Group")));

    QVector<QStringList> lessons(groups.size(), emptyLessons());
    const QJsonArray cells = root.value(QStringLiteral("Array")).toArray();
    if (!groups.isEmpty()) {
        for (int i = 0; i < cells.size(); ++i) {
            const int row = i / static_cast<int>(groups.size());
            const int column = i % static_cast<int>(groups.size());
            if (row == GroupRow)
                continue;
            if (row > Timetable::LessonCount)
                break;
            const QString value = cells.at(i).toString();
            if (subjects.contains(value))
                lessons[column][row - 1] = value;
        }
    }

    beginResetModel();
    m_subjects = subjects;
    m_groups = groups;
    m_lessons = lessons;
    endResetModel();
    emit contentChanged();
    return true;
}
