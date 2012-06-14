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

    m_Mapper->addMapping(ui->leName, CMediaLibraryModel::MLM_Name);
    m_Mapper->addMapping(ui->leBackColor, CMediaLibraryModel::MLM_BackColor);
    m_Mapper->addMapping(ui->leForeColor, CMediaLibraryModel::MLM_ForeColor);
    m_Mapper->addMapping(ui->leLocation, CMediaLibraryModel::MLM_FilePaht);
    m_Mapper->toFirst();

    libs->setStringList(libraries->getLibraryNames());
    ui->lstLibraries->setModel(libs);
    connect(ui->lstLibraries->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(changeLibrary(QModelIndex,QModelIndex)));

    connect(ui->bExportData, SIGNAL(clicked()), this, SLOT(exportLibraryData()));
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

    if(!lib.isNull())
    {
        QString fileOut = QFileDialog::getSaveFileName(this, tr("Export media lib"),
                                    QDir::homePath(), tr("CSV files (*.csv)"));

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
            int i;

            for(i=0; i<lib->count(); i++)
            {
                mediaFile = lib->getMediaFile(i);
                exportData << mediaFile->getArtist() << ";" << mediaFile->getTitle() << "\n";
            }
        }
    }
}
