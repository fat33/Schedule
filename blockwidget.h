#ifndef BLOCKWIDGET_H
#define BLOCKWIDGET_H

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPaintEvent>
#include <QWidget>


class BlockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlockWidget( QWidget *parent=0 );

    void addWidget( QWidget *w, QString text=tr("Here must be hint text") );
    void clear();

protected:
    virtual void paintEvent( QPaintEvent *pe );
    virtual void mousePressEvent( QMouseEvent *mpe );
    virtual void keyPressEvent( QKeyEvent *ke );
    bool eventFilter(QObject *object, QEvent *event);

private:
    class Private;
    Private * const d;
};

#endif // BLOCKWIDGET_H
