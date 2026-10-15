/*
Copyright (c) 2009, Krasnoshchekov Petr
All rights reserved. BSD license, see LICENSE in this directory.
Modified for Qt 5.15 / Qt 6, see README.md in this directory.
*/

#ifndef HIERARCHICAL_HEADER_VIEW_H
#define HIERARCHICAL_HEADER_VIEW_H

#include <QHeaderView>

/**
Hierarchical header view for QTableView.
Header data is described by QAbstractItemModel.
Table model's function "data" should return pointer to QAbstractItemModel, representing
header structure,
for horizontal header it uses HorizontalHeaderDataRole, for vertical -
VerticalHeaderDataRole.
If header's model function "data" with Qt::UserRole return valid QVariant, text in header
cell is rotated.
*/
class HierarchicalHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    enum HeaderDataModelRoles {
        HorizontalHeaderDataRole = Qt::UserRole,
        VerticalHeaderDataRole = Qt::UserRole + 1
    };

    explicit HierarchicalHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~HierarchicalHeaderView() override;

    void setModel(QAbstractItemModel *model) override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;
    bool event(QEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void onSectionResized(int logicalIndex);
    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;

    class HierarchicalHeaderViewPrivate;
    HierarchicalHeaderViewPrivate *const d;
};

#endif
