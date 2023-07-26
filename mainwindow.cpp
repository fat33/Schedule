#include "about.h"
#include "hierarchicalheaderview.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>

#include <QTextBrowser>
#include <QTextDocument>
#include <QTextTableCell>

#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#endif
#endif

#include <qdebug.h>

inline QStringList getList(const QListWidget *listWidget)
{
    QStringList list;
    for(int i = 0; i < listWidget->count(); ++i)
        list.append(listWidget->item(i)->data(Qt::DisplayRole).toString());
    return list;
}

inline void addItem(QListWidget *listWidget, QString text)
{
    QList<QListWidgetItem *> listCount = listWidget->findItems(text, Qt::MatchContains);
    if(listCount.count() > 0)
    {
        if(QMessageBox::No == QMessageBox::question(listWidget,
                                                    QObject::tr("Add the term "),
                                                    QObject::tr("The term: %0, is in the list, add?").arg(text)))
            return;
    }
    listWidget->addItem(text);

}

inline void delItem(QListWidget *listWidget)
{
    int row = listWidget->currentRow();
    if(row >= 0)
        listWidget->takeItem(row);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = nullptr;
    comboBoxDelegateGroup = nullptr;
    comboBoxDelegateObject = nullptr;

    HierarchicalHeaderView * ierarchicalHeaderView = new HierarchicalHeaderView(Qt::Vertical, ui->tableView);
    ierarchicalHeaderView->setHighlightSections(true);
    //ierarchicalHeaderView->setClickable(true);

    ui->tableView->setVerticalHeader(ierarchicalHeaderView);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    loadIniFile();
}

MainWindow::~MainWindow()
{
    saveIniFile();
    delete ui;
}


void MainWindow::loadIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/schedule/schedule.ini"), QSettings::IniFormat);
    setGeometry( settings.value("MainWin/geometry").toRect());
    ui->splitter->restoreState(settings.value( "MainWin/splitter").toByteArray());
    ui->splitter_2->restoreState(settings.value( "MainWin/splitter_2").toByteArray());

    pahtOpenDialog = settings.value("MainWin/pahtOpenDialog", QDir::home().absolutePath()).toString();
    currentFile = settings.value("MainWin/currentFile").toString();
    if(!currentFile.isEmpty())
    {
        loadSchedule(currentFile);
    }
    else
    {
        on_actionNew_triggered();
    }
}

void MainWindow::saveIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/schedule/schedule.ini"), QSettings::IniFormat);
    settings.setValue  ("MainWin/geometry", geometry() );
    settings.setValue ( "MainWin/splitter", ui->splitter->saveState());
    settings.setValue ( "MainWin/splitter_2", ui->splitter_2->saveState());

    settings.setValue ( "MainWin/pahtOpenDialog", pahtOpenDialog );
    settings.setValue ( "MainWin/currentFile", currentFile );
}

QStringList MainWindow::listGroup()
{
    return getList(ui->groupListWidget);
}

QStringList MainWindow::listObjects()
{
    return getList(ui->objectsListWidget);
}

void MainWindow::loadSchedule(QString fileName)
{
    ui->objectsListWidget->clear();
    ui->groupListWidget->clear();

    QByteArray array;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        array = file.readAll();
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Error open file"),
                              tr("\"%1\" - the file is not available to reading")
                              .arg(fileName));
        return;
    }
    file.close();

    QStringList _listGroup;
    QStringList _listObjects;

    QJsonObject bodyObject = QJsonDocument::fromJson(array).object();
    QJsonValue valueJson = bodyObject.value("Object");
    if(!valueJson.isUndefined() && !valueJson.isNull())
    {
        _listObjects = valueJson.toVariant().toString().split(",");
        ui->objectsListWidget->addItems(_listObjects);
    }

    valueJson = bodyObject.value("Group");
    if(!valueJson.isUndefined() && !valueJson.isNull())
    {
        _listGroup = valueJson.toVariant().toString().split(",");
        ui->groupListWidget->addItems(_listGroup);
    }

    if(model) delete model;
    model = new ScheduleModel(this, listGroup(), listObjects());

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->hide();


    if(comboBoxDelegateGroup) delete comboBoxDelegateGroup;
    if(comboBoxDelegateObject) delete comboBoxDelegateObject;
    comboBoxDelegateGroup = new ComboBoxDelegate(this, listGroup());
    comboBoxDelegateObject = new ComboBoxDelegate(this, listObjects());
    ui->tableView->setItemDelegateForRow(0, comboBoxDelegateGroup);

    for(int i = 1; i < 13; ++i)
    {
        ui->tableView->setItemDelegateForRow(i, comboBoxDelegateObject);
    }


    int currColumn = 0; // groupList.count();
    int currRow = 0;    //13;
    QJsonArray arrayJson = bodyObject.value("Array").toArray();
    foreach (QJsonValue value, arrayJson)
    {
        QString item = value.toString();
        QModelIndex index = model->index(currRow, currColumn, QModelIndex());
        model->setData(index, item);

        ++currColumn;
        if(currColumn == listGroup().count())
        {
            currColumn = 0;
            ++currRow;
        }
    }
}

QString MainWindow::getHtml()
{
    QString text;
    const long rowCount = ui->tableView->model()->rowCount();
    const long columnCount = ui->tableView->model()->columnCount();
    QStringList list;
    list << tr("<td bkcolor=0 colspan=\"2\">№</td><td bkcolor=0>Time-Group</td>");
    list << "<td bkcolor=0 rowspan=\"2\">1</td><td bkcolor=0>1</td><td bkcolor=0>8:00-8:45</td>";
    list << "<td bkcolor=0>2</td><td bkcolor=0>8:50-9:35</td>";
    list << "<td bkcolor=0 rowspan=\"2\">2</td><td bkcolor=0>3</td><td bkcolor=0>9:55-10:40</td>";
    list << "<td bkcolor=0>4</td><td bkcolor=0>11:00-11:45</td>";
    list << "<td bkcolor=0 rowspan=\"2\">3</td><td bkcolor=0>5</td><td bkcolor=0>12:05-12:50</td>";
    list << "<td bkcolor=0>6</td><td bkcolor=0>12:55-13:40</td>";
    list << "<td bkcolor=0 rowspan=\"2\">4</td><td bkcolor=0>7</td><td bkcolor=0>14:00-14:45</td>";
    list << "<td bkcolor=0>8</td><td bkcolor=0>14:50-15:35</td>";
    list << "<td bkcolor=0 rowspan=\"2\">5</td><td bkcolor=0>9</td><td bkcolor=0>15:55-16:40</td>";
    list << "<td bkcolor=0>10</td><td bkcolor=0>17:00-17:45</td>";
    list << "<td bkcolor=0 rowspan=\"2\">6</td><td bkcolor=0>11</td><td bkcolor=0>17:55-18:40</td>";
    list << "<td bkcolor=0>12</td><td bkcolor=0>18:45-19:30</td>";

    text += "<html>\n";
    text += "<head>\n";
    text += "<meta Content=\"Text/html; charset=Windows-1251\">\n";
    text += "<title>Schedule</title>\n";
    text += "</head>\n";
    text += "<body bgcolor=#ffffff link=#5000A0>\n";
    text += "<table border=1 cellspacing=0 cellpadding=2>\n";;

    for (long row = 0; row < rowCount; row++) {
        text +=  "<tr>";
        text += list.at(row);

        for (long column = 0; column < columnCount; column++) {
            if (!ui->tableView->isColumnHidden(column)) {
                QString data = ui->tableView->model()->data(ui->tableView->model()->index(row, column)).toString().simplified();
                text += QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        text += "</tr>\n";
    }
    text += "</table>\n";
    text += "</body>\n";
    text += "</html>\n";

    return text;
}

void MainWindow::on_actionNew_triggered()
{
    currentFile = "";
    ui->objectsListWidget->clear();
    ui->groupListWidget->clear();

    if(model) delete model;
    model = new ScheduleModel(this, listGroup(), listObjects());

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->hide();

    if(comboBoxDelegateGroup) delete comboBoxDelegateGroup;
    if(comboBoxDelegateObject) delete comboBoxDelegateObject;
    comboBoxDelegateGroup = new ComboBoxDelegate(this, listGroup());
    comboBoxDelegateObject = new ComboBoxDelegate(this, listObjects());
    ui->tableView->setItemDelegateForRow(0, comboBoxDelegateGroup);

    for(int i = 1; i < 13; ++i)
    {
        ui->tableView->setItemDelegateForRow(i, comboBoxDelegateObject);
    }

}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Schedule"),
                                                    pahtOpenDialog, tr("Schedule (*.conf)"));
    if(fileName.isEmpty()) return;
    if(!QFile(fileName).exists())
    {
        QMessageBox::critical(this, tr("Error open file"),
                              tr("\"%1\" - not exist").arg(fileName));
        return;
    }

    pahtOpenDialog = QFileInfo(fileName).absolutePath();
    currentFile = fileName;
    loadSchedule(currentFile);
}

void MainWindow::on_actionSave_triggered()
{
    if(currentFile.isEmpty())
    {
        on_actionSaveAs_triggered();
    }
    else
    {
        QByteArray array = model->save();

        QFile file(currentFile);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            file.write(array);
        }
        file.close();
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Schedule"),
                                                    pahtOpenDialog, tr("Schedule (*.conf)"));
    if(fileName.isEmpty()) return;

    QFileInfo f(fileName);
    QDir dir = f.absoluteDir();
    if(!dir.exists())
    {
        QMessageBox::critical(this, tr("Error open dir"),
                              tr("\"%1\" - not exist").arg(dir.absolutePath()));
        return;
    }
    QString ext = f.completeSuffix();
    if(ext.isEmpty())
        fileName += ".conf";

    currentFile = fileName;
    pahtOpenDialog = QFileInfo(fileName).absolutePath();

    QByteArray array = model->save();

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(array);
    }
    file.close();
}

void MainWindow::on_actionPrint_triggered()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QTextDocument *document = new QTextDocument();
    document->setHtml(getHtml());
    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    if (dialog->exec() == QDialog::Accepted) {
        document->print(&printer);
    }
#endif
}

void MainWindow::on_actionExport_triggered()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printpreviewdialog)
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Schedule (PDF)"),
                                                    pahtOpenDialog, tr("Schedule (*.pdf)"));
    if(fileName.isEmpty()) return;

    QFileInfo f(fileName);
    QDir dir = f.absoluteDir();
    if(!dir.exists())
    {
        QMessageBox::critical(this, tr("Error open dir"),
                              tr("\"%1\" - not exist").arg(dir.absolutePath()));
        return;
    }
    QString ext = f.completeSuffix();
    if(ext.isEmpty())
        fileName += ".pdf";

    pahtOpenDialog = QFileInfo(fileName).absolutePath();


    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(getHtml());
    doc.setPageSize(printer.pageRect().size());
    doc.print(&printer);
#endif
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_addGroupPB_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Group"),
                                         tr("Group:"), QLineEdit::Normal,
                                         tr("Group"), &ok);
    if (ok && !text.isEmpty())
    {
        addItem(ui->groupListWidget, text);
        model->setGroupList(listGroup());
        comboBoxDelegateGroup->setList(listGroup());
    }
}

void MainWindow::on_delGroupPB_clicked()
{
    delItem(ui->groupListWidget);
    model->setGroupList(listGroup());
    comboBoxDelegateGroup->setList(listGroup());
}

void MainWindow::on_addObjectsPB_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Objects"),
                                         tr("Object:"), QLineEdit::Normal,
                                         tr("Object"), &ok);
    if (ok && !text.isEmpty())
    {
        addItem(ui->objectsListWidget, text);
        model->setObjectList(listObjects());
        comboBoxDelegateObject->setList(listObjects());
    }
}

void MainWindow::on_delObjectsPB_clicked()
{
    delItem(ui->objectsListWidget);
    model->setObjectList(listObjects());
    comboBoxDelegateObject->setList(listObjects());
}

void MainWindow::on_actionAbout_triggered()
{
    About *a = new About(this);
    a->setAttribute(Qt::WA_DeleteOnClose);
    a->show();
}


