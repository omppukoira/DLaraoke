#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "medialibraryedit.h"
#include "settingsedit.h"
#include "dlaraoke.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QFile>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Libraries(new CMediaFileLibraries())
{
    m_pFiles = new CMediaFileModel();
    m_pFiles->setMediaLibraries(m_Libraries);
    m_pSortModel = new QSortFilterProxyModel();

    ui->setupUi(this);
    ui->tvMediaFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvMediaFiles->setSortingEnabled(true);
    ui->tvMediaFiles->setModel(m_pSortModel);
    m_pSortModel->setSourceModel(m_pFiles);

    m_pInfo = new QLabel();
    ui->sbMain->addWidget(m_pInfo, 1);

    connect(ui->actionScan_media_library, SIGNAL(triggered()), this, SLOT(scanMediaFiles()));
    connect(ui->actionRead_media_data, SIGNAL(triggered()), this, SLOT(readMediaData()));
    connect(ui->actionWrite_media_data , SIGNAL(triggered()), this, SLOT(writeMediaData()));
    connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actionMedia_Libraries, SIGNAL(triggered()), this, SLOT(editMediaLibraries()));
    connect(ui->actionEdit_Settings, SIGNAL(triggered()), this, SLOT(editSettings()));

    connect(ui->tvMediaFiles, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playFile(QModelIndex)));
    connect(ui->leSearchText, SIGNAL(textChanged(QString)), this, SLOT(searchTextChange(QString)));

    // Event filters
    ui->tvMediaFiles->installEventFilter(this);
    ui->leSearchText->installEventFilter(this);

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pSortModel;
    delete m_pFiles;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

void MainWindow::readSettings()
{

    DLaraoke::settings().beginGroup("MainWindow");
    ui->tvMediaFiles->setColumnWidth(CMediaFileModel::MFM_Artist,  DLaraoke::settings().value("Artist_Len", QVariant(200)).toInt());
    ui->tvMediaFiles->setColumnWidth(CMediaFileModel::MFM_Title,   DLaraoke::settings().value("Title_Len",  QVariant(350)).toInt());
    ui->tvMediaFiles->setColumnWidth(CMediaFileModel::MFM_Type,    DLaraoke::settings().value("Type_Len",   QVariant(70)).toInt());
    DLaraoke::settings().endGroup();
}

void MainWindow::writeSettings()
{
    DLaraoke::settings().beginGroup("MainWindow");
    DLaraoke::settings().setValue("Artist_Len",  ui->tvMediaFiles->columnWidth(CMediaFileModel::MFM_Artist));
    DLaraoke::settings().setValue("Title_Len",   ui->tvMediaFiles->columnWidth(CMediaFileModel::MFM_Title));
    DLaraoke::settings().setValue("Type_Len",    ui->tvMediaFiles->columnWidth(CMediaFileModel::MFM_Type));
    DLaraoke::settings().endGroup();
    if(DLaraoke::settings().value("Color/Global Unique", "").toString().isEmpty())
        DLaraoke::settings().setValue("Color/Global Unique", 0);
    if(DLaraoke::settings().value("Color/Library Unique", "").toString().isEmpty())
        DLaraoke::settings().setValue("Color/Library Unique", 0);
    if(DLaraoke::settings().value("MediaFile/md5Size", "").toString().isEmpty())
        DLaraoke::settings().setValue("MediaFile/md5Size", 1024*1024*100);
    if(DLaraoke::settings().value("MediaFile/Show Global Unique", "").toString().isEmpty())
        DLaraoke::settings().setValue("MediaFile/Show Global Unique", 1);
    if(DLaraoke::settings().value("MediaFile/Show Library Unique", "").toString().isEmpty())
        DLaraoke::settings().setValue("MediaFile/Show Library Unique", 1);
    if(DLaraoke::settings().value("MediaFile/Read audio tracks", "").toString().isEmpty())
        DLaraoke::settings().setValue("MediaFile/Read audio tracks", 0);

}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->tvMediaFiles)              // Messages from tvMediaFiles
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Tab)
            {
                ui->leSearchText->setFocus();
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            {
                playFile(ui->tvMediaFiles->currentIndex());
                return true;
            }
        }

    if(target == ui->leSearchText)          // Messages from leSearchText
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
            {
                ui->tvMediaFiles->setFocus();
                return true;
            }
        }
        else if(event->type() == QEvent::FocusIn)
        {
            QPalette pal = ui->leSearchText->palette();

            pal.setColor(ui->leSearchText->backgroundRole(), Qt::yellow);
            ui->leSearchText->setPalette(pal);
        }
        else if(event->type() == QEvent::FocusOut)
        {
            QPalette pal = ui->leSearchText->palette();

            pal.setColor(ui->leSearchText->backgroundRole(), Qt::white);
            ui->leSearchText->setPalette(pal);
        }
    }
    return QMainWindow::eventFilter(target, event);
}

bool MainWindow::appendReadCancel()
{
    if(m_Libraries->count() > 0)
    {
        int ret = QMessageBox::question(this, tr("Media files already in memory"),
                    tr("Clear current files from memory"), QMessageBox::Yes | QMessageBox::No  | QMessageBox::Cancel);

        if(ret == QMessageBox::Yes)
            m_Libraries->clear();
        else if(ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::setFilters()
{
    m_pFiles->resetModel();

    m_pSortModel->sort(CMediaFileModel::MFM_Title);   // Sort by title name
    m_pSortModel->setFilterKeyColumn(-1);             // Search from all columns
    m_pSortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void MainWindow::scanMediaFiles()
{
    appendReadCancel();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        QSharedPointer<CMediaFileLibrary> tmpLib = QSharedPointer<CMediaFileLibrary>(new CMediaFileLibrary());
        tmpLib->setShowUniqueFiles(false);
        tmpLib->scanMediaFiles(dir, true, NULL);
        if(tmpLib->count() <= 0)
        {
            QMessageBox::warning(this, tr("Media files"), tr("No media files found"));
            return;
        }
        else
        {
            QSharedPointer<CMediaFileLibrary> lib = QSharedPointer<CMediaFileLibrary>(new CMediaFileLibrary());
            QProgressDialog progress("Scanning media files...", "Cancel", 0, tmpLib->count(), this);
            progress.setWindowModality(Qt::WindowModal);

            lib->scanMediaFiles(dir, false, &progress);
            if(lib->count() == tmpLib->count())
            {
                m_Libraries->addMediaLibrary(lib);
                setFilters();
            }
            else
            {
                QMessageBox::warning(this, "Scan media files", "Meida file scan cancelled");
            }
        }
    }
}

void MainWindow::readMediaData()
{
    appendReadCancel();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Media library"), "/home", tr("XML files (*.xml)"));
    QFile fileIn(fileName);
    QXmlStreamReader reader;

    if(!fileIn.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Open Media library"), "Failed to open file");
        return;
    }

    reader.setDevice(&fileIn);
    m_Libraries->readMediaLibraries(&reader);
    setFilters();
}

void MainWindow::writeMediaData()
{
    if(m_pFiles->rowCount(QModelIndex()) <= 0)
    {
        QMessageBox::information(this, tr("Write data"), tr("No media files loaded, nothing to write"));
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(this, tr("Save media data"),
                                    QDir::homePath(), tr("XML files (*.xml)"));
    if(!fileOut.isEmpty())
    {
        QFile file(fileOut);
        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::warning(this, tr("Write data"), tr("Unable to open file for writing"));
            return;
        }

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);

        m_Libraries->writeMediaLibraries(&writer);
    }
}

void MainWindow::playFile(const QModelIndex &index)
{
    QModelIndex file;                                   // File being clicked

    file = m_pSortModel->mapToSource(index);            // Remap from sorted and searched filter
    if(m_player.state() != QProcess::NotRunning)
    {
        m_player.kill();
    }
    m_player.start(m_Libraries->getMediaFile(file.row())->getExecCmd());
}

void MainWindow::searchTextChange(QString szText)
{
    ui->tvMediaFiles->clearSelection();
    m_pSortModel->setFilterFixedString(szText);
}

void MainWindow::editMediaLibraries()
{
    MediaLIbraryEdit *dlg = new MediaLIbraryEdit(m_Libraries);

    dlg->exec();
}

void MainWindow::editSettings()
{
    SettingsEdit *dlg = new SettingsEdit("Duo Liukko", "Karaoke");

    dlg->exec();
}




































