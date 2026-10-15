#ifndef SCHEDULE_SCHEDULEMODEL_H
#define SCHEDULE_SCHEDULEMODEL_H

#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QStringList>
#include <QVector>

class QJsonObject;

/**
 * Table model of a weekly-day timetable.
 *
 * Layout:
 *  - every column is a study group;
 *  - row 0 shows the group name (read-only);
 *  - rows 1..Timetable::LessonCount hold the subject for that lesson
 *    (empty string means "no lesson").
 *
 * A cell may only contain a subject from subjects(). Removing a subject
 * clears every cell that used it; removing a group removes its column.
 *
 * The vertical header is described by a tree model returned for
 * HierarchicalHeaderView::VerticalHeaderDataRole.
 */
class ScheduleModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Role {
        /// QStringList of allowed values for a cell (used by the editor delegate).
        AllowedValuesRole = Qt::UserRole + 100,
    };

    static constexpr int GroupRow = 0;
    static constexpr int FormatVersion = 2;

    explicit ScheduleModel(QObject *parent = nullptr);

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Groups (columns)
    QStringList groups() const { return m_groups; }
    bool addGroup(const QString &name);
    bool removeGroup(int column);

    // Subjects (allowed cell values)
    QStringList subjects() const { return m_subjects; }
    bool addSubject(const QString &name);
    bool removeSubject(const QString &name);

    /// 0-based lesson index; empty string if no lesson.
    QString lesson(int column, int lessonIndex) const;
    bool setLesson(int column, int lessonIndex, const QString &subject);

    void clear();

    // Persistence
    QByteArray toJson() const;
    /// Accepts both the current format and the legacy v1 format.
    bool fromJson(const QByteArray &json, QString *errorMessage = nullptr);

signals:
    /// Emitted after any user-visible change of the schedule content.
    void contentChanged();

private:
    void buildHeaderModel();
    bool loadLegacy(const QJsonObject &root);
    static QString normalized(const QString &name);

    QStringList m_groups;
    QStringList m_subjects;
    QVector<QStringList> m_lessons; ///< m_lessons[column][lessonIndex]
    QStandardItemModel m_verticalHeaderModel;
};

#endif // SCHEDULE_SCHEDULEMODEL_H
