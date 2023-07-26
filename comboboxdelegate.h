#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <qstyleditemdelegate.h>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxDelegate(QObject *parent = 0, const QStringList & list = QStringList());

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    //QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options) const;
    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setList(const QStringList & list);

private:
    QStringList listItem;
signals:

public slots:

};

#endif // COMBOBOXDELEGATE_H
