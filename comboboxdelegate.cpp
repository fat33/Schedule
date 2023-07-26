#include "comboboxdelegate.h"
#include <QComboBox>
#include <QCheckBox>
#include <QRect>
#include <QApplication>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent, const QStringList & list) :
    QStyledItemDelegate(parent), listItem(list)
{
}

QWidget* ComboBoxDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
    QComboBox *cb = new QComboBox(parent);
    cb->addItems(listItem);
    return cb;
}

void ComboBoxDelegate::setEditorData ( QWidget *editor, const QModelIndex &index ) const
{
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // get the index of the text in the combobox that matches the current value of the itenm
        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);
        // if it is valid, adjust the combobox
        if(cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

//void ComboBoxDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
//{
//    if(QComboBox *cb = qobject_cast<QComboBox *>(editor))
//        model->setData(index, cb->currentText(), Qt::EditRole);
//    else
//        QStyledItemDelegate::setModelData(editor, model, index);
//}

//QRect ComboBoxDelegate::CheckBoxRect(const QStyleOptionViewItem &view_item_style_options) const {

////    QStyleOptionButton check_box_style_option;
////    QRect check_box_rect = QApplication::style()->subElementRect( QStyle::SE_CheckBoxIndicator, &check_box_style_option);
////    QPoint check_box_point(view_item_style_options.rect.x() + view_item_style_options.rect.width() / 2 - check_box_rect.width() / 2,
////                           view_item_style_options.rect.y() + view_item_style_options.rect.height() / 2 -
////                           check_box_rect.height() / 2);
////    return QRect(check_box_point, check_box_rect.size());
//}

//void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

////    if (index.column() == 6) {
////        int value = index.model()->data(index, Qt::DisplayRole).toInt();

////        QStyleOptionButton check_box_style_option;
////        check_box_style_option.state |= QStyle::State_Enabled;
////        if (value == 1) {
////            check_box_style_option.state |= QStyle::State_On;
////        } else {
////            check_box_style_option.state |= QStyle::State_Off;
////        }
////        check_box_style_option.rect = ComboBoxDelegate::CheckBoxRect(option);

////        QApplication::style()->drawControl(QStyle::CE_CheckBox, &check_box_style_option, painter);
////    } else
//        QStyledItemDelegate::paint(painter, option, index);
//}

void ComboBoxDelegate::setList(const QStringList &list)
{
    listItem  = list;
}
