#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "comboboxdelegate.h"
#include "schedulemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void loadIniFile();
    void saveIniFile();

    QStringList listGroup();
    QStringList listObjects();

    void loadSchedule(QString);

    QString getHtml();

private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionSaveAs_triggered();

    void on_actionPrint_triggered();

    void on_actionExport_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_addGroupPB_clicked();

    void on_delGroupPB_clicked();

    void on_addObjectsPB_clicked();

    void on_delObjectsPB_clicked();

private:
    Ui::MainWindow *ui;
    QString pahtOpenDialog;
    QString currentFile;
    ScheduleModel * model;
    ComboBoxDelegate * comboBoxDelegateGroup;
    ComboBoxDelegate * comboBoxDelegateObject;
};
#endif // MAINWINDOW_H
