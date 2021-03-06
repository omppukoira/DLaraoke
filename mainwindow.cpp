#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsedit.h"
#include "dlaraoke.h"
#include "ctool.h"
#include "medialibraryedit.h"
#include "playlistedit.h"
#include "playlistselect.h"

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
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pLibrary(NULL)
{
    m_pFiles = new CMediaModel();
    m_pSortModel = new MediaFilterProxyModel();
    m_pLibrary = new CMediaLibrary();

    m_pSortModel->setStars(4);
    m_pSortModel->setDynamicSortFilter(true);

    ui->setupUi(this);
    ui->tvMediaFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvMediaFiles->setSortingEnabled(true);
    ui->tvMediaFiles->setModel(m_pSortModel);
    m_pSortModel->setSourceModel(m_pFiles);

    QAction *play = new QAction("Play", ui->tvMediaFiles);
    QAction *rename = new QAction("Rename", ui->tvMediaFiles);
    QAction *addPlaylist = new QAction("Add to playlist...", ui->tvMediaFiles);
    QAction *setPlaylist = new QAction("Select playlist...", ui->tvMediaFiles);

    play->setShortcut(QKeySequence("Ctrl+P"));
    rename->setShortcut(Qt::Key_F2);
    addPlaylist->setShortcut(Qt::Key_F4);
    setPlaylist->setShortcut(Qt::Key_F8);

    ui->tvMediaFiles->addAction(play);
    ui->tvMediaFiles->addAction(rename);
    ui->tvMediaFiles->addAction(addPlaylist);
    ui->tvMediaFiles->addAction(setPlaylist);

    ui->menu_Edit->addAction(rename);

    ui->view_star_1->setShortcut(Qt::CTRL + Qt::Key_F1);
    ui->view_star_2->setShortcut(Qt::CTRL + Qt::Key_F2);
    ui->view_star_3->setShortcut(Qt::CTRL + Qt::Key_F3);
    ui->view_star_4->setShortcut(Qt::CTRL + Qt::Key_F4);
    ui->view_star_5->setShortcut(Qt::CTRL + Qt::Key_F5);

    connect(play,           SIGNAL(triggered()), this, SLOT(playCurrent()));
    connect(rename,         SIGNAL(triggered()), this, SLOT(renameCurrent()));
    connect(addPlaylist,    SIGNAL(triggered()), this, SLOT(addToPlaylist()));
    connect(setPlaylist,    SIGNAL(triggered()), this, SLOT(setPlaylist()));

    ui->tvMediaFiles->setContextMenuPolicy(Qt::ActionsContextMenu);

    // ui->treeMediaFiles->setModel(m_pDetailFiles);
    m_pInfo = new QLabel();
    ui->sbMain->addWidget(m_pInfo, 1);

    connect(ui->actionScan_media_library,   SIGNAL(triggered()), this, SLOT(scanMediaFiles()));
    connect(ui->actionRead_media_data,      SIGNAL(triggered()), this, SLOT(readMediaData()));
    connect(ui->actionWrite_media_data,     SIGNAL(triggered()), this, SLOT(writeMediaData()));
    connect(ui->actionWrite_playlist,       SIGNAL(triggered()), this, SLOT(writePalylist()));
    connect(ui->action_Exit,                SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actionEdit_Settings,        SIGNAL(triggered()), this, SLOT(editSettings()));

    connect(ui->tvMediaFiles,               SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playFile(QModelIndex)));
    connect(ui->leSearchText,               SIGNAL(textChanged(QString)), this, SLOT(searchTextChange(QString)));

    connect(ui->view_star_1,                SIGNAL(triggered()), this, SLOT(setViewStars()));
    connect(ui->view_star_2,                SIGNAL(triggered()), this, SLOT(setViewStars()));
    connect(ui->view_star_3,                SIGNAL(triggered()), this, SLOT(setViewStars()));
    connect(ui->view_star_4,                SIGNAL(triggered()), this, SLOT(setViewStars()));
    connect(ui->view_star_5,                SIGNAL(triggered()), this, SLOT(setViewStars()));

    connect(ui->action_Libraries,           SIGNAL(triggered()), this, SLOT(editMediaLibraries()));
    connect(ui->actionPlaylists,            SIGNAL(triggered()), this, SLOT(editPlaylists()));

    // Event filters
    ui->tvMediaFiles->installEventFilter(this);
    ui->leSearchText->installEventFilter(this);
    ui->treeMediaFiles->installEventFilter(this);

    readSettings();

    if(DLaraoke::settings().value("MainWindow/MediaLibrary", "").toString() != "")
    {
        readMediaData(DLaraoke::settings().value("MainWindow/MediaLibrary", "").toString());
    }
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
    ui->tvMediaFiles->setColumnWidth(CMediaModel::MFM_Date,    DLaraoke::settings().value("DateColumn", QVariant(95)).toInt());
    ui->tvMediaFiles->setColumnWidth(CMediaModel::MFM_Stars,   DLaraoke::settings().value("MediaStars", QVariant(80)).toInt());
    DLaraoke::settings().endGroup();
}

void MainWindow::writeSettings()
{
    DLaraoke::settings().beginGroup("MainWindow");
    DLaraoke::settings().setValue("Artist_Len",  ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Artist));
    DLaraoke::settings().setValue("Title_Len",   ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Title));
    DLaraoke::settings().setValue("Type_Len",    ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Type));
    DLaraoke::settings().setValue("DateColumn",  ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Date));
    DLaraoke::settings().setValue("MediaStars",  ui->tvMediaFiles->columnWidth(CMediaModel::MFM_Stars));
    DLaraoke::settings().setValue("MediaLibrary", m_pLibrary->getMediaLibraryFile());
    DLaraoke::settings().endGroup();
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

void MainWindow::updateViewPlaylists()
{
    QAction *pAction;
    CPlaylists::tPlaylists lists = m_pLibrary->getPlaylists()->getPlaylists();
    CPlaylists::tPlaylists::iterator itPL;
    int i;

    ui->menuPlaylists->clear();
    pAction = new QAction("Media library", ui->menuPlaylists);
    pAction->setShortcut(Qt::SHIFT + Qt::Key_F1);
    ui->menuPlaylists->addAction(pAction);

    connect(pAction,    SIGNAL(triggered()), this, SLOT(setViewLibrary()));

    i=2;
    for(itPL = lists.begin(); itPL != lists.end(); itPL++)
    {
        pAction = new QAction(itPL->second->name(), ui->menuPlaylists);
        switch(i)
        {
        case 2:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F2);
            break;
        case 3:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F3);
            break;
        case 4:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F4);
            break;
        case 5:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
            break;
        case 6:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F6);
            break;
        case 7:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F7);
            break;
        case 8:
            pAction->setShortcut(Qt::SHIFT + Qt::Key_F8);
            break;
        }

        ui->menuPlaylists->addAction(pAction);
        connect(pAction, SIGNAL(triggered()), this, SLOT(setViewPlaylist()));
        i++;
    }
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
                if(m_pLibrary != NULL)
                {
                    pTmpLib->getPlaylists()->readPlaylists(m_pLibrary->getPlaylists()->fileName());
                    delete m_pLibrary;
                }
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
    if(m_pLibrary == NULL)
        m_pLibrary = new CMediaLibrary();
    m_pLibrary->readXmlData(fileName);
    setFilters();
    updateViewPlaylists();
}

void MainWindow::writeMediaData()
{
    if(m_pLibrary->count() <= 0)
    {
        QMessageBox::information(this, tr("Write data"), tr("No media files loaded, nothing to write"));
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(this, tr("Save media data"),
                                    QDir::homePath(), tr("XML files (*.xml)"));
    if(!fileOut.isEmpty())
    {
        fileOut = CTool::makeSuffix(fileOut, "xml");
        writeMediaData(fileOut);
    }
}

void MainWindow::writePalylist()
{
    CPlaylists *lists = m_pLibrary->getPlaylists();

    if(!lists->fileName().isEmpty())
        lists->writePlaylists(lists->fileName());
}

void MainWindow::writeMediaData(QString fileName)
{
    if(m_pLibrary != NULL)
        m_pLibrary->writeXmlData(fileName);
}

CMediaFile *MainWindow::getCurrent()
{
    QModelIndex file;

    file = m_pSortModel->mapToSource(ui->tvMediaFiles->currentIndex());
    return m_pLibrary->getMediaFile(file.row());
}

void MainWindow::playCurrent()
{
    if(ui->tvMediaFiles->currentIndex().isValid())
    {
        playFile(ui->tvMediaFiles->currentIndex());
    }
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

void MainWindow::setViewStars()
{
    if(sender() == ui->view_star_1)
        m_pSortModel->setStars(1);
    else if(sender() == ui->view_star_2)
        m_pSortModel->setStars(2);
    else if(sender() == ui->view_star_3)
        m_pSortModel->setStars(3);
    else if(sender() == ui->view_star_4)
        m_pSortModel->setStars(4);
    else if(sender() == ui->view_star_5)
        m_pSortModel->setStars(5);
}

void MainWindow::setViewLibrary()
{
    m_pSortModel->setPlaylist(NULL);
}

void MainWindow::setViewPlaylist()
{
    QAction *pSender = dynamic_cast<QAction *>(sender());

    if(pSender != NULL)
    {
        CPlaylist *list = m_pLibrary->getPlaylists()->getPlaylist(pSender->text());
        m_pSortModel->setPlaylist(list);
    }
}

void MainWindow::addToPlaylist()
{
    CMediaFile *pFile = getCurrent();
    CPlaylist *pList = m_pSortModel->playlist();

    if(pFile != NULL)
    {
        if(pList != NULL)
        {
            QMessageBox::information(NULL, "Add to playlist", QString("Playlist [%1] is selected.\n\nAdd to playlist failed").arg(pList->name()));
        }
        else
        {
            PlaylistSelect *dlg = new PlaylistSelect(this, m_pLibrary->getPlaylists(), false);

            if(dlg->exec() == QDialog::Accepted)
            {
                pList = m_pLibrary->getPlaylists()->getPlaylist(dlg->playlist());
                pList->addArtistTitle(pFile->getArtistTitle());
            }

            delete dlg;
        }
    }
}

void MainWindow::setPlaylist()
{
    PlaylistSelect *dlg = new PlaylistSelect(this, m_pLibrary->getPlaylists(), true);

    if(dlg->exec() == QDialog::Accepted)
    {
        if(dlg->playlist() == PlaylistSelect::MEDIA_LIBRARY)
            m_pSortModel->setPlaylist(NULL);
        else
            m_pSortModel->setPlaylist(m_pLibrary->getPlaylists()->getPlaylist(dlg->playlist()));
    }
}

void MainWindow::renameCurrent()
{
    CMediaFile *pFile = getCurrent();
    QString fileName;
    bool ok=false;

    if(pFile != NULL)
    {
        fileName = pFile->getArtistTitle();
        fileName = QInputDialog::getText(this, tr("New name"), tr(""), QLineEdit::Normal, fileName, &ok);
        if(ok)
        {
            pFile->setArtistTitle(fileName);
            pFile->renameFile();
            m_pFiles->resetModel();
        }
    }

}

void MainWindow::editSettings()
{
    SettingsEdit *dlg = new SettingsEdit("Duo Liukko", "Karaoke");

    dlg->exec();
    delete dlg;
}

void MainWindow::editMediaLibraries()
{
    CMediaLibraryEdit *dlg = new CMediaLibraryEdit();

    dlg->exec();
    delete dlg;
}

void MainWindow::editPlaylists()
{
    PlaylistEdit *dlg;

    if(m_pLibrary->getPlaylists() != NULL)
    {
        dlg = new PlaylistEdit(0, m_pLibrary->getPlaylists());
        dlg->exec();
        delete dlg;
    }
}























