/*
Copyright (c) 2009, Krasnoshchekov Petr
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Krasnoshchekov Petr ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Krasnoshchekov Petr BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "hierarchicalheaderview.h"

#include <QAbstractItemModel>
#include <QHelpEvent>
#include <QPainter>
#include <QPointer>
#include <QToolTip>
#include <QWhatsThis>

class HierarchicalHeaderView::HierarchicalHeaderViewPrivate {
public:
    QPointer<QAbstractItemModel> headerModel;
    int hover;
    int pressed;

    void initFromNewModel(int orientation, QAbstractItemModel *model)
    {
        headerModel = QPointer<QAbstractItemModel>();
        if(model) {
            QVariant v(model->data(QModelIndex(),
                                   (orientation == Qt::Horizontal ? HorizontalHeaderDataRole
                                                                  : VerticalHeaderDataRole)));
            if(v.isValid())
                headerModel = qobject_cast<QAbstractItemModel *>(qvariant_cast<QObject *>(v));
        }
    }

    QModelIndex findRootIndex(QModelIndex index) const
    {
        while(index.parent().isValid())
            index = index.parent();
        return index;
    }

    QModelIndexList parentIndexes(QModelIndex index) const
    {
        QModelIndexList indexes;
        while(index.isValid()) {
            indexes.push_front(index);
            index = index.parent();
        }
        return indexes;
    }

    QModelIndex findLeaf(const QModelIndex &curentIndex, int sectionIndex,
                         int &curentLeafIndex) const
    {
        if(curentIndex.isValid()) {
            int childCount = curentIndex.model()->columnCount(curentIndex);
            if(childCount) {
                for(int i = 0; i < childCount; ++i) {
                    QModelIndex res(findLeaf(curentIndex.model()->index(0, i, curentIndex),
                                             sectionIndex, curentLeafIndex));
                    if(res.isValid())
                        return res;
                }
            }
            else {
                ++curentLeafIndex;
                if(curentLeafIndex == sectionIndex)
                    return curentIndex;
            }
        }
        return QModelIndex();
    }

    QModelIndex leafIndex(int sectionIndex) const
    {
        if(headerModel) {
            int curentLeafIndex = -1;
            for(int i = 0; i < headerModel->columnCount(); ++i) {
                QModelIndex res(findLeaf(headerModel->index(0, i), sectionIndex, curentLeafIndex));
                if(res.isValid())
                    return res;
            }
        }
        return QModelIndex();
    }

    QModelIndexList searchLeafs(const QModelIndex &curentIndex) const
    {
        QModelIndexList res;
        if(curentIndex.isValid()) {
            int childCount = curentIndex.model()->columnCount(curentIndex);
            if(childCount) {
                for(int i = 0; i < childCount; ++i)
                    res += searchLeafs(curentIndex.model()->index(0, i, curentIndex));
            }
            else {
                res.push_back(curentIndex);
            }
        }
        return res;
    }

    QModelIndexList leafs(const QModelIndex &searchedIndex) const
    {
        QModelIndexList leafs;
        if(searchedIndex.isValid()) {
            int childCount = searchedIndex.model()->columnCount(searchedIndex);
            for(int i = 0; i < childCount; ++i)
                leafs += searchLeafs(searchedIndex.model()->index(0, i, searchedIndex));
        }
        return leafs;
    }

    void setForegroundBrush(QStyleOptionHeader &opt, const QModelIndex &index) const
    {
        QVariant foregroundBrush = index.data(Qt::ForegroundRole);
        if(foregroundBrush.canConvert<QBrush>())
            opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));
    }

    void setBackgroundBrush(QStyleOptionHeader &opt, const QModelIndex &index) const
    {
        QVariant backgroundBrush = index.data(Qt::BackgroundRole);
        if(backgroundBrush.canConvert<QBrush>()) {
            opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
            opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
        }
    }

    QSize cellSize(const QModelIndex &leafIndex, const QHeaderView *hv,
                   QStyleOptionHeader styleOptions) const
    {
        QSize res;
        QVariant variant(leafIndex.data(Qt::SizeHintRole));
        if(variant.isValid())
            res = qvariant_cast<QSize>(variant);
        QFont fnt(hv->font());
        QVariant var(leafIndex.data(Qt::FontRole));
        if(var.isValid() && var.canConvert<QFont>())
            fnt = qvariant_cast<QFont>(var);
        fnt.setBold(true);
        QFontMetrics fm(fnt);
        QSize size(fm.size(0, leafIndex.data(Qt::DisplayRole).toString()));
        if(leafIndex.data(Qt::UserRole).isValid())
            size.transpose();
        QSize decorationsSize(hv->style()->sizeFromContents(QStyle::CT_HeaderSection, &styleOptions,
                                                            QSize(), hv));
        QSize emptyTextSize(fm.size(0, QLatin1String("")));
        return res.expandedTo(size + decorationsSize - emptyTextSize);
    }

    int currentCellWidth(const QModelIndex &searchedIndex, const QModelIndex &leafIndex,
                         int sectionIndex, const QHeaderView *hv) const
    {
        QModelIndexList leafsList(leafs(searchedIndex));
        if(leafsList.empty())
            return hv->sectionSize(sectionIndex);
        int width = 0;
        int firstLeafSectionIndex = sectionIndex - leafsList.indexOf(leafIndex);
        for(int i = 0; i < leafsList.size(); ++i)
            width += hv->sectionSize(firstLeafSectionIndex + i);
        return width;
    }

    int currentCellLeft(const QModelIndex &searchedIndex, const QModelIndex &leafIndex,
                        int sectionIndex, int left, const QHeaderView *hv) const
    {
        QModelIndexList leafsList(leafs(searchedIndex));
        if(!leafsList.empty()) {
            int n = leafsList.indexOf(leafIndex);
            int firstLeafSectionIndex = sectionIndex - n;
            --n;
            for(; n >= 0; --n)
                left -= hv->sectionSize(firstLeafSectionIndex + n);
        }
        return left;
    }
    int paintCell(QPainter *painter, const QHeaderView *hv, const QModelIndex &cellIndex,
                  const QModelIndex &leafIndex, int logicalLeafIndex,
                  const QStyleOptionHeader &styleOptions, const QRect &sectionRect,
                  int topOrLeft) const
    {
        QStyleOptionHeader uniopt(styleOptions);
        setForegroundBrush(uniopt, cellIndex);
        setBackgroundBrush(uniopt, cellIndex);

        if(cellIndex != leafIndex) {
            uniopt.sortIndicator = QStyleOptionHeader::None;
            uniopt.state &= ~(QStyle::State_MouseOver);
        }

        QRect r = cellRect(cellIndex, leafIndex, logicalLeafIndex, sectionRect, hv, uniopt,
                           topOrLeft);

        uniopt.text = cellIndex.data(Qt::DisplayRole).toString();

        QVariant alignmentData = cellIndex.data(Qt::TextAlignmentRole);
        if(alignmentData.isValid()) {
            uniopt.textAlignment = Qt::Alignment(qvariant_cast<int>(alignmentData));
        }
        else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
            int textWidth = uniopt.fontMetrics.horizontalAdvance(uniopt.text);
#else
            int textWidth = uniopt.fontMetrics.width(uniopt.text);
#endif
            if(textWidth >=
                    r.width() - textMargin(cellIndex, leafIndex, logicalLeafIndex, hv, &uniopt))
                uniopt.textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        }
        painter->save();
        uniopt.rect = r;
        if(cellIndex.data(Qt::UserRole).isValid()) {
            hv->style()->drawControl(QStyle::CE_HeaderSection, &uniopt, painter, hv);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
            QTransform m;
            m.rotate(-90);
            painter->setWorldTransform(m, true);
#else
            QMatrix m;
            m.rotate(-90);
            painter->setWorldMatrix(m, true);
#endif
            QRect new_r(0, 0, r.height(), r.width());
            new_r.moveCenter(QPoint(-r.center().y(), r.center().x()));
            uniopt.rect = new_r;
            hv->style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, painter, hv);
        }
        else {
            hv->style()->drawControl(QStyle::CE_Header, &uniopt, painter, hv);
        }
        painter->restore();
        return topOrLeft + (hv->orientation() == Qt::Horizontal ? r.height() : r.width());
    }
    void paintSection(QPainter *painter, const QRect &sectionRect, int logicalLeafIndex,
                      const QHeaderView *hv, const QStyleOptionHeader &styleOptions,
                      const QModelIndex &leafIndex) const
    {
        QPointF oldBO(painter->brushOrigin());
        int topOrLeft = (hv->orientation() == Qt::Horizontal) ? sectionRect.y() : sectionRect.x();
        QModelIndexList indexes(parentIndexes(leafIndex));
        for(int i = 0; i < indexes.size(); ++i) {
            QStyleOptionHeader realStyleOptions(styleOptions);
            if(i < indexes.size() - 1 &&
                    (realStyleOptions.state.testFlag(QStyle::State_Sunken) ||
                     realStyleOptions.state.testFlag(QStyle::State_On) ||
                     realStyleOptions.state.testFlag(QStyle::State_MouseOver))) {
                QStyle::State t(QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver);
                realStyleOptions.state &= (~t);
            }
            topOrLeft = paintCell(painter, hv, indexes[i], leafIndex, logicalLeafIndex,
                                  realStyleOptions, sectionRect, topOrLeft);
        }
        painter->setBrushOrigin(oldBO);
    }

    bool helpEvent(QHelpEvent *event, const QModelIndex &cellIndex, QHeaderView *hv) const
    {
        switch(event->type()) {
        case QEvent::ToolTip: {
            QVariant variant = cellIndex.data(Qt::ToolTipRole);
            if(variant.isValid()) {
                QToolTip::showText(event->globalPos(), variant.toString(), hv);
                return true;
            }
            break;
        }
        case QEvent::StatusTip: {
            QString statustip = cellIndex.data(Qt::StatusTipRole).toString();
            if(!statustip.isEmpty()) {
                hv->setStatusTip(statustip);
            }
            return true;
        }
        case QEvent::QueryWhatsThis: return cellIndex.data(Qt::WhatsThisRole).isValid();
        case QEvent::WhatsThis: {
            QVariant variant = cellIndex.data(Qt::WhatsThisRole);
            if(variant.isValid()) {
                QWhatsThis::showText(event->globalPos(), variant.toString(), hv);
                return true;
            }
            break;
        }
        default: break;
        }
        return false;
    }
    int textMargin(const QModelIndex &cellIndex, const QModelIndex &leafIndex, int logicalIndex,
                   const QHeaderView *hv, const QStyleOptionHeader *opt) const
    {
        QRect labelRect = hv->style()->subElementRect(QStyle::SE_HeaderLabel, opt, hv);
        int margin = labelRect.x() * 2;

        if(cellIndex == leafIndex && logicalIndex == hv->sortIndicatorSection()) {
            QRect arrowRect = hv->style()->subElementRect(QStyle::SE_HeaderArrow, opt, hv);
            int headerMargin = hv->style()->pixelMetric(QStyle::PM_HeaderMargin, opt, hv);
            margin += arrowRect.width() + headerMargin;
        }
        return margin;
    }
    QRect cellRect(const QModelIndex &cellIndex, const QModelIndex &leafIndex, int logicalLeafIndex,
                   const QRect &sectionRect, const QHeaderView *hv,
                   const QStyleOptionHeader &styleOption, int topOrLeft) const
    {
        switch(hv->orientation()) {
        case Qt::Horizontal: {
            int height = cellSize(cellIndex, hv, styleOption).height();
            if(cellIndex == leafIndex)
                height = sectionRect.height() - topOrLeft;
            int left = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex,
                                       sectionRect.left(), hv);
            int width = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

            return QRect(left, topOrLeft, width, height);
        }

        case Qt::Vertical: {
            int width = cellSize(cellIndex, hv, styleOption).width();
            if(cellIndex == leafIndex)
                width = sectionRect.width() - topOrLeft;
            int top = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.top(),
                                      hv);
            int height = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

            return QRect(topOrLeft, top, width, height);
        }
        default: break;
        }
        return QRect();
    }
    inline bool reverse(const QHeaderView *hv) const
    {
        return hv->orientation() == Qt::Horizontal && hv->isRightToLeft();
    }
    int sectionHandleAt(int position, const QHeaderView *q) const
    {
        int visual = q->visualIndexAt(position);
        if(visual == -1)
            return -1;
        int log = q->logicalIndex(visual);
        int pos = q->sectionViewportPosition(log);
        int grip = q->style()->pixelMetric(QStyle::PM_HeaderGripMargin, 0, q);

        bool atLeft = position < pos + grip;
        bool atRight = (position > pos + q->sectionSize(log) - grip);
        if(reverse(q))
            qSwap(atLeft, atRight);

        if(atLeft) {
            // grip at the beginning of the section
            while(visual > -1) {
                int logical = q->logicalIndex(--visual);
                if(!q->isSectionHidden(logical))
                    return logical;
            }
        }
        else if(atRight) {
            // grip at the end of the section
            return log;
        }
        return -1;
    }
};

HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , d(new HierarchicalHeaderViewPrivate)
{
    d->hover = d->pressed = -1;
    connect(this, SIGNAL(sectionResized(int, int, int)), this, SLOT(on_sectionResized(int)));
}

HierarchicalHeaderView::~HierarchicalHeaderView()
{
}

QStyleOptionHeader HierarchicalHeaderView::styleOptionForCell(int logicalInd) const
{
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    if(window()->isActiveWindow())
        opt.state |= QStyle::State_Active;
    opt.textAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
    opt.iconAlignment = Qt::AlignVCenter;
    opt.section = logicalInd;

    if(isSortIndicatorShown() && sortIndicatorSection() == logicalInd)
        opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                ? QStyleOptionHeader::SortDown
                : QStyleOptionHeader::SortUp;

    int visual = visualIndex(logicalInd);

    if(count() == 1)
        opt.position = QStyleOptionHeader::OnlyOneSection;
    else {
        if(visual == 0)
            opt.position = QStyleOptionHeader::Beginning;
        else
            opt.position =
                    (visual == count() - 1 ? QStyleOptionHeader::End : QStyleOptionHeader::Middle);
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if(sectionsClickable()) {
#else
    if(isClickable()) {
#endif
        if(logicalInd == d->hover)
            opt.state |= QStyle::State_MouseOver;
        if(logicalInd == d->pressed) {
            opt.state |= QStyle::State_Sunken;
        }
        else {
            if(highlightSections() && selectionModel()) {
                if(orientation() == Qt::Horizontal) {
                    if(selectionModel()->columnIntersectsSelection(logicalInd, rootIndex()))
                        opt.state |= QStyle::State_On;
                    if(selectionModel()->isColumnSelected(logicalInd, rootIndex()))
                        opt.state |= QStyle::State_Sunken;
                }
                else {
                    if(selectionModel()->rowIntersectsSelection(logicalInd, rootIndex()))
                        opt.state |= QStyle::State_On;
                    if(selectionModel()->isRowSelected(logicalInd, rootIndex()))
                        opt.state |= QStyle::State_Sunken;
                }
            }
        }
    }
    if(selectionModel()) {
        bool previousSelected = false;
        if(orientation() == Qt::Horizontal)
            previousSelected =
                    selectionModel()->isColumnSelected(logicalIndex(visual - 1), rootIndex());
        else
            previousSelected =
                    selectionModel()->isRowSelected(logicalIndex(visual - 1), rootIndex());
        bool nextSelected = false;
        if(orientation() == Qt::Horizontal)
            nextSelected =
                    selectionModel()->isColumnSelected(logicalIndex(visual + 1), rootIndex());
        else
            nextSelected = selectionModel()->isRowSelected(logicalIndex(visual + 1), rootIndex());
        if(previousSelected && nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        else {
            if(previousSelected)
                opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
            else {
                if(nextSelected)
                    opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
                else
                    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
            }
        }
    }
    return opt;
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    if(d->headerModel) {
        QModelIndex curLeafIndex(d->leafIndex(logicalIndex));
        if(curLeafIndex.isValid()) {
            QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
            QSize s(d->cellSize(curLeafIndex, this, styleOption));
            curLeafIndex = curLeafIndex.parent();
            while(curLeafIndex.isValid()) {
                if(orientation() == Qt::Horizontal)
                    s.rheight() += d->cellSize(curLeafIndex, this, styleOption).height();
                else
                    s.rwidth() += d->cellSize(curLeafIndex, this, styleOption).width();
                curLeafIndex = curLeafIndex.parent();
            }
            return s;
        }
    }
    return QHeaderView::sectionSizeFromContents(logicalIndex);
}

bool HierarchicalHeaderView::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent *>(event);
        d->hover = logicalIndexAt(he->pos());
        break;
    }

    case QEvent::HoverLeave: d->hover = -1; break;

    default: break;
    }
    return QHeaderView::event(event);
}

bool HierarchicalHeaderView::viewportEvent(QEvent *event)
{
    switch(event->type()) {
    case QEvent::ToolTip:
    case QEvent::StatusTip:
    case QEvent::QueryWhatsThis:
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
        int logicalIndex = logicalIndexAt(he->pos());
        if(logicalIndex != -1) {
            int topOrLeft = 0;
            QRect sectionRect(sectionPosition(logicalIndex), topOrLeft,
                              sectionSize(logicalIndex), height());
            QModelIndex leafIndex(d->leafIndex(logicalIndex));
            if(leafIndex.isValid()) {
                QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
                QModelIndexList indexes(d->parentIndexes(leafIndex));
                for(int i = 0; i < indexes.size(); ++i) {
                    QModelIndex cellIndex(indexes[i]);
                    QRect r = d->cellRect(cellIndex, leafIndex, logicalIndex, sectionRect, this,
                                          styleOption, topOrLeft);
                    if(r.contains(he->pos())) {
                        if(d->helpEvent(he, cellIndex, this))
                            return true;
                        break;
                    }

                    topOrLeft += orientation() == Qt::Horizontal ? r.height() : r.width();
                }
            }
        }
        break;
    }
    case QEvent::ContextMenu: d->pressed = -1; break;
    default: break;
    }
    return QHeaderView::viewportEvent(event);
}

void HierarchicalHeaderView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        int pos = orientation() == Qt::Horizontal ? event->x() : event->y();
        int handle = d->sectionHandleAt(pos, this);
        if(handle == -1) {
            d->pressed = logicalIndexAt(pos);
        }
    }
    QHeaderView::mousePressEvent(event);
}

void HierarchicalHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    d->pressed = -1;
    QHeaderView::mouseReleaseEvent(event);
}

void HierarchicalHeaderView::paintSection(QPainter *painter, const QRect &rect,
                                          int logicalIndex) const
{
    if(rect.isValid()) {
        QModelIndex leafIndex(d->leafIndex(logicalIndex));
        if(leafIndex.isValid()) {
            d->paintSection(painter, rect, logicalIndex, this, styleOptionForCell(logicalIndex),
                            leafIndex);
            return;
        }
    }
    QHeaderView::paintSection(painter, rect, logicalIndex);
}


void HierarchicalHeaderView::on_sectionResized(int logicalIndex)
{
    if(isSectionHidden(logicalIndex))
        return;

    QModelIndex leafIndex(d->leafIndex(logicalIndex));
    if(leafIndex.isValid()) {
        QModelIndexList leafsList(d->leafs(d->findRootIndex(leafIndex)));
        for(int n = leafsList.indexOf(leafIndex); n > 0; --n) {
            --logicalIndex;

            int w = viewport()->width();
            int h = viewport()->height();
            int pos = sectionViewportPosition(logicalIndex);
            QRect r(pos, 0, w - pos, h);
            if(orientation() == Qt::Horizontal) {
                if(isRightToLeft())
                    r.setRect(0, 0, pos + sectionSize(logicalIndex), h);
            }
            else
                r.setRect(0, pos, w, h - pos);

            viewport()->update(r.normalized());
        }
    }
}

void HierarchicalHeaderView::setModel(QAbstractItemModel *model)
{
    d->initFromNewModel(orientation(), model);
    QHeaderView::setModel(model);
    if(model) {
        int cnt = (orientation() == Qt::Horizontal ? model->columnCount() : model->rowCount());
        if(cnt)
            initializeSections(0, cnt - 1);
    }
}
