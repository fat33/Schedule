#include "comboboxdelegate.h"

#include <QComboBox>

ComboBoxDelegate::ComboBoxDelegate(int valuesRole, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_valuesRole(valuesRole)
{}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    const QVariant values = index.data(m_valuesRole);
    if (!values.isValid())
        return QStyledItemDelegate::createEditor(parent, option, index);

    auto *combo = new QComboBox(parent);
    combo->addItems(values.toStringList());

    // Commit as soon as the user picks a value instead of waiting for focus-out.
    connect(combo, QOverload<int>::of(&QComboBox::activated), this, [this, combo] {
        auto *self = const_cast<ComboBoxDelegate *>(this);
        emit self->commitData(combo);
        emit self->closeEditor(combo);
    });
    return combo;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (auto *combo = qobject_cast<QComboBox *>(editor)) {
        const int i = combo->findText(index.data(Qt::EditRole).toString());
        combo->setCurrentIndex(qMax(i, 0));
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if (auto *combo = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, combo->currentText(), Qt::EditRole);
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}
