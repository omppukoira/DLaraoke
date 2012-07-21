#include <QStringListModel>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

#include "medialibraryedit.h"
#include "ui_medialibraryedit.h"

MediaLIbraryEdit::MediaLIbraryEdit(QSharedPointer<CMediaFileLibraries> libraries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MediaLIbraryEdit),
    m_Model(new CMediaLibraryModel)
{
    QStringListModel *libs = new QStringListModel(this);

    m_Model = new CMediaLibraryModel(this);
    m_Model->setMediaLibraries(libraries);
    ui->setupUi(this);

    m_Mapper = new QDataWidgetMapper(this);
    m_Mapper->setModel(m_Model);

    m_Mapper->addMapping(ui->leName,        CMediaLibraryModel::MLM_Name);
    m_Mapper->addMapping(ui->leBackColor,   CMediaLibraryModel::MLM_BackColor);
    m_Mapper->addMapping(ui->leForeColor,   CMediaLibraryModel::MLM_ForeColor);
    m_Mapper->addMapping(ui->leLocation,    CMediaLibraryModel::MLM_FilePaht);
    m_Mapper->addMapping(ui->isMainLib,     CMediaLibraryModel::MLM_MainLib);
    m_Mapper->toFirst();

    libs->setStringList(libraries->getLibraryNames());
    ui->lstLibraries->setModel(libs);
    connect(ui->lstLibraries->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(changeLibrary(QModelIndex,QModelIndex)));

    connect(ui->bExportData, SIGNAL(clicked()), this, SLOT(exportLibraryData()));
    connect(ui->bRemoveDuplicates, SIGNAL(clicked()), this, SLOT(removeDuplicates()));
    connect(ui->bRelocateMediaFiles, SIGNAL(clicked()), this, SLOT(relocateMediaFiles()));
}

MediaLIbraryEdit::~MediaLIbraryEdit()
{
    delete ui;
}

QSharedPointer<CMediaFileLibrary> MediaLIbraryEdit::getCurrentMediaLibrary()
{
    QSharedPointer<CMediaFileLibraries> libs = m_Model->getMediaLibraries();

    if(!libs.isNull())
    {
        QSharedPointer<CMediaFileLibrary> lib = libs->getMediaLibrary(m_Mapper->currentIndex());
        return lib;
    }
    else
        return QSharedPointer<CMediaFileLibrary>();
}

void MediaLIbraryEdit::changeLibrary(const QModelIndex &current, const QModelIndex & /*previous*/)
{

    m_Mapper->setCurrentIndex(current.row());

    QSharedPointer<CMediaFileLibrary> lib = getCurrentMediaLibrary();
    if(!lib.isNull())
        ui->lblCountInformation->setText(QString("Files: %1 Unique: %2").arg(lib->fileCount()).arg(lib->uniqueFileCount()));
}

void MediaLIbraryEdit::exportLibraryData()
{
    QSharedPointer<CMediaFileLibrary> lib = getCurrentMediaLibrary();
    bool bTmp = lib->showUniqueFiles();

    if(!lib.isNull())
    {
        QString fileOut = QFileDialog::getSaveFileName(this, tr("Export media lib"),
                                    QDir::homePath(), tr("CSV files (*.csv)"));

        lib->setShowUniqueFiles(true);  // We write unique files in CSV

        if(!fileOut.isEmpty())
        {
            QFile exportFile(fileOut);

            if(!exportFile.open(QIODevice::WriteOnly))
            {
                QMessageBox::warning(this, tr("Edit medialibrary"),
                                     QString("Cant not write file %1:\n%2")
                                     .arg(fileOut)
                                     .arg(exportFile.errorString()));
                return;
            }

            // QDataStream exportData(&exportFile);
            QTextStream exportData(&exportFile);
            QSharedPointer<CMediaFile> mediaFile;
            unsigned int i;

            for(i=0; i<lib->count(); i++)
            {
                mediaFile = lib->getMediaFile(i);
                exportData << mediaFile->getArtist() << ";" << mediaFile->getTitle() << "\n";
            }
        }
        lib->setShowUniqueFiles(bTmp);
    }
}

void MediaLIbraryEdit::removeDuplicates()
{
    QSharedPointer<CMediaFileLibrary> lib = getCurrentMediaLibrary();

    if(!lib.isNull())
    {
        bool bTmp = lib->showUniqueFiles();
        QString fileOut = QFileDialog::getSaveFileName(this, tr("Remove duplicate files"),
                                              QDir::homePath(), tr("sh-script file (*.sh)"));

        if(!fileOut.isEmpty())
        {
            QFile exportFile(fileOut);

            if(!exportFile.open(QIODevice::WriteOnly))
            {
                QMessageBox::warning(this, tr("Edit medialibrary"),
                                     QString("Cant not write file %1:\n%2")
                                     .arg(fileOut)
                                     .arg(exportFile.errorString()));
                return;
            }

            // QDataStream exportData(&exportFile);
            QTextStream exportData(&exportFile);
            QSharedPointer<CMediaFile> mediaFile;
            unsigned int i;

            exportData << "#!/bin/sh\n";
            exportData << "#\n";
            exportData << "# Removing duplicates from medialibrary:\t"<< lib->getName() <<"\n";
            exportData << "# Library path: \t\t\t\t\t\t\t"<< lib->getLocation() << "\n";
            exportData << "# Files in libraray: \t\t\t" << lib->fileCount() << "/" << lib->uniqueFileCount() << "\n";
            exportData << "#\n";

            lib->setShowUniqueFiles(false);
            for(i=0; i<lib->count(); i++)
            {
                mediaFile = lib->getMediaFile(i);
                if(!mediaFile->isGlobalUnique())
                {
                    exportData << "rm \"" << mediaFile->getPaht() << QDir::separator() << mediaFile->getFileName() <<"\"\n";
                    if(mediaFile->getType() == CMediaFile::MF_CDG)
                    {
                        QString szTmpName;
                        int idx;

                        szTmpName = mediaFile->getFileName();
                        idx = szTmpName.lastIndexOf(".");
                        if(idx>0)
                        {
                            exportData << "rm \"" << mediaFile->getPaht() << QDir::separator() << szTmpName.left(idx) <<".cdg\"\n";
                        }
                    }
                }
            }

            QString fileName;

            for(i=0; i<lib->count(); i++)
            {
                mediaFile = lib->getMediaFile(i);
                fileName = mediaFile->getFileName();

                if(fileName.mid(fileName.lastIndexOf(".")).toLower() != fileName.mid(fileName.lastIndexOf(".")))
                {
                    exportData << "mv \"" << mediaFile->getPaht() << QDir::separator() << fileName << "\" \"" << mediaFile->getPaht()
                               << QDir::separator() << fileName.left(fileName.lastIndexOf(".")) << fileName.mid(fileName.lastIndexOf(".")).toLower() <<"\"\n";
                }

                if(mediaFile->getType() == CMediaFile::MF_CDG)
                {
                    fileName = fileName.left(fileName.lastIndexOf(".")) + ".CDG";
                    if(QFile::exists(mediaFile->getPaht() + QDir::separator() + fileName) == true)
                    {
                        exportData << "mv \"" << mediaFile->getPaht() << QDir::separator() << fileName << "\" \"" << mediaFile->getPaht()
                                   << QDir::separator() << fileName.left(fileName.lastIndexOf(".")) << fileName.mid(fileName.lastIndexOf(".")).toLower() << "\"\n";
                    }
                }

            }
        }
        lib->setShowUniqueFiles(bTmp);
    }
}

void MediaLIbraryEdit::relocateMediaFiles()
{
    QSharedPointer<CMediaFileLibrary> lib = getCurrentMediaLibrary();
    QSharedPointer<CMediaFile> mediaFile;
    int i;

    if(!lib.isNull())
    {
        bool bTmp = lib->showUniqueFiles();
        lib->setShowUniqueFiles(false);

        QProgressDialog progress("Relocating mediafiles...", "Cancel", 0, lib->count()-1, this);

        for(i=0; i<lib->count(); i++)
        {
            progress.setValue(i);
            mediaFile = lib->getMediaFile(i);
            mediaFile->moveToPreferredLocation(lib);
        }
        lib->setShowUniqueFiles(bTmp);
    }
}



























