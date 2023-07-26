#ifndef SCHEDULEMODEL_H
#define SCHEDULEMODEL_H

#include <QAbstractTableModel>
#include <QStandardItemModel>

class ScheduleModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ScheduleModel(QObject *parent = nullptr, QStringList group = QStringList(), QStringList object = QStringList());

    void fillHeaderModel(QStandardItemModel& headerModel);
    // Header:
//    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;


    QMap<QPair<int,int>, QString> reconstruct(QMap<QPair<int,int>, QString>, const QStringList & list);
    QMap<QPair<int, int>, QString> reconstruct(QMap<QPair<int, int>, QString> &, int, int );
    int findGroup(QMap<QPair<int,int>, QString>, const QString & string);
    void setGroupList(QStringList);
    void setObjectList(QStringList);

    QByteArray save();
    void open(QByteArray);

private:
    QStringList groupList;
    QStringList objectList;
    QMap<QPair<int,int>, QString> mapData;
    QStandardItemModel _verticalHeaderModel;
};

#endif // SCHEDULEMODEL_H
