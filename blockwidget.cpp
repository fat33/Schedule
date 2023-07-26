#include "blockwidget.h"

class BlockWidget::Private
{
public:
    Private()
        : x( 0 ),
          y( 0 )
    {}
    // координаты
    int x, y;
    // список виджетов
    QList<QWidget*> widgets;
    // список пояснений
    QStringList texts;
    // функция для отрисовки текста
    void drawText(QPainter &painter, const QString &text , const QSize &size);
    // функция для установки координат
    void setCoordinates();
};

BlockWidget::BlockWidget( QWidget *parent ) :
    QWidget( parent ),
    d(new Private())
{
    // устанавливаем такой же размер, как и у родителя
    resize( parent->size() );
    raise();
    parent->installEventFilter(this);
}

void BlockWidget::addWidget( QWidget *w, QString text )
{
    // устанавливаем текущий виджет, обновляем геометрию и показываем виджет
    d->widgets.append( w );
    d->texts.append( text );

    update();
    show();
}

void BlockWidget::clear()
{
    //сбрасываем указатель и скрываем
    d->widgets.removeFirst();
    d->texts.removeFirst();
    hide();
}

void BlockWidget::paintEvent( QPaintEvent *pe )
{
    // если есть виджет
    if( !d->widgets.isEmpty() )
    {
        // устанавливаем координаты
        d->setCoordinates();
        // затемняем фон(полупрозрачным), кроме текущего виджета
        QPainter painter( this );
        QPainterPath path;
        path.addRect(pe->rect());
        QPainterPath widgetPath;
        widgetPath.addRoundedRect(d->x, d->y,
                                  d->widgets.first()->width(),
                                  d->widgets.first()->height(),
                                  5,
                                  5);
        path = path.subtracted(widgetPath);
        QColor bgColor = Qt::black;
        bgColor.setAlpha(200);
        painter.setPen( QPen(Qt::white) );
        painter.fillPath(path, bgColor);
        // отображаем текст подсказки
        d->drawText( painter, d->texts.first(), size() );
        setFocus();
    }
    else
        hide();
}

void BlockWidget::mousePressEvent( QMouseEvent *mpe )
{
    // если нажимаем в область виджета и несколько элементов,
    // то удаялем и тем самым переходим к следующему
    // иначе скрываем
    if (mpe->button() & Qt::LeftButton){
        int x = mpe->x();
        int y = mpe->y();
        if( ((x > d->x) && (x < d->x+d->widgets.first()->width()) )
                && ((y > d->y) && (y < d->y+d->widgets.first()->height())) )
        {
            if( !d->widgets.isEmpty() )
            {
                d->widgets.removeFirst();
                d->texts.removeFirst();
                update();
            }
            else
                hide();
        }
    }
}

void BlockWidget::keyPressEvent( QKeyEvent *ke )
{
    switch( ke->key() )
    {
        case Qt::Key_Escape:
            d->widgets.clear();
            d->texts.clear();
            update();
            break;
        default:
            break;
    }
    QWidget::keyPressEvent( ke );
}

bool BlockWidget::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Resize && object == parentWidget()) {
        QResizeEvent *ev = static_cast<QResizeEvent*>(event);
        resize(ev->size());
    }
    return false;
}

void BlockWidget::Private::drawText( QPainter &painter, const QString &text, const QSize &size )
{
    // определяяем ширину текста
    QFontMetrics fm( painter.font() );
    int tWidth = fm.width( text );
    // если есть место справа
    if( size.width()-(widgets.first()->width()+this->x+10) > tWidth )
    {
        painter.drawText( (widgets.first()->width() + this->x + 10),
                          (this->y + widgets.first()->height() / 2), text );
        return;
    }
    // если нет места справа, но есть слева
    else
        if (this->x - 10 > tWidth)
        {
            painter.drawText( (this->x - tWidth - 10), (this->y + widgets.first()->height() / 2), text );
            return;
        }
        else
            if( size.height()-this->y+widgets.first()->height() > fm.height() )
            {
                painter.drawText( this->x + 10, (this->y+widgets.first()->height()+10+fm.height()), text);
                return;
            }

    // если нет места ни справа, ни слева, то делим текст на несколько строк
    QStringList source = text.split( QLatin1String(" ") );
    QStringList lines;
    QString line;
    int space = 0, x = 5, y = 10;
    // если справа больше места чем слева
    if( size.width()-(widgets.first()->width()+this->x+10) > this->x-10 )
    {
        space = size.width() - (widgets.first()->width()+this->x+10);
        x = widgets.first()->width() + this->x + 10;
    }
    else
    {
        space = this->x - 10;
        x = 5;
    }
    // разделяем строку на несколько других строк, если не умещается
    while( !source.isEmpty() )
    {
        QString buf = line;
        line.append( source.first() + QLatin1String(" ") );
        // если слово умещается в строку
        if( fm.width(line) < space )
        {
            if( source.count() == 1 )
                lines.append( line );
            source.removeFirst();
        }
        else
        {
            lines.append( buf );
            line.clear();
            line.append( source.first() + QLatin1String(" ") );
            if( source.count() == 1 )
                lines.append( source.first() );
            source.removeFirst();
        }
    }
    // получаем количество строк
    int linesHeight = lines.count() * fm.height();
    // если текст умещается на одном уровне с виджетом
    if( linesHeight < widgets.first()->height() )
        y = this->y + (widgets.first()->height() - linesHeight) / 2;
    // если текст умещается снизу
    else
        if( size.height() - this->y > linesHeight )
            y = this->y + widgets.first()->height() / 2;
    // если умещается сверху
        else
            if( this->y + widgets.first()->height() > linesHeight )
                y = this->y + widgets.first()->height() - linesHeight;

    for( int i=0; i<lines.count(); i++ )
    {
        painter.drawText( x, y+i*fm.height(), lines.at(i) );
    }
}

void BlockWidget::Private::setCoordinates()
{
    x = widgets.first()->x();
    y = widgets.first()->y();
    QWidget *w = widgets.first();
    while( w->parent()->parent() )
    {
        w = static_cast<QWidget*>( w->parent() );
        x += w->x();
        y += w->y();
    }
    w = 0;
    delete( w );
}
