#ifndef TRICHHEADER_H
#define TRICHHEADER_H

#include <QMap>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QAbstractTableModel>

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
class TRichHeaderModel : public QAbstractTableModel
{
    struct TColumn {
        QString key;
        QString name;
        QString parent;

        QStandardItem* item;
    };

public:
    TRichHeaderModel();

    void clear();
    void addColumn( const QString& key, const QString& caption, const QString& parent );

    virtual int rowCount( const QModelIndex& ) const;
    virtual int columnCount( const QModelIndex& ) const;

    QVariant data( const QModelIndex& index, int role ) const;

protected:
//    virtual void fillHeaderModel( QStandardItemModel& headerModel );
    int depth(const QString& key) const;
    int cols( const QString& key ) const;

private:
    QStandardItemModel HeaderModel;
//    QStandardItemModel _verticalHeaderModel;

    typedef QMap <QString, TColumn> THeader;
    THeader Header;
};

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
/**
 * TRichHeader* h = new TRichHeader( QTreeWidget* tw );
 * tw->setHeader( h );
 */
class TRichHeader : public QHeaderView
{
Q_OBJECT
public:
    class private_data;
    private_data* _pd;

    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;

    enum HeaderDataModelRoles {
        HorizontalHeaderDataRole = Qt::UserRole,
        VerticalHeaderDataRole = Qt::UserRole+1
    };

    TRichHeader( QWidget* parent = 0 );
    ~TRichHeader();

    void clear();
    void addColumn( const QString& key, const QString& caption, const QString& parent = "" );

    virtual int rowCount() const;
    virtual int numCols() const;

private slots:
    void on_sectionResized(int logicalIndex);

protected:
    void setModel( QAbstractItemModel* model );

    void paintSection( QPainter* painter, const QRect& rect, int logicalIndex ) const;
    QSize sectionSizeFromContents( int logicalIndex ) const;

private:
    TRichHeaderModel* HeaderModel;
};

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
#endif // TRICHHEADER_H
