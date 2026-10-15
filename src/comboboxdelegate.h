#ifndef SCHEDULE_COMBOBOXDELEGATE_H
#define SCHEDULE_COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

/**
 * Edits a cell with a combo box. The list of choices is taken from the model
 * on every edit (via the role passed to the constructor), so the delegate never
 * has to be recreated when the list of subjects changes.
 */
class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboBoxDelegate(int valuesRole, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private:
    int m_valuesRole;
};

#endif // SCHEDULE_COMBOBOXDELEGATE_H
