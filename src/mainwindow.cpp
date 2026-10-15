#include "mainwindow.h"

#include "comboboxdelegate.h"
#include "hierarchicalheaderview.h"
#include "htmlexport.h"
#include "schedulemodel.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPrintDialog>
#include <QPrinter>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTextDocument>

namespace {

constexpr int StatusTimeoutMs = 4000;

QString fileFilter()
{
    return MainWindow::tr("Schedule (*.json *.conf);;All files (*)");
}

QString compilerName()
{
#if defined(__clang__)
    return QStringLiteral("Clang %1.%2.%3")
        .arg(__clang_major__)
        .arg(__clang_minor__)
        .arg(__clang_patchlevel__);
#elif defined(__GNUC__)
    return QStringLiteral("GCC %1.%2.%3")
        .arg(__GNUC__)
        .arg(__GNUC_MINOR__)
        .arg(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    return QStringLiteral("MSVC %1").arg(_MSC_VER);
#else
    return QStringLiteral("unknown compiler");
#endif
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new ScheduleModel(this))
{
    ui->setupUi(this);

    auto *header = new HierarchicalHeaderView(Qt::Vertical, ui->tableView);
    header->setHighlightSections(true);
    ui->tableView->setVerticalHeader(header);
    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->hide();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(110);
    ui->tableView->setItemDelegate(new ComboBoxDelegate(ScheduleModel::AllowedValuesRole, this));

    ui->mainSplitter->setStretchFactor(0, 4);
    ui->mainSplitter->setStretchFactor(1, 1);

    setupActions();
    setupConnections();
    readSettings();
    setCurrentFile(QString());
    updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void MainWindow::setupActions()
{
    struct ActionSetup
    {
        QAction *action;
        QKeySequence shortcut;
        const char *themeIcon;
        QStyle::StandardPixmap fallback;
    };

    const ActionSetup actions[] = {
        {ui->actionNew, QKeySequence::New, "document-new", QStyle::SP_FileIcon},
        {ui->actionOpen, QKeySequence::Open, "document-open", QStyle::SP_DialogOpenButton},
        {ui->actionSave, QKeySequence::Save, "document-save", QStyle::SP_DialogSaveButton},
        {ui->actionSaveAs, QKeySequence::SaveAs, "document-save-as", QStyle::SP_DialogSaveButton},
        {ui->actionPrint, QKeySequence::Print, "document-print", QStyle::SP_FileDialogDetailedView},
        {ui->actionExportPdf, QKeySequence(Qt::CTRL | Qt::Key_E), "document-export",
         QStyle::SP_ArrowDown},
        {ui->actionExit, QKeySequence::Quit, "application-exit", QStyle::SP_DialogCloseButton},
        {ui->actionAbout, QKeySequence(), "help-about", QStyle::SP_MessageBoxInformation},
    };

    for (const ActionSetup &a : actions) {
        if (!a.shortcut.isEmpty())
            a.action->setShortcut(a.shortcut);
        a.action->setIcon(
            QIcon::fromTheme(QLatin1String(a.themeIcon), style()->standardIcon(a.fallback)));
    }
}

void MainWindow::setupConnections()
{
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::print);
    connect(ui->actionExportPdf, &QAction::triggered, this, &MainWindow::exportPdf);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(ui->addGroupButton, &QPushButton::clicked, this, &MainWindow::addGroup);
    connect(ui->removeGroupButton, &QPushButton::clicked, this, &MainWindow::removeGroup);
    connect(ui->addSubjectButton, &QPushButton::clicked, this, &MainWindow::addSubject);
    connect(ui->removeSubjectButton, &QPushButton::clicked, this, &MainWindow::removeSubject);

    connect(ui->groupsList, &QListWidget::currentRowChanged, this, &MainWindow::updateActions);
    connect(ui->subjectsList, &QListWidget::currentRowChanged, this, &MainWindow::updateActions);

    connect(m_model, &ScheduleModel::contentChanged, this, [this] {
        setWindowModified(true);
        refreshLists();
        updateActions();
    });
}

void MainWindow::readSettings()
{
    QSettings settings;
    if (!restoreGeometry(settings.value(QStringLiteral("window/geometry")).toByteArray()))
        resize(1100, 680);
    restoreState(settings.value(QStringLiteral("window/state")).toByteArray());
    ui->mainSplitter->restoreState(
        settings.value(QStringLiteral("window/mainSplitter")).toByteArray());
    ui->sideSplitter->restoreState(
        settings.value(QStringLiteral("window/sideSplitter")).toByteArray());
    m_lastDirectory =
        settings
            .value(QStringLiteral("files/lastDirectory"),
                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
            .toString();
}

void MainWindow::writeSettings() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings.setValue(QStringLiteral("window/state"), saveState());
    settings.setValue(QStringLiteral("window/mainSplitter"), ui->mainSplitter->saveState());
    settings.setValue(QStringLiteral("window/sideSplitter"), ui->sideSplitter->saveState());
    settings.setValue(QStringLiteral("files/lastDirectory"), m_lastDirectory);
    settings.setValue(QStringLiteral("files/lastFile"), m_currentFile);
}

void MainWindow::openInitialFile(const QString &fileName)
{
    QString file = fileName;
    if (file.isEmpty())
        file = QSettings().value(QStringLiteral("files/lastFile")).toString();
    if (!file.isEmpty() && QFileInfo::exists(file))
        loadFile(file);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

// ---------------------------------------------------------------------------
// File operations
// ---------------------------------------------------------------------------

void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    m_model->clear();
    setCurrentFile(QString());
}

void MainWindow::open()
{
    if (!maybeSave())
        return;
    const QString fileName =
        QFileDialog::getOpenFileName(this, tr("Open Schedule"), m_lastDirectory, fileFilter());
    if (!fileName.isEmpty())
        loadFile(fileName);
}

bool MainWindow::save()
{
    // Legacy *.conf files are re-saved under a new name in the current format.
    if (m_currentFile.isEmpty() || m_currentFile.endsWith(QLatin1String(".conf")))
        return saveAs();
    return saveFile(m_currentFile);
}

bool MainWindow::saveAs()
{
    const QFileInfo current(m_currentFile);
    const QString suggested = m_currentFile.isEmpty()
                                  ? QString(m_lastDirectory + QStringLiteral("/schedule.json"))
                                  : QString(current.path() + QLatin1Char('/') +
                                            current.completeBaseName() + QStringLiteral(".json"));
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Schedule"), suggested, fileFilter());
    if (fileName.isEmpty())
        return false;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += QStringLiteral(".json");
    return saveFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Open Schedule"),
                             tr("Cannot read file \"%1\":\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QString error;
    if (!m_model->fromJson(file.readAll(), &error)) {
        QMessageBox::warning(
            this, tr("Open Schedule"),
            tr("Cannot load file \"%1\":\n%2").arg(QDir::toNativeSeparators(fileName), error));
        return false;
    }

    m_lastDirectory = QFileInfo(fileName).absolutePath();
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), StatusTimeoutMs);
    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    // QSaveFile writes to a temporary file and renames it on commit,
    // so a failure never leaves a half-written schedule on disk.
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly) || file.write(m_model->toJson()) < 0 || !file.commit()) {
        QMessageBox::warning(this, tr("Save Schedule"),
                             tr("Cannot write file \"%1\":\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    m_lastDirectory = QFileInfo(fileName).absolutePath();
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), StatusTimeoutMs);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!isWindowModified())
        return true;

    const auto answer = QMessageBox::warning(
        this, tr("Unsaved changes"), tr("The schedule has been modified.\nSave your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    switch (answer) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        return true;
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    setWindowModified(false);
    setWindowFilePath(fileName.isEmpty() ? tr("untitled") : fileName);
    refreshLists();
    updateActions();
}

QString MainWindow::documentTitle() const
{
    return m_currentFile.isEmpty() ? tr("Schedule") : QFileInfo(m_currentFile).completeBaseName();
}

// ---------------------------------------------------------------------------
// Printing / export
// ---------------------------------------------------------------------------

void MainWindow::print()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setDocName(documentTitle());

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QTextDocument document;
    document.setHtml(scheduleToHtml(*m_model, documentTitle()));
    document.print(&printer);
}

void MainWindow::exportPdf()
{
    const QString suggested =
        m_lastDirectory + QLatin1Char('/') + documentTitle() + QStringLiteral(".pdf");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), suggested,
                                                    tr("PDF documents (*.pdf)"));
    if (fileName.isEmpty())
        return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);
    printer.setDocName(documentTitle());

    QTextDocument document;
    document.setHtml(scheduleToHtml(*m_model, documentTitle()));
    document.print(&printer);

    m_lastDirectory = QFileInfo(fileName).absolutePath();
    statusBar()->showMessage(tr("Exported to %1").arg(QDir::toNativeSeparators(fileName)),
                             StatusTimeoutMs);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About %1").arg(QApplication::applicationDisplayName()),
                       tr("<h3>%1 %2</h3>"
                          "<p>Desktop editor for a school / college class timetable "
                          "with printing and PDF export.</p>"
                          "<table>"
                          "<tr><td>Qt:&nbsp;</td><td>%3 (built with %4)</td></tr>"
                          "<tr><td>Compiler:&nbsp;</td><td>%5</td></tr>"
                          "<tr><td>OS:&nbsp;</td><td>%6</td></tr>"
                          "</table>")
                           .arg(QApplication::applicationDisplayName(),
                                QApplication::applicationVersion(), QString::fromLatin1(qVersion()),
                                QStringLiteral(QT_VERSION_STR), compilerName(),
                                QSysInfo::prettyProductName()));
}

// ---------------------------------------------------------------------------
// Groups and subjects
// ---------------------------------------------------------------------------

void MainWindow::addGroup()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("Add Group"), tr("Group name:"),
                                               QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty())
        return;
    if (!m_model->addGroup(name)) {
        QMessageBox::information(this, tr("Add Group"),
                                 tr("Group \"%1\" already exists.").arg(name.simplified()));
        return;
    }
    ui->groupsList->setCurrentRow(ui->groupsList->count() - 1);
}

void MainWindow::removeGroup()
{
    const int row = ui->groupsList->currentRow();
    if (row < 0)
        return;

    const QString name = m_model->groups().at(row);
    const auto answer = QMessageBox::question(
        this, tr("Remove Group"), tr("Remove group \"%1\" together with its timetable?").arg(name));
    if (answer == QMessageBox::Yes)
        m_model->removeGroup(row);
}

void MainWindow::addSubject()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("Add Subject"), tr("Subject name:"),
                                               QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty())
        return;
    if (!m_model->addSubject(name)) {
        QMessageBox::information(this, tr("Add Subject"),
                                 tr("Subject \"%1\" already exists.").arg(name.simplified()));
        return;
    }
    ui->subjectsList->setCurrentRow(ui->subjectsList->count() - 1);
}

void MainWindow::removeSubject()
{
    QListWidgetItem *item = ui->subjectsList->currentItem();
    if (!item)
        return;

    const QString name = item->text();
    int usages = 0;
    for (int column = 0; column < m_model->columnCount(); ++column) {
        for (int i = 0; i < m_model->rowCount() - 1; ++i) {
            if (m_model->lesson(column, i) == name)
                ++usages;
        }
    }

    if (usages > 0) {
        const auto answer = QMessageBox::question(
            this, tr("Remove Subject"),
            tr("Subject \"%1\" is used in %n lesson(s). Remove it and clear those lessons?", "",
               usages)
                .arg(name));
        if (answer != QMessageBox::Yes)
            return;
    }
    m_model->removeSubject(name);
}

void MainWindow::refreshLists()
{
    auto sync = [](QListWidget *list, const QStringList &items) {
        const int row = list->currentRow();
        const QSignalBlocker blocker(list);
        list->clear();
        list->addItems(items);
        if (!items.isEmpty())
            list->setCurrentRow(qBound(0, row, static_cast<int>(items.size()) - 1));
    };
    sync(ui->groupsList, m_model->groups());
    sync(ui->subjectsList, m_model->subjects());
}

void MainWindow::updateActions()
{
    ui->removeGroupButton->setEnabled(ui->groupsList->currentRow() >= 0);
    ui->removeSubjectButton->setEnabled(ui->subjectsList->currentRow() >= 0);

    const bool hasContent = m_model->columnCount() > 0;
    ui->actionPrint->setEnabled(hasContent);
    ui->actionExportPdf->setEnabled(hasContent);
}
