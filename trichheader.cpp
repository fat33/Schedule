#include <QDebug>
//#include <QColor>
#include <QPointer>
#include <QPainter>
#include <QAbstractItemModel>

#include "trichheader.h"

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
TRichHeaderModel::TRichHeaderModel()
{
//    fillHeaderModel( _horizontalHeaderModel );
//    fillHeaderModel( _verticalHeaderModel );
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeaderModel::clear()
{
    HeaderModel.clear();

    Header.clear();
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeaderModel::addColumn( const QString& key, const QString& caption, const QString& parent )
{
    TColumn column;
    column.key = key;
    column.name = caption;
    column.parent = parent;

    column.item = new QStandardItem( column.name );

    Header[ column.key ] = column;

    if( parent.isEmpty() || !Header.contains( parent ) )
        HeaderModel.appendColumn( QList<QStandardItem*>() << column.item );
    else
        Header[ parent ].item->appendColumn( QList<QStandardItem*>() << column.item );
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeaderModel::rowCount( const QModelIndex& ) const
{
    return depth("");
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeaderModel::columnCount( const QModelIndex& ) const
{
    return cols("");
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeaderModel::depth( const QString& key ) const
{
    int my_d = -1;
    for( THeader::const_iterator it = Header.begin(); it != Header.end(); ++it )
    {
        if( it.value().parent != key ) continue;

        int ch_d = depth( it.key() );
        if( my_d < ch_d ) my_d = ch_d;
    }
    return my_d+1;
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeaderModel::cols( const QString& key ) const
{
    int my_c = 0;
    for( THeader::const_iterator it = Header.begin(); it != Header.end(); ++it )
    {
        if( it.value().parent != key ) continue;

        my_c += cols( it.key() );
    }
    if( my_c ) return my_c;

    return 1;
}

//----------------------------------------------------------------------------------------------------------------------------
QVariant TRichHeaderModel::data( const QModelIndex& index, int role ) const
{
    if( role == TRichHeader::HorizontalHeaderDataRole )
    {
        QVariant v;
        v.setValue( (QObject*)&HeaderModel );
        return v;
    }
/*
    if( role==TRichHeader::VerticalHeaderDataRole )
    {
        QVariant v;
        v.setValue( (QObject*)&_verticalHeaderModel );
        return v;
    }
*/
    if( role==Qt::DisplayRole && index.isValid() )
    {
        return QString("index(%1, %2)").arg(index.row()).arg(index.column());
    }

    return QVariant();
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
class TRichHeader::private_data
{
    public:
        QPointer<QAbstractItemModel> headerModel;

        private_data()
        {}

        void initFromNewModel( QAbstractItemModel* model)
        {
            headerModel=QPointer<QAbstractItemModel>();
            QVariant v( model->data( QModelIndex(), HorizontalHeaderDataRole ) );
            if( v.isValid() )
                headerModel = qobject_cast<QAbstractItemModel*>(v.value<QObject*>());
        }

        QModelIndex findRootIndex(QModelIndex index) const
        {
            while( index.parent().isValid() )
                index=index.parent();
            return index;
        }

        QModelIndexList parentIndexes(QModelIndex index) const
        {
            QModelIndexList indexes;
            while(index.isValid())
            {
                indexes.push_front(index);
                index=index.parent();
            }
            return indexes;
        }

        QModelIndex findLeaf(const QModelIndex& curentIndex, int sectionIndex, int& curentLeafIndex)
        {
            if(curentIndex.isValid())
            {
                int childCount=curentIndex.model()->columnCount(curentIndex);
                if(childCount)
                {
                    for(int i=0; i<childCount; ++i)
                    {
                        QModelIndex res(findLeaf(curentIndex.child(0, i), sectionIndex, curentLeafIndex));
                        if(res.isValid())
                            return res;
                    }
                }
                else
                {
                    ++curentLeafIndex;
                    if(curentLeafIndex==sectionIndex)
                        return curentIndex;
                }
            }
            return QModelIndex();
        }

        QModelIndex leafIndex(int sectionIndex)
        {
            if(headerModel)
            {
                int curentLeafIndex=-1;
                for(int i=0; i<headerModel->columnCount(); ++i)
                {
                    QModelIndex res(findLeaf(headerModel->index(0, i), sectionIndex, curentLeafIndex));
                    if(res.isValid())
                        return res;
                }
            }
            return QModelIndex();
        }

        QModelIndexList searchLeafs(const QModelIndex& curentIndex) const
        {
            QModelIndexList res;
            if(curentIndex.isValid())
            {
                int childCount=curentIndex.model()->columnCount(curentIndex);
                if(childCount)
                {
                    for(int i=0; i<childCount; ++i)
                        res+=searchLeafs(curentIndex.child(0, i));
                }
                else
                {
                    res.push_back(curentIndex);
                }
            }
            return res;
        }

        QModelIndexList leafs(const QModelIndex& searchedIndex) const
        {
            QModelIndexList leafs;
            if(searchedIndex.isValid())
            {
                int childCount=searchedIndex.model()->columnCount(searchedIndex);
                for(int i=0; i<childCount; ++i)
                    leafs+=searchLeafs(searchedIndex.child(0, i));
            }
            return leafs;
        }

        void setForegroundBrush(QStyleOptionHeader& opt, const QModelIndex& index) const
        {
            QVariant foregroundBrush = index.data(Qt::ForegroundRole);
            if (QVariant(foregroundBrush).canConvert<QBrush>())
                opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));
        }

        void setBackgroundBrush(QStyleOptionHeader& opt, const QModelIndex& index) const
        {
            QVariant backgroundBrush = index.data(Qt::BackgroundRole);
            if (QVariant(backgroundBrush).canConvert<QBrush>())
            {
                opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
                opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
            }
        }

        QSize cellSize
            (const QModelIndex& leafIndex,
             const QHeaderView* hv,
             QStyleOptionHeader styleOptions) const
        {
            QSize res;
            QVariant variant(leafIndex.data(Qt::SizeHintRole));
            if (variant.isValid())
                res=qvariant_cast<QSize>(variant);
            QFont fnt(hv->font());
            QVariant var(leafIndex.data(Qt::FontRole));
            if (var.isValid() && QVariant(var).canConvert<QFont>())
                fnt = qvariant_cast<QFont>(var);
            fnt.setBold(true);
            QFontMetrics fm(fnt);
            QSize size(fm.size(0, leafIndex.data(Qt::DisplayRole).toString()));
            if(leafIndex.data(Qt::UserRole).isValid())
                size.transpose();
            QSize decorationsSize(hv->style()->sizeFromContents(QStyle::CT_HeaderSection, &styleOptions, QSize(), hv));
            QSize emptyTextSize(fm.size(0, ""));
            return res.expandedTo(size+decorationsSize-emptyTextSize);
        }

        int	currentCellWidth
            (const QModelIndex& searchedIndex,
             const QModelIndex& leafIndex,
             int sectionIndex,
             const QHeaderView* hv) const
        {
            QModelIndexList leafsList(leafs(searchedIndex));
            if(leafsList.empty())
                return hv->sectionSize(sectionIndex);
            int width=0;
            int firstLeafSectionIndex=sectionIndex-leafsList.indexOf(leafIndex);
            for(int i=0; i<leafsList.size(); ++i)
                width+=hv->sectionSize(firstLeafSectionIndex+i);
            return width;
        }

        int	currentCellLeft
            (const QModelIndex& searchedIndex,
             const QModelIndex& leafIndex,
             int sectionIndex,
             int left,
             const QHeaderView* hv) const
        {
            QModelIndexList leafsList(leafs(searchedIndex));
            if(!leafsList.empty())
            {
                int n=leafsList.indexOf(leafIndex);
                int firstLeafSectionIndex=sectionIndex-n;
                --n;
                for(; n>=0; --n)
                    left-=hv->sectionSize(firstLeafSectionIndex+n);
            }
            return left;
        }

        int paintHorizontalCell
            (QPainter *painter,
             const QHeaderView* hv,
             const QModelIndex& cellIndex,
             const QModelIndex& leafIndex,
             int logicalLeafIndex,
             const QStyleOptionHeader& styleOptions,
             const QRect& sectionRect,
             int top) const
        {
            QStyleOptionHeader uniopt(styleOptions);
            setForegroundBrush(uniopt, cellIndex);
            setBackgroundBrush(uniopt, cellIndex);

            int height=cellSize(cellIndex, hv, uniopt).height();
            if(cellIndex==leafIndex)
                height=sectionRect.height()-top;
            int left=currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.left(), hv);
            int width=currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

            QRect r(left, top, width, height);

            uniopt.text = cellIndex.data(Qt::DisplayRole).toString();
            painter->save();
            uniopt.rect = r;
            if(cellIndex.data(Qt::UserRole).isValid())
            {
                hv->style()->drawControl(QStyle::CE_HeaderSection, &uniopt, painter, hv);
                QMatrix m;
                m.rotate(-90);
                painter->setWorldMatrix(m, true);
                QRect new_r(0, 0,  r.height(), r.width());
                new_r.moveCenter(QPoint(-r.center().y(), r.center().x()));
                uniopt.rect = new_r;
                hv->style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, painter, hv);
            }
            else
            {
                hv->style()->drawControl(QStyle::CE_Header, &uniopt, painter, hv);
            }
            painter->restore();
            return top+height;
        }

        void paintHorizontalSection
            (QPainter *painter,
             const QRect& sectionRect,
             int logicalLeafIndex,
             const QHeaderView* hv,
             const QStyleOptionHeader& styleOptions,
             const QModelIndex& leafIndex) const
        {
            QPointF oldBO(painter->brushOrigin());
            int top=sectionRect.y();
            QModelIndexList indexes(parentIndexes(leafIndex));
            for(int i=0; i<indexes.size(); ++i)
            {
                QStyleOptionHeader realStyleOptions(styleOptions);
                if(   i<indexes.size()-1
                    &&
                      (   realStyleOptions.state.testFlag(QStyle::State_Sunken)
                       || realStyleOptions.state.testFlag(QStyle::State_On)))
                {
                    QStyle::State t(QStyle::State_Sunken | QStyle::State_On);
                    realStyleOptions.state&=(~t);
                }
                top=paintHorizontalCell(painter,
                               hv,
                               indexes[i],
                               leafIndex,
                               logicalLeafIndex,
                               realStyleOptions,
                               sectionRect,
                               top);
            }
            painter->setBrushOrigin(oldBO);
        }

        int paintVerticalCell
            (QPainter *painter,
             const QHeaderView* hv,
             const QModelIndex& cellIndex,
             const QModelIndex& leafIndex,
             int logicalLeafIndex,
             const QStyleOptionHeader& styleOptions,
             const QRect& sectionRect,
             int left) const
        {
            QStyleOptionHeader uniopt(styleOptions);
            setForegroundBrush(uniopt, cellIndex);
            setBackgroundBrush(uniopt, cellIndex);

            int width=cellSize(cellIndex, hv, uniopt).width();
            if(cellIndex==leafIndex)
                width=sectionRect.width()-left;
            int top=currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.top(), hv);
            int height=currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

            QRect r(left, top, width, height);

            uniopt.text = cellIndex.data(Qt::DisplayRole).toString();
            painter->save();
            uniopt.rect = r;
            if(cellIndex.data(Qt::UserRole).isValid())
            {
                hv->style()->drawControl(QStyle::CE_HeaderSection, &uniopt, painter, hv);
                QMatrix m;
                m.rotate(-90);
                painter->setWorldMatrix(m, true);
                QRect new_r(0, 0,  r.height(), r.width());
                new_r.moveCenter(QPoint(-r.center().y(), r.center().x()));
                uniopt.rect = new_r;
                hv->style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, painter, hv);
            }
            else
            {
                hv->style()->drawControl(QStyle::CE_Header, &uniopt, painter, hv);
            }
            painter->restore();
            return left+width;
        }

        void paintVerticalSection
            (QPainter *painter,
             const QRect& sectionRect,
             int logicalLeafIndex,
             const QHeaderView* hv,
             const QStyleOptionHeader& styleOptions,
             const QModelIndex& leafIndex) const
        {
            QPointF oldBO(painter->brushOrigin());
            int left=sectionRect.x();
            QModelIndexList indexes(parentIndexes(leafIndex));
            for(int i=0; i<indexes.size(); ++i)
            {
                QStyleOptionHeader realStyleOptions(styleOptions);
                if(   i<indexes.size()-1
                    &&
                      (   realStyleOptions.state.testFlag(QStyle::State_Sunken)
                       || realStyleOptions.state.testFlag(QStyle::State_On)))
                {
                    QStyle::State t(QStyle::State_Sunken | QStyle::State_On);
                    realStyleOptions.state&=(~t);
                }
                left=paintVerticalCell(painter,
                               hv,
                               indexes[i],
                               leafIndex,
                               logicalLeafIndex,
                               realStyleOptions,
                               sectionRect,
                               left);
            }
            painter->setBrushOrigin(oldBO);
        }
};

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
TRichHeader::TRichHeader(QWidget *parent)
    : QHeaderView(Qt::Horizontal, parent)
    , _pd( new private_data() )
{
    connect( this, SIGNAL(sectionResized(int, int, int)), this, SLOT(on_sectionResized(int)));

    HeaderModel = new TRichHeaderModel();
    setModel( HeaderModel );
}

//----------------------------------------------------------------------------------------------------------------------------
TRichHeader::~TRichHeader()
{
    delete _pd;
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeader::rowCount() const
{
    return HeaderModel->rowCount( QModelIndex() );
}

//----------------------------------------------------------------------------------------------------------------------------
int TRichHeader::numCols() const
{
    return HeaderModel->columnCount( QModelIndex() );
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeader::clear()
{
    HeaderModel->clear();
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeader::addColumn( const QString& key, const QString& caption, const QString& parent )
{
    HeaderModel->addColumn( key, caption, parent );
}

//----------------------------------------------------------------------------------------------------------------------------
QStyleOptionHeader TRichHeader::styleOptionForCell(int logicalInd) const
{
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    if (window()->isActiveWindow())
        opt.state |= QStyle::State_Active;
    opt.textAlignment = Qt::AlignCenter;
    opt.iconAlignment = Qt::AlignVCenter;
    opt.section = logicalInd;

    int visual = visualIndex(logicalInd);

    if( count() == 1 )
        opt.position = QStyleOptionHeader::OnlyOneSection;
    else
    {
        if( visual == 0 )
            opt.position = QStyleOptionHeader::Beginning;
        else
            opt.position=(visual==count()-1 ? QStyleOptionHeader::End : QStyleOptionHeader::Middle);
    }
#ifndef QT5_VERSION
    if( isClickable() )
    {
        if( highlightSections() && selectionModel() )
        {
            if( orientation()==Qt::Horizontal )
            {
                if(selectionModel()->columnIntersectsSelection(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_On;
                if(selectionModel()->isColumnSelected(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_Sunken;
            }
            else
            {
                if(selectionModel()->rowIntersectsSelection(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_On;
                if(selectionModel()->isRowSelected(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_Sunken;
            }
        }
    }
#endif
    if( selectionModel() )
    {
        bool previousSelected=false;
        if( orientation()==Qt::Horizontal )
            previousSelected = selectionModel()->isColumnSelected(logicalIndex(visual - 1), rootIndex());
        else
            previousSelected = selectionModel()->isRowSelected(logicalIndex(visual - 1), rootIndex());

        bool nextSelected=false;
        if(orientation()==Qt::Horizontal)
            nextSelected = selectionModel()->isColumnSelected(logicalIndex(visual + 1), rootIndex());
        else
            nextSelected = selectionModel()->isRowSelected(logicalIndex(visual + 1), rootIndex());
        if(previousSelected && nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        else
        {
            if (previousSelected)
                opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
            else
            {
                if (nextSelected)
                    opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
                else
                    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
            }
        }
    }

    return opt;
}

//----------------------------------------------------------------------------------------------------------------------------
QSize TRichHeader::sectionSizeFromContents(int logicalIndex) const
{
    if(_pd->headerModel)
    {
        QModelIndex curLeafIndex(_pd->leafIndex(logicalIndex));
        if(curLeafIndex.isValid())
        {
            QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
            QSize s(_pd->cellSize(curLeafIndex, this, styleOption));
            curLeafIndex=curLeafIndex.parent();
            while(curLeafIndex.isValid())
            {
                if(orientation() == Qt::Horizontal)
                    s.rheight()+=_pd->cellSize(curLeafIndex, this, styleOption).height();
                else
                    s.rwidth()+=_pd->cellSize(curLeafIndex, this, styleOption).width();
                curLeafIndex=curLeafIndex.parent();
            }
            return s;
        }
    }
    return QHeaderView::sectionSizeFromContents(logicalIndex);
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if( !painter->isActive() ) return;

    if( rect.isValid() )
    {
        QModelIndex leafIndex(_pd->leafIndex(logicalIndex));
        if(leafIndex.isValid())
        {
            if(orientation() == Qt::Horizontal)
                _pd->paintHorizontalSection(painter, rect, logicalIndex, this, styleOptionForCell(logicalIndex), leafIndex);
            else
                _pd->paintVerticalSection(painter, rect, logicalIndex, this, styleOptionForCell(logicalIndex), leafIndex);
            return;
        }
    }
    QHeaderView::paintSection(painter, rect, logicalIndex);
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeader::on_sectionResized( int logicalIndex )
{
    if( isSectionHidden(logicalIndex) )
        return;

    QModelIndex leafIndex( _pd->leafIndex(logicalIndex) );
    if( leafIndex.isValid() )
    {
        QModelIndexList leafsList(_pd->leafs(_pd->findRootIndex(leafIndex)));
        for(int n=leafsList.indexOf(leafIndex); n>0; --n)
        {
            --logicalIndex;

            int w = viewport()->width();
            int h = viewport()->height();
            int pos = sectionViewportPosition(logicalIndex);
            QRect r(pos, 0, w - pos, h);
            if(orientation() == Qt::Horizontal)
            {
                if (isRightToLeft())
                    r.setRect(0, 0, pos + sectionSize(logicalIndex), h);
            }
            else
                r.setRect(0, pos, w, h - pos);

            viewport()->update(r.normalized());
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------
void TRichHeader::setModel(QAbstractItemModel* model)
{
    _pd->initFromNewModel( model );
    QHeaderView::setModel( model );
    int cnt = model->columnCount();

    if( cnt ) initializeSections( 0, cnt-1 );
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
