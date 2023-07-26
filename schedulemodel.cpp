#include "hierarchicalheaderview.h"
#include "schedulemodel.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qdebug.h>

ScheduleModel::ScheduleModel(QObject *parent, QStringList group, QStringList object)
    : QAbstractTableModel(parent), groupList(group), objectList(object)
{
    fillHeaderModel(_verticalHeaderModel);
}

void ScheduleModel::fillHeaderModel(QStandardItemModel &headerModel)
{
    QStandardItem* rootItem = new QStandardItem(tr("Schedule"));
    rootItem->setData(1, Qt::UserRole);
    QList<QStandardItem*> l;

    QStandardItem* numberCell=new QStandardItem(" № ");
    l.push_back(numberCell);
    rootItem->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(tr("Time-Group")+QString("  ")));
    numberCell->appendColumn(l);
    l.clear();

    QStandardItem* cell_1=new QStandardItem(" 1 ");
    l.push_back(cell_1);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_11=new QStandardItem(" 1 ");
    l.push_back(cell_11);
    cell_1->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("8:00-8:45")));
    cell_11->appendColumn(l);
    l.clear();

    QStandardItem* cell_12=new QStandardItem(" 2 ");
    l.push_back(cell_12);
    cell_1->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("8:50-9:35")));
    cell_12->appendColumn(l);
    l.clear();

    QStandardItem* cell_2=new QStandardItem(" 2 ");
    l.push_back(cell_2);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_21=new QStandardItem(" 1 ");
    l.push_back(cell_21);
    cell_2->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("9:55-10:40")));
    cell_21->appendColumn(l);
    l.clear();

    QStandardItem* cell_22=new QStandardItem(" 2 ");
    l.push_back(cell_22);
    cell_2->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("11:00-11:45")));
    cell_22->appendColumn(l);
    l.clear();

    QStandardItem* cell_3=new QStandardItem(" 3 ");
    l.push_back(cell_3);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_31=new QStandardItem(" 1 ");
    l.push_back(cell_31);
    cell_3->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("12:05-12:50")));
    cell_31->appendColumn(l);
    l.clear();

    QStandardItem* cell_32=new QStandardItem(" 2 ");
    l.push_back(cell_32);
    cell_3->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("12:55-13:40")));
    cell_32->appendColumn(l);
    l.clear();

    QStandardItem* cell_4=new QStandardItem(" 4 ");
    l.push_back(cell_4);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_41=new QStandardItem(" 1 ");
    l.push_back(cell_41);
    cell_4->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("14:00-14:45")));
    cell_41->appendColumn(l);
    l.clear();

    QStandardItem* cell_42=new QStandardItem(" 2 ");
    l.push_back(cell_42);
    cell_4->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("14:50-15:35")));
    cell_42->appendColumn(l);
    l.clear();

    QStandardItem* cell_5=new QStandardItem(" 5 ");
    l.push_back(cell_5);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_51=new QStandardItem(" 1 ");
    l.push_back(cell_51);
    cell_5->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("15:55-16:40")));
    cell_51->appendColumn(l);
    l.clear();

    QStandardItem* cell_52=new QStandardItem(" 2 ");
    l.push_back(cell_52);
    cell_5->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("17:00-17:45")));
    cell_52->appendColumn(l);
    l.clear();

    QStandardItem* cell_6=new QStandardItem(" 6 ");
    l.push_back(cell_6);
    rootItem->appendColumn(l);
    l.clear();

    QStandardItem* cell_61=new QStandardItem(" 1 ");
    l.push_back(cell_61);
    cell_6->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("17:55-18:40")));
    cell_61->appendColumn(l);
    l.clear();

    QStandardItem* cell_62=new QStandardItem(" 2 ");
    l.push_back(cell_62);
    cell_6->appendColumn(l);
    l.clear();

    l.push_back(new QStandardItem(("18:45-19:30")));
    cell_62->appendColumn(l);
    l.clear();

    headerModel.setItem(0, 0, rootItem);
}

int ScheduleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 13;
}

int ScheduleModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return groupList.count();
}

QVariant ScheduleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        if(role == HierarchicalHeaderView::VerticalHeaderDataRole)
        {
            QVariant v;
            v.setValue((QObject*)&_verticalHeaderModel);
            return v;
            //        return QVariant::fromValue<QObject *>(_verticalHeaderModel);
        }
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return mapData[{index.row(), index.column()}];
    }
    if (role == Qt::EditRole)
    {
        return "";
    }
    return QVariant();
}

bool ScheduleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        mapData[{index.row(), index.column()}] = value.toString();
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags ScheduleModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled|Qt::ItemIsEditable; // FIXME: Implement me!
}


bool ScheduleModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}


bool ScheduleModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

QMap<QPair<int, int>, QString> ScheduleModel::reconstruct(QMap<QPair<int, int>, QString> oldMap, const QStringList & list)
{
    for(int i = 0; i < list.count(); ++i)
    {
        int columnFind = findGroup(oldMap,list.at(i));
        int idx = 0;
        while(columnFind >= 0)
        {
            oldMap = reconstruct(oldMap, columnFind, idx );
            columnFind = findGroup(oldMap,list.at(i));
            ++idx;
        }
    }
    return oldMap;
}

QMap<QPair<int, int>, QString> ScheduleModel::reconstruct(QMap<QPair<int, int>, QString> &oldMap, int col, int idx)
{
    QMap<QPair<int,int>, QString> temp;
    QMap<QPair<int,int>, QString>::iterator i = oldMap.begin();
    while (i != oldMap.end()) {
        if(i.key().second > col)
        {
            temp[{i.key().first, i.key().second-1}] = i.value();
        }
        else if(i.key().second < col)
        {
            temp[i.key()] = i.value();
        }
        ++i;
    }
    if(idx == 0)
        removeColumns(col, 1);
    return temp;

}

int ScheduleModel::findGroup(QMap<QPair<int, int>, QString> map, const QString &string)
{
    QMap<QPair<int,int>, QString>::iterator i = map.begin();
    while (i != map.end()) {
        if(string == i.value())
        {
            return i.key().second;
        }
        ++i;
    }
    return -1;
}

void ScheduleModel::setGroupList(QStringList list)
{
    int count = list.count() - groupList.count();

    QStringList delTermList;
    for(int i = 0; i < groupList.count(); ++i)
    {
        if(!list.contains(groupList.at(i)))
        {
            delTermList.append(groupList.at(i));
        }
    }

    groupList = list;

    if(count > 0)
    {
        insertColumns(groupList.count()-1, qAbs(count));
        QModelIndex index = this->index( 0, groupList.count()-1, QModelIndex());
        setData(index,groupList.at(groupList.count()-1));
    }
    else
    {
        mapData = reconstruct(mapData, delTermList);
    }
}

void ScheduleModel::setObjectList(QStringList list)
{
    objectList = list;
    QMap<QPair<int,int>, QString>::iterator i = mapData.begin();
    while (i != mapData.end()) {
        if(!objectList.contains(i.value()))
        {
            if(i.key().first != 0)
            {
                QModelIndex index = this->index(i.key().first,  i.key().second, QModelIndex());
                mapData.remove(i.key());
                emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
            }
        }
        ++i;
    }
}


QByteArray ScheduleModel::save()
{
    QJsonObject bodyObject;
    bodyObject.insert("Object", QJsonValue::fromVariant(objectList.join(",")));
    bodyObject.insert("Group" ,QJsonValue::fromVariant(groupList.join(",")));
    QJsonArray arrayJson;
    QMap<QPair<int,int>, QString>::iterator i = mapData.begin();
    while (i != mapData.end()) {
        arrayJson.append(QJsonValue::fromVariant(i.value()));
        ++i;
    }
    bodyObject.insert("Array", arrayJson);
    return QJsonDocument(bodyObject).toJson();
}

void ScheduleModel::open(QByteArray array)
{
    mapData.clear();
    objectList.clear();
    groupList.clear();

    QJsonObject bodyObject = QJsonDocument::fromJson(array).object();
    QJsonValue valueJson = bodyObject.value("Object");
    if(!valueJson.isUndefined() && !valueJson.isNull())
        objectList = valueJson.toVariant().toString().split(",");

    valueJson = bodyObject.value("Group");
    if(!valueJson.isUndefined() && !valueJson.isNull())
        groupList = valueJson.toVariant().toString().split(",");

    int currColumn = 0; // groupList.count();
    int currRow = 0;    //13;
    QJsonArray arrayJson = bodyObject.value("Array").toArray();
    foreach (QJsonValue value, arrayJson)
    {
        QString item = value.toString();
        mapData[{currRow, currColumn}] = item;
        ++currColumn;
        if(currColumn > groupList.count())
        {
            currColumn = 0;
            ++currRow;
        }
    }
    QModelIndex index1 = this->index(0, 0, QModelIndex());
    QModelIndex index2 = this->index(12, groupList.count()-1, QModelIndex());
    emit dataChanged(index1, index2, QVector<int>() <<  Qt::DisplayRole);
}
