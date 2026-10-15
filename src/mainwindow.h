#ifndef SCHEDULE_MAINWINDOW_H
#define SCHEDULE_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ScheduleModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /// Opens a file given on the command line or restores the last one.
    void openInitialFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupActions();
    void setupConnections();
    void readSettings();
    void writeSettings() const;

    void newFile();
    void open();
    bool save();
    bool saveAs();
    void print();
    void exportPdf();
    void about();

    void addGroup();
    void removeGroup();
    void addSubject();
    void removeSubject();

    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void refreshLists();
    void updateActions();
    QString documentTitle() const;

    Ui::MainWindow *ui;
    ScheduleModel *m_model;
    QString m_currentFile;
    QString m_lastDirectory;
};

#endif // SCHEDULE_MAINWINDOW_H
