#include "medialibraryedit.h"
#include "ui_medialibraryedit.h"
#include "MediaFile.h"
#include "dlaraoke.h"

#include <QFileDialog>
#include <list>

using namespace std;

#define DELETE(ptr) if(ptr!=NULL) \
                    {\
                        delete ptr;\
                        ptr = NULL;\
                    }

CMediaLibraryEdit::CMediaLibraryEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMediaLibraryEdit),
    m_pSrcLib(NULL),
    m_pDestLib(NULL)
{
    ui->setupUi(this);

    connect(ui->bSelectSrcLib,      SIGNAL(clicked()),  this,   SLOT(selectSrcLib()));
    connect(ui->bSelectDestLib,     SIGNAL(clicked()),  this,   SLOT(selectDestLib()));
    connect(ui->bCopyNew,           SIGNAL(clicked()),  this,   SLOT(copyNewFiles()));
    connect(ui->bRenameDestFiles,   SIGNAL(clicked()),  this,   SLOT(renameDestFiles()));
    updateCommandButtons();
}

CMediaLibraryEdit::~CMediaLibraryEdit()
{
    DELETE(m_pSrcLib);
    DELETE(m_pDestLib);
    delete ui;
}

void CMediaLibraryEdit::updateCommandButtons()
{
    bool bEnabled;

    if(m_pSrcLib != NULL && m_pDestLib != NULL)
        bEnabled = true;
    else
        bEnabled = false;

    ui->bCopyNew->setEnabled(bEnabled);
    ui->bRenameDestFiles->setEnabled(bEnabled);
}

CMediaLibrary *CMediaLibraryEdit::selectMediaLibrary(QString szcaption)
{
    QString fileName = QFileDialog::getOpenFileName(this, szcaption, QDir::homePath(), tr("XML files (*.xml)"));
    CMediaLibrary *pLib;

    if(fileName.isEmpty()) return NULL;

    pLib = new CMediaLibrary();

    if(pLib->readXmlData(fileName))
    {
        return pLib;
    }
    else
    {
        delete pLib;
        return NULL;
    }
}

void CMediaLibraryEdit::selectSrcLib()
{
    DELETE(m_pSrcLib);
    m_pSrcLib = selectMediaLibrary(tr("Select source media library"));
    if(m_pSrcLib != NULL)
    {
        ui->eSrcLibName->setText(m_pSrcLib->getLocation());
    }
    updateCommandButtons();
}

void CMediaLibraryEdit::selectDestLib()
{
    DELETE(m_pDestLib);
    m_pDestLib = selectMediaLibrary(tr("Select destination media library"));
    if(m_pDestLib != NULL)
    {
        ui->eDestLibName->setText(m_pDestLib->getLocation());
    }
    updateCommandButtons();
}

void CMediaLibraryEdit::copyNewFiles()
{
    list<CMediaFile *> filesToCopy;
    list<CMediaFile *>::iterator itF;
    CMediaFile *pFile;
    int iFileNo;

    for(iFileNo = 0; iFileNo < m_pSrcLib->count(); iFileNo++)
    {
        pFile = m_pSrcLib->getMediaFile(iFileNo);
        if(m_pDestLib->isInLibrary(pFile) == false)
        {
            filesToCopy.push_back(pFile);
        }
    }

    QProgressDialog prgDlg("Copying media files...", "Cancel", 0, filesToCopy.size(), this);
    prgDlg.setModal(true);
    prgDlg.setValue(0);
    iFileNo=0;

    for(itF = filesToCopy.begin(); itF != filesToCopy.end(); itF++)
    {
        iFileNo++;
        prgDlg.setValue(iFileNo);
        prgDlg.setLabelText(pFile->getArtistTitle());
        DLaraoke::processEvents();
        m_pDestLib->copyTo(*itF);
        DLaraoke::processEvents();
        if(prgDlg.wasCanceled())
            return;
    }
}

void CMediaLibraryEdit::renameDestFiles()
{}

































