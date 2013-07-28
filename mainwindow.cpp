#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsedit.h"
#include "dlaraoke.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QFile>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QtGlobal>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pLibrary(NULL)
{
    m_pFiles = new CMediaModel();
    m_pSortModel = new QSortFilterProxyModel();
    m_pLibrary = new CMediaLibrary();

    ui->setupUi(this);
    ui->tvMediaFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvMediaFiles->setSortingEnabled(true);
    ui->tvMediaFiles->setModel(m_pSortModel);
    m_pSortModel->setSourceModel(m_pFiles);

    // ui->treeMediaFiles->setModel(m_pDetailFiles);
    m_pInfo = new QLabel();
    ui->sbMain->addWidget(m_pInfo, 1);

    connect(ui->actionScan_media_library, SIGNAL(triggered()), this, SLOT(scanMediaFiles()));
    connect(ui->actionRead_media_data, SIGNAL(triggered()), this, SLOT(readMediaData()));
    connect(ui->actionWrite_media_data , SIGNAL(triggered()), this, SLOT(writeMediaData()));
    connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actionEdit_Settings, SIGNAL(triggered()), this, SLOT(editSettings()));

    connect(ui->tvMediaFiles, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playFile(QModelIndex)));
    connect(ui->leSearchText, SIGNAL(textChanged(QString)), this, SLOT(searchTextChange(QString)));

    // Event filters
    ui->tvMediaFiles->installEventFilter(this);
    ui->leSearchText->installEventFilter(this);
    ui->treeMediaFiles->installEventFilter(this);

    readSettings();

    QString mainLib = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    mainLib += QDir::separator();
    mainLib += "mainLib.xml";
    if(QFile::exists(mainLib))
        readMediaData(mainLib);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pSortModel;
    delete m_pFiles;
    delete m_pLibrary;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

void MainWindow::readSettings()
{

    DLaraoke::settings().beginGroup("MainWindow");
    ui->tvMediaFiles->setColumnWidth(CMediaModel::MFM_Artist,  DLaraoke::settings().value("Artist_Len", QVariant(200)).toInt());
    ui->tvMediaFiles->setColumnWidth(CMediaModel::MFM_Title,   DLaraoke::settings().value("Title_Len",  QVariant(350)).toInt());
    ui->tvMediaFiles->setColumnWidth(CMediaModel::MFM_Type,    DLaraoke::settings().value("Type_Len",   QVariant(70)).toInt());
    DLaraoke::settings().endGroup();
}

void MainWindow::writeSettings()
{
    DLaraoke::settings().beginGroup("MainWindow");
    DLaraoke::settings().setValue("Artist_Len",  ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Artist));
    DLaraoke::settings().setValue("Title_Len",   ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Title));
    DLaraoke::settings().setValue("Type_Len",    ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Type));
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
    if(target == ui->tvMediaFiles)              // Messages from main tree view
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

            if((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) && keyEvent->modifiers() == Qt::ControlModifier)
            {
                ui->stwMain->setCurrentIndex((ui->stwMain->currentIndex()+1)%2 );
            }
        }

    if(target == ui->treeMediaFiles)
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) && keyEvent->modifiers() == Qt::ControlModifier)
            {
                ui->stwMain->setCurrentIndex((ui->stwMain->currentIndex()+1)%2 );
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
            else if((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) && keyEvent->modifiers() == Qt::ControlModifier)
            {
                ui->stwMain->setCurrentIndex((ui->stwMain->currentIndex()+1)%2 );
            }
        }
        else if(event->type() == QEvent::FocusIn)
        {
            QPalette pal = ui->leSearchText->palette();

            pal.setColor(ui->leSearchText->backgroundRole(), QColor("#DDAA00") );
            ui->leSearchText->setPalette(pal);
            ui->leSearchText->selectAll();
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

void MainWindow::setFilters()
{
    if(m_pLibrary != NULL)
        qDebug() << "setFilters for lib ["<< m_pLibrary->getName() <<"]";
    else
        qDebug("setFilters for no lib");

    m_pFiles->setMediaLibrary(m_pLibrary);

    m_pSortModel->sort(CMediaModel::MFM_Title);         // Sort by title name
    m_pSortModel->setFilterKeyColumn(-1);               // Search from all columns
    m_pSortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // m_pDetailFiles->setMediaLibrary(m_Library);
}

void MainWindow::scanMediaFiles()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        CMediaLibrary *pTmpLib = new CMediaLibrary();
        pTmpLib->scanMediaFiles(dir, true, NULL);
        if(pTmpLib->count() <= 0)
        {
            QMessageBox::warning(this, tr("Media files"), tr("No media files found"));
            delete pTmpLib;
            return;
        }
        else
        {
            QProgressDialog progress("Scanning media files...", "Cancel", 0, pTmpLib->count(), this);
            progress.setModal(true);
            progress.setMinimumDuration(0);             // Force QProgressDialog to be shown
            progress.setValue(0);
            DLaraoke::processEvents();

            pTmpLib->clear();
            pTmpLib->scanMediaFiles(dir, false, &progress);
            if(!progress.wasCanceled())
            {
                if(m_pLibrary != NULL) delete m_pLibrary;
                m_pLibrary = pTmpLib;
                m_pLibrary->setName(dir);
                qDebug("Scanned %d media files", m_pLibrary->count());
                setFilters();
            }
            else
            {
                QMessageBox::warning(this, "Scan media files", "Meida file scan cancelled");
                setFilters();
                delete pTmpLib;
            }
        }
    }
}

void MainWindow::readMediaData()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Media library"), QDir::homePath(), tr("XML files (*.xml)"));
    if(!fileName.isEmpty())
    {
        readMediaData(fileName);
    }
}

void MainWindow::readMediaData(QString fileName)
{
    QFile fileIn(fileName);
    QXmlStreamReader reader;

    if(!fileIn.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Read Media library"), tr("Failed to open file [%1]").arg(fileName));
        return;
    }

    reader.setDevice(&fileIn);
    while(!reader.atEnd() && !(reader.isStartElement() && reader.name() == CMediaLibrary::NOD_MEDIA_LIBRARY))
        reader.readNext();
    if(reader.atEnd()) return;

    m_pLibrary->readXmlData(&reader);
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
        writeMediaData(fileOut);
    }
}

void MainWindow::writeMediaData(QString fileName)
{
    QFile file(fileName);

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Write data"), tr("Unable to open file for writing"));
        return;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    m_pLibrary->writeXmlData(&writer);
}

void MainWindow::playFile(const QModelIndex &index)
{
    QModelIndex file;                                   // File being clicked
    CMediaFile *pFile;

    file = m_pSortModel->mapToSource(index);            // Remap from sorted and searched filter
    if(m_player.state() != QProcess::NotRunning)
    {
        m_player.kill();
        m_player.waitForFinished();
    }
    pFile = m_pLibrary->getMediaFile(file.row());
    if(pFile != NULL)
    {
        m_player.start(pFile->getExecCmd());
    }
    else
    {
        QMessageBox::warning(this, tr("Play file"), tr("Mediafile not found"));
    }
}

void MainWindow::searchTextChange(QString szText)
{
    ui->tvMediaFiles->clearSelection();
    m_pSortModel->setFilterFixedString(szText);
}

void MainWindow::editSettings()
{
    SettingsEdit *dlg = new SettingsEdit("Duo Liukko", "Karaoke");

    dlg->exec();
}




































