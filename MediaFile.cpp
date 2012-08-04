#include "MediaFile.h"

#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QColor>
#include <QSettings>
#include <QCryptographicHash>
#include <QMessageBox>

#include "ctool.h"
#include "dlaraoke.h"

#define IS_START_ELEM(r, n) ((*r).isStartElement()) && ((*r).name() == n)

const QString CMediaFile::NOD_MEDIA_FILE            = "media_file";
const QString CMediaFile::ATTR_FILE_TYPE            = "type";
const QString CMediaFile::NOD_ARTIST                = "artist";
const QString CMediaFile::NOD_TITLE                 = "title";
const QString CMediaFile::NOD_PATH                  = "path";
const QString CMediaFile::NOD_FILE                  = "file";
const QString CMediaFile::NOD_AUDIO_TRACKS          = "audio_tracks";
const QString CMediaFile::ATTR_ID                   = "id";
const QString CMediaFile::NOD_AUDIO_TRACK           = "audio_track";
const QString CMediaFile::NOD_NAME                  = "name";
const QString CMediaFile::NOD_HASH                  = "hash";
const QString CMediaFile::NOD_SIZE                  = "file_size";
const QString CMediaFile::NOD_CREATED               = "created";
const QString CMediaFile::NOD_MODIFIED              = "modified";
const QString CMediaFile::NOD_TAGS                  = "tags";
const QString CMediaFile::NOD_TAG                   = "tag";
const QString CMediaFile::ATTR_NAME                 = "name";

const QString CMediaFile::TAG_DUPLICATE             = "duplicate";
const QString CMediaFile::TAG_LAST_FOLDER           = "last_folder";

const QString CMediaFileLibrary::m_szMediaFileTypes = "*.VOB;*.AVI;*.MP3;*.MPG;*.FLV;*.MP4";

const QString CMediaFileLibrary::NOD_MEDIA_LIBRARY  = "media_library";
const QString CMediaFileLibrary::ATTR_TYPE          = "type";
const QString CMediaFileLibrary::NOD_LOCATION       = "location";
const QString CMediaFileLibrary::NOD_MEDIA_FILES    = "media_files";
const QString CMediaFileLibrary::NOD_FORE_COLOR     = "forecolor";
const QString CMediaFileLibrary::NOD_BACK_COLOR     = "backcolor";


CMediaLibraryModel::CMediaLibraryModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

CMediaLibraryModel::~CMediaLibraryModel()
{}

QSharedPointer<CMediaFileLibraries> CMediaLibraryModel::getMediaLibraries()
{
    return m_Libraries;
}

void CMediaLibraryModel::setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs)
{
    m_Libraries = libs;
    resetModel();
}

void CMediaLibraryModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

int CMediaLibraryModel::rowCount(const QModelIndex &parent) const
{
    int iRowCount;

    if(!m_Libraries.isNull())
    {
        iRowCount = m_Libraries->count();
    }
    else
    {
        iRowCount = 0;
    }

    return iRowCount;
}

int CMediaLibraryModel::columnCount(const QModelIndex & /* parent */) const
{
    return MLM_Columns;
}

QVariant CMediaLibraryModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case MLM_Name:
            return QVariant(m_Libraries->getMediaLibrary(index.row())->getName());
        case MLM_BackColor:
            return QVariant(m_Libraries->getMediaLibrary(index.row())->getBackColor());
        case MLM_ForeColor:
            return QVariant(m_Libraries->getMediaLibrary(index.row())->getForeColor());
        case MLM_FilePaht:
            return QVariant(m_Libraries->getMediaLibrary(index.row())->getLocation());
        case MLM_MainLib:
            return QVariant(m_Libraries->getMediaLibrary(index.row())->isMain());
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool CMediaLibraryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid()) return false;

    if(role == Qt::EditRole)
    {
        switch(index.column())
        {
        case MLM_Name:
            m_Libraries->getMediaLibrary(index.row())->setName(value.toString());
            return true;
        case MLM_BackColor:
            m_Libraries->getMediaLibrary(index.row())->setBackColr(value.toString());
            return true;
        case MLM_ForeColor:
            m_Libraries->getMediaLibrary(index.row())->setForeColor(value.toString());
            return true;
        case MLM_MainLib:
            m_Libraries->getMediaLibrary(index.row())->setIsMain(value.toBool());
            return true;
        }
    }
    return false;
}

// Purpose: Constructs media library object view model
//
CMediaFileModel::CMediaFileModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

CMediaFileModel::~CMediaFileModel()
{}

void CMediaFileModel::setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs)
{
    m_Libraries = libs;
}

void CMediaFileModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

int CMediaFileModel::rowCount(const QModelIndex & /* parent*/ ) const
{
    return m_Libraries->fileCount();
}

int CMediaFileModel::columnCount(const QModelIndex & /* parent */ ) const
{
    return MFM_Type + 1;
}

QVariant CMediaFileModel::data(const QModelIndex &index, int role) const
{
    QSharedPointer<CMediaFileLibrary> lib;
    QSharedPointer<CMediaFile> file;

    if(!index.isValid()) return QVariant();

    if(role == Qt::TextAlignmentRole)
    {
        return (int)(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case MFM_Artist:
            return QVariant(m_Libraries->getMediaFile(index.row())->getArtist());
        case MFM_Title:
            return QVariant(m_Libraries->getMediaFile(index.row())->getTitle());
        case MFM_Type:
            switch(m_Libraries->getMediaFile(index.row())->getType())
            {
            case CMediaFile::MF_VOB:
                return QVariant("VOB");
            case CMediaFile::MF_AVI:
                return QVariant("AVI");
             case CMediaFile::MF_CDG:
                return QVariant("CDG");
             case CMediaFile::MF_Unknown:
                return QVariant("Unknown");
            }
        case 3:
            return QVariant();
        }
    }
    else if(role == Qt::TextColorRole)
    {
        QString szColor;

        file = m_Libraries->getMediaFile(index.row());
        if(!file->isGlobalUnique())
        {
            szColor = DLaraoke::settings().value("Color/Global Unique", "#FF0000").toString();
        }
        else if(!file->isUnique())
        {
            szColor = DLaraoke::settings().value("Color/Library Unique", "#AA0000").toString();
        }

        if(!szColor.isEmpty())
            return QColor(szColor);

        lib = file->getParent();
        if(!lib.isNull() && !lib->getForeColor().isEmpty())
            return QColor(lib->getForeColor());
    }
    else if(role == Qt::BackgroundColorRole)
    {
        lib = m_Libraries->getMediaFile(index.row())->getParent();
        if(!lib.isNull() && !lib->getBackColor().isEmpty())
            return QColor(lib->getBackColor());
    }

    return QVariant();
}

QVariant CMediaFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole || orientation == Qt::Vertical)
        return QVariant();

    switch(section)
    {
    case MFM_Artist:
        return QVariant(tr("Artist"));
    case MFM_Title:
        return QVariant(tr("Title"));
    case MFM_Type:
        return QVariant(tr("Type"));
    default:
        return QVariant();
    }
}

CDetailNode::CDetailNode(Type type, QSharedPointer<CMediaFile> mediaFile)
    : m_Type(type)
    , m_pParent(NULL)
    , m_MediaFile(mediaFile)
{ }

CDetailNode::~CDetailNode()
{
    int i;

    for(i=0; i<m_Children.size(); i++)
    {
        delete m_Children[i];
    }
    m_Children.clear();
}

CDetailNode *CDetailNode::parent() const
{
    return m_pParent;
}

void CDetailNode::setParent(CDetailNode *pParent)
{
    m_pParent = pParent;
}

CDetailNode::Type CDetailNode::type() const
{
    return m_Type;
}

QSharedPointer<CMediaFile> CDetailNode::mediaFile() const
{
    return m_MediaFile;
}

CDetailNode *CDetailNode::child(int index) const
{
    if(index < 0 || index >= m_Children.size())
        return NULL;
    else
        return m_Children[index];
}

int CDetailNode::childCount() const
{
    return m_Children.size();
}

int CDetailNode::rowNo() const
{
    return m_iRowNo;
}

void CDetailNode::setRowNo(int iRowNo)
{
    m_iRowNo = iRowNo;
}

void CDetailNode::addChild(CDetailNode *pChild)
{
    m_Children.push_back(pChild);
    pChild->setRowNo(m_Children.size()-1);
    pChild->setParent(this);
}

CDetailMediaFileModel::CDetailMediaFileModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_pRoot(NULL)
{}

CDetailMediaFileModel::~CDetailMediaFileModel()
{
    delete m_pRoot;
    m_pRoot = NULL;
}

CDetailNode *CDetailMediaFileModel::findArtist(QString szName) const
{
    int i;

    if(m_pRoot == NULL)
        return 0;

    for(i=0; i<m_pRoot->childCount(); i++)
    {
        if(!(m_pRoot->child(i)->mediaFile().isNull()))
        {
            if(m_pRoot->child(i)->mediaFile()->getArtist() == szName)
                return m_pRoot->child(i);
        }
    }

    return NULL;
}

CDetailNode *CDetailMediaFileModel::nodeFromIndex(const QModelIndex &index) const
{
    if(index.isValid())
        return static_cast<CDetailNode *>(index.internalPointer());
    else
        return m_pRoot;
}

void CDetailMediaFileModel::setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs)
{
    unsigned int i, j;
    CDetailNode *pArtist, *pMediaFile, *pAudioTracks, *pAudioTrack;
    QSharedPointer<CMediaFile> mediaFile;
    vector<QSharedPointer<CAudioTrack> > *audioTracks;

    beginResetModel();
    if(m_pRoot) delete m_pRoot;
    m_pRoot = new CDetailNode(CDetailNode::Root, QSharedPointer<CMediaFile>());
    m_pRoot->setRowNo(0);

    for(i=0; i<libs->fileCount(); i++)
    {
        mediaFile = libs->getMediaFile(i);
        if(!mediaFile.isNull())
        {
            pArtist = findArtist(mediaFile->getArtist());
            if(pArtist == NULL)
            {
                pArtist = new CDetailNode(CDetailNode::Artist, mediaFile);
                m_pRoot->addChild(pArtist);
            }

            pMediaFile = new CDetailNode(CDetailNode::MediaFile, mediaFile);
            pArtist->addChild(pMediaFile);
            if(mediaFile->audioTracks()->size() > 0)
            {
                pAudioTracks = new CDetailNode(CDetailNode::AudioTracks, mediaFile);
                pMediaFile->addChild(pAudioTracks);

                audioTracks = mediaFile->audioTracks();
                for(j=0; j<audioTracks->size(); j++)
                {
                    pAudioTrack = new CDetailNode(CDetailNode::AudioTrack, mediaFile);
                    pAudioTracks->addChild(pAudioTrack);
                }
            }
        }
    }
    endResetModel();
}

QModelIndex CDetailMediaFileModel::index(int row, int column, const QModelIndex &parent) const
{
    if(m_pRoot == NULL || row < 0 || column < 0)
        return QModelIndex();

    CDetailNode *pParentNode = nodeFromIndex(parent);
    CDetailNode *pChildNode = pParentNode->child(row);

    if(!pChildNode)
        return QModelIndex();

    return createIndex(row, column, pChildNode);
}

QModelIndex CDetailMediaFileModel::parent(const QModelIndex &child) const
{
    CDetailNode *pChild, *pParent;
    int i;

    if((pChild = nodeFromIndex(child)) == NULL)
        return QModelIndex();
    if((pParent = pChild->parent()) == NULL)
        return QModelIndex();

    return createIndex(pParent->rowNo(), 0, pParent);
}

int CDetailMediaFileModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column()>0)
        return 0;

    CDetailNode *pParent = nodeFromIndex(parent);

    if(pParent)
        return pParent->childCount();
    else
        return 0;
}

int CDetailMediaFileModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant CDetailMediaFileModel::data(const QModelIndex &index, int role) const
{
    CDetailNode *pNode;

    if(role != Qt::DisplayRole)
        return QVariant();

    if((pNode = nodeFromIndex(index)) == NULL || pNode->mediaFile().isNull())
        return QVariant();


    if(pNode->type() == CDetailNode::Artist)
    {
        if(index.column() == 0)
            return pNode->mediaFile()->getArtist();
    }
    else if(pNode->type() == CDetailNode::MediaFile)
    {
        if(index.column() == 0)
            return pNode->mediaFile()->getTitle();
        else if(index.column() == 1)
            return pNode->mediaFile()->getFileName();
    }
    else if(pNode->type() == CDetailNode::AudioTracks)
    {
        if(index.column() == 0)
            return tr("Audio Tracks");
    }
    else if(pNode->type() == CDetailNode::AudioTrack)
    {
        if(index.column() == 0)
        {
            vector<QSharedPointer<CAudioTrack> > *audioTracks;

            audioTracks = pNode->mediaFile()->audioTracks();
            if(pNode->rowNo() >= 0 && pNode->rowNo() < audioTracks->size())
                return QString("ID %1").arg(audioTracks->at(pNode->rowNo())->getId());
        }
        else if(index.column() == 1)
            return pNode->mediaFile()->audioTracks()->at(index.row())->getName();
    }

    return QVariant();
}

QVariant CDetailMediaFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(section == 0)
            return tr("Track");
        else if(section == 1)
            return tr("Path");
        else if(section == 2)
            return tr("File name");
    }
    return QVariant();
}

bool CMediaFileLibraries::isUniqueFile(QSharedPointer<CMediaFile> file)
{
    unsigned int i;

    for(i=0; i<m_MediaFiles.size(); i++)
    {
        if(m_MediaFiles[i]->getHash() == file->getHash())
            return false;
    }
    return true;
}

void CMediaFileLibraries::collectMediaFiles()
{
    m_MediaFiles.clear();
    m_UniqueMediaFiles.clear();
    unsigned int i, j;

    for(i=0; i<m_Libs.size(); i++)
    {
        for(j=0; j<m_Libs[i]->count(); j++)
        {
            QSharedPointer<CMediaFile> mediaFile;

            mediaFile = m_Libs[i]->getMediaFile(j);
            if(mediaFile->isUnique())
            {
                if(isUniqueFile(mediaFile))                 // Is this media file globally unique ?
                {
                    mediaFile->setIsGlobalUnique(true);
                    m_UniqueMediaFiles.push_back(mediaFile);
                }
                else
                    mediaFile->setIsGlobalUnique(false);
            }
            else
                mediaFile->setIsGlobalUnique(false);

            m_MediaFiles.push_back(mediaFile);
        }
    }
}

int CMediaFileLibraries::fileCount()
{
    if(m_bUniqueFiles)
        return (int)m_UniqueMediaFiles.size();
    else
        return (int)m_MediaFiles.size();
}

QStringList CMediaFileLibraries::getLibraryNames()
{
    QStringList libs;
    unsigned int i;

    for(i=0; i<m_Libs.size(); i++)
    {
        libs << m_Libs[i]->getName();
    }
    return libs;
}

QSharedPointer<CMediaFile> CMediaFileLibraries::getMediaFile(int index) const
{
    tMediaFiles *pFiles;

    if(m_bUniqueFiles)
        pFiles = (tMediaFiles *)&(m_UniqueMediaFiles);
    else
        pFiles = (tMediaFiles *)&(m_MediaFiles);

    if(index >= 0 && index < (int)(pFiles->size()))
        return pFiles->operator [](index);
    else
        return QSharedPointer<CMediaFile>();
}

// Purpose: Returns media files media library
//
QSharedPointer<CMediaFileLibrary> CMediaFileLibraries::getMediaLibrary(int index) const
{
    if(index >= 0 && index < (int)m_Libs.size())
        return m_Libs[index];
    return QSharedPointer<CMediaFileLibrary>();
}

void CMediaFileLibraries::addMediaLibrary(QSharedPointer<CMediaFileLibrary> mediaLib)
{
    bool bShowUniqueFiles;

    if(DLaraoke::settings().value("MediaFile/Show Global Unique", 1).toInt())
        bShowUniqueFiles = true;
    else
        bShowUniqueFiles = false;

    setShowUniqueFiles(bShowUniqueFiles);
    mediaLib->setShowUniqueFiles(bShowUniqueFiles);

    mediaLib->setThis(mediaLib);
    m_Libs.push_back(mediaLib);

    collectMediaFiles();
}

void CMediaFileLibraries::readMediaLibraries(QXmlStreamReader *reader)
{
    reader->readNext();

    while(!reader->atEnd())
    {
        if(IS_START_ELEM(reader, "media_libraries"))
        {
            while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == "media_libraries"))
            {
                if(IS_START_ELEM(reader, CMediaFileLibrary::NOD_MEDIA_LIBRARY))
                {
                    QSharedPointer<CMediaFileLibrary> lib = QSharedPointer<CMediaFileLibrary>(new CMediaFileLibrary());

                    if(reader->attributes().value(CMediaFileLibrary::ATTR_TYPE).toString() == "main")
                        lib->setIsMain(true);
                    else
                        lib->setIsMain(false);

                    lib->setThis(lib);
                    lib->readXmlData(reader);
                    if(lib->count() > 0)
                    {
                        addMediaLibrary(lib);
                    }
                }
                reader->readNext();
            }
        }
        reader->readNext();
    }

    if(reader->hasError())
    {
        return;
    }
}

void CMediaFileLibraries::writeMediaLibraries(QXmlStreamWriter *writer)
{
    unsigned int i;

    writer->writeStartDocument();
    writer->writeStartElement("media_libraries");
    for(i=0; i<m_Libs.size(); i++)
    {
        m_Libs[i]->writeXmlData(writer);
    }
    writer->writeEndElement();
    writer->writeEndDocument();
}

QString CMediaFileLibrary::setColor(QString szColor)
{
    QColor color(szColor);
    if(szColor.isEmpty())
        return "";
    return color.name();
}

// Purpose: Add media file into library
//
//  m_MediaFiles        contains all media files in this library
//  m_UniqueMediaFiles  contains all unique (by hash) files in this library
//
void CMediaFileLibrary::addFile(QSharedPointer<CMediaFile> file)
{
    bool isUnique = true;
    unsigned int i;
    QString szLastFolder;

    file->setParent(m_This);

    for(i=0; i<m_MediaFiles.size(); i++)
        if(m_MediaFiles[i]->getHash() == file->getHash())
        {
            m_MediaFiles[i]->addTag(CMediaFile::TAG_DUPLICATE, file->getFullName());

            szLastFolder = file->getPahtInLibrary();
            szLastFolder = szLastFolder.mid(szLastFolder.lastIndexOf(QDir::separator())+1);
            if(!szLastFolder.isEmpty())
            {
                m_MediaFiles[i]->addTag(CMediaFile::TAG_LAST_FOLDER, szLastFolder);
            }
            isUnique = false;
            break;
        }

    m_MediaFiles.push_back(file);
    if(isUnique)
    {
        szLastFolder = file->getPahtInLibrary();
        szLastFolder = szLastFolder.mid(szLastFolder.lastIndexOf(QDir::separator())+1);
        if(!szLastFolder.isEmpty())
        {
            file->addTag(CMediaFile::TAG_LAST_FOLDER, szLastFolder);
        }
        file->setIsUnique(true);
        m_UniqueMediaFiles.push_back(file);
    }
    else
    {
        file->setIsUnique(false);
    }
}

// Purpose: Add mediafiles from given directory and subdirectories into media lib
//
// Parameters:  szDirectory     Name of directory being added
//
void CMediaFileLibrary::addFiles(QString szDirectory, bool bPrescan, QProgressDialog *pPrgDlg)
{
    QDir mlib(szDirectory);
    QStringList szFilters;

    szFilters = m_szMediaFileTypes.split(";", QString::SkipEmptyParts);

    if(pPrgDlg != NULL)
    {
        if(pPrgDlg->wasCanceled())
            return;
        pPrgDlg->setLabelText(szDirectory);
    }

    foreach (QString file, mlib.entryList(szFilters, QDir::Files))
    {
        QSharedPointer<CMediaFile> pMediaFile;
        QFileInfo fi(mlib.absolutePath() + QDir::separator() + file);

        if(!fi.exists())
        {
            QMessageBox::warning(NULL, "addFiles", QString("File [%1] does not exist").arg(mlib.dirName() + QDir::separator() + file));
        }
        if(!fi.isDir())
        {
            pMediaFile = QSharedPointer<CMediaFile>(new CMediaFile());

            pMediaFile->Create(fi, szDirectory); // Oli QFileInfo(file) ??
            if(!bPrescan)
                pMediaFile->setHash(pMediaFile->calcHash());

            if(pMediaFile->getType() != CMediaFile::MF_Unknown)
            {
                addFile(pMediaFile);

                if(!bPrescan)
                {
                    if(pMediaFile->getType() == CMediaFile::MF_VOB)
                    {
                        if(DLaraoke::settings().value("MediaFile/Read audio tracks").toInt() != 0)
                            pMediaFile->readAudioTrackInfo();
                    }
                }

                if(pPrgDlg != NULL)
                {
                    if(pPrgDlg->wasCanceled())
                        return;
                    pPrgDlg->setValue(m_MediaFiles.size());
                }
            }
        }
    }

    foreach (QString subDir, mlib.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        addFiles(szDirectory + QDir::separator() + subDir, bPrescan, pPrgDlg);
}

void CMediaFileLibrary::setThis(QWeakPointer<CMediaFileLibrary> wpThis)
{
    m_This = wpThis;
}

void CMediaFileLibrary::scanMediaFiles(QString szMediaLib, bool bPreScan, QProgressDialog *pPrgDlg)
{
    m_szMediaLibLocation = szMediaLib;

    addFiles(szMediaLib, bPreScan, pPrgDlg);
}

void CMediaFileLibrary::readXmlData(QXmlStreamReader *reader)
{
    if(IS_START_ELEM(reader, NOD_MEDIA_LIBRARY))
    {
        m_szName = reader->attributes().value("name").toString();
        if(reader->attributes().value(CMediaFileLibrary::ATTR_TYPE).toString() == "main")
            m_bIsMainLib = true;
        else
            m_bIsMainLib = false;

        while(!(reader->atEnd()) && !(reader->isEndElement() && reader->name() == NOD_MEDIA_LIBRARY))
        {
            if(IS_START_ELEM(reader, NOD_LOCATION))
            {
                m_szMediaLibLocation = reader->readElementText();
            }
            else if(IS_START_ELEM(reader, NOD_FORE_COLOR))
            {
                m_szForeColor = reader->readElementText();
            }
            else if(IS_START_ELEM(reader, NOD_BACK_COLOR))
            {
                m_szBackColor = reader->readElementText();
            }
            else if(IS_START_ELEM(reader, NOD_MEDIA_FILES))
            {
                while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_MEDIA_FILES))
                {
                    if(IS_START_ELEM(reader, CMediaFile::NOD_MEDIA_FILE))
                    {
                        QSharedPointer<CMediaFile> mediaFile = QSharedPointer<CMediaFile>(new CMediaFile());

                        mediaFile->readXmlData(reader);
                        if(!reader->atEnd())
                        {
                            addFile(mediaFile);
                        }
                    }
                    reader->readNext();
                    qDebug() << CTool::xmlNodeInfo(*reader);
                }
            }
            reader->readNext();
        }
    }
    else
    {
        reader->raiseError("Expecting media library node");
    }
}

void CMediaFileLibrary::writeXmlData(QXmlStreamWriter *writer)
{
    unsigned int i;

    writer->writeStartElement(NOD_MEDIA_LIBRARY);
    writer->writeAttribute("name", m_szName);
    if(m_bIsMainLib)
        writer->writeAttribute(ATTR_TYPE, "main");
    else
        writer->writeAttribute(ATTR_TYPE, "secondary");
    writer->writeTextElement(NOD_LOCATION, m_szMediaLibLocation);
    writer->writeTextElement(NOD_FORE_COLOR, m_szForeColor);
    writer->writeTextElement(NOD_BACK_COLOR, m_szBackColor);
    writer->writeStartElement(NOD_MEDIA_FILES);
    for(i=0; i<m_MediaFiles.size(); i++)
    {
        m_MediaFiles[i]->writeXmlData(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}

unsigned int CMediaFileLibrary::count()
{
    if(m_bUniqueFiles)
        return m_UniqueMediaFiles.size();
    else
        return m_MediaFiles.size();
}

int CMediaFileLibrary::fileCount()
{
    return m_MediaFiles.size();
}

int CMediaFileLibrary::uniqueFileCount()
{
    return m_UniqueMediaFiles.size();
}

QSharedPointer<CMediaFile> CMediaFileLibrary::getMediaFile(int index)
{
    tMediaFiles *pFiles;

    if(m_bUniqueFiles)
        pFiles = &(m_UniqueMediaFiles);
    else
        pFiles = &(m_MediaFiles);

    if(index >= 0 && index < (int)pFiles->size())
        return (*pFiles)[index];
    else
        return QSharedPointer<CMediaFile>();
}

CAudioTrack::CAudioTrack()
    : m_iId(0)
{}

CAudioTrack::CAudioTrack(int iId)
    : m_iId(iId)
{}

CAudioTrack::CAudioTrack(int iId, QString szName)
    : m_iId(iId), m_szName(szName)
{}

CAudioTrack &CAudioTrack::operator =(int iId)
{
    m_iId = iId;
    m_szName = QString("Audio track [%1]").arg(iId);
    return *this;
}

CFileTag::CFileTag()
    : m_szName("")
{}

CFileTag::CFileTag(QString szName, QString szValue)
    : m_szName(szName)
    , m_szValue(szValue)
{}

QString CFileTag::name() const
{
    return m_szName;
}

void CFileTag::setName(QString szName)
{
    szName = szName;
}

QString CFileTag::value() const
{
    return m_szValue;
}

void CFileTag::setValue(QString szValue)
{
    m_szValue = szValue;
}

// Purpose: Constructs media file object
//
CMediaFile::CMediaFile()
    : m_FileType(MF_Unknown)
{}

bool CMediaFile::Create(QFileInfo &mediaFile, QString szPath)
{
    m_szFilePaht = szPath;
    m_szFileName = mediaFile.fileName();

    if(mediaFile.suffix().toUpper() == "VOB")
        m_FileType = MF_VOB;
    else if(mediaFile.suffix().toUpper() == "AVI" || mediaFile.suffix().toUpper() == "MPG" || mediaFile.suffix().toUpper() == "MP4" ||
            mediaFile.suffix().toUpper() == "FLV")
        m_FileType = MF_AVI;
    else if(mediaFile.suffix().toUpper() == "MP3")
        m_FileType = MF_CDG;
    else
        m_FileType = MF_Unknown;

    int iPos = m_szFileName.lastIndexOf(" - ");
    if(iPos > 0)
    {
        m_szArtist = m_szFileName.left(iPos).simplified();
        m_szTitle = m_szFileName.mid(iPos+3).simplified();
    }
    else
        m_szTitle = m_szFileName;

    iPos = m_szTitle.lastIndexOf(".");
    if(iPos > 0)
        m_szTitle = m_szTitle.left(iPos).simplified();

    iPos = m_szTitle.lastIndexOf("_");
    if(iPos > 0)
        m_szTitle = m_szTitle.left(iPos).simplified();

    if(m_szArtist == "" && m_szTitle == "")
    {
        qDebug() << "ERROR: File"<< m_szFileName <<"Artist and Title is empty";
        m_FileType = MF_Unknown;
    }

    m_iFileSize = mediaFile.size();
    m_Created = mediaFile.created();
    m_Modified = mediaFile.lastModified();
    return true;
}

QString CMediaFile::getExecCmd() const
{
    QString cmd, player;
    QSettings settings;

    switch(m_FileType)
    {
    case MF_VOB:
        player = settings.value("Media Player/VOB").toString();
        if(player.isEmpty())    // Forcing settings to default value
        {
            settings.setValue("Media Player/VOB", "vlc -f \"%1\"");
        }
        break;
    case MF_AVI:
        player = settings.value("Media Player/AVI").toString();
        if(player.isEmpty())    // Forcing settings to default value
        {
            settings.setValue("Media Player/AVI", "vlc -f \"%1\"");
        }
        break;
    case MF_CDG:
        player = settings.value("Media Player/CDG").toString();
        if(player.isEmpty())    // Forcing settings to default value
        {
            settings.setValue("Media Player/CDG", "vlc -f \"%1\"");
        }
        break;
    default:
        return "";
    }

    cmd = player.arg(m_szFilePaht + QDir::separator() + m_szFileName);
    return cmd;
}

QString CMediaFile::getPahtInLibrary() const
{
    QSharedPointer<CMediaFileLibrary> mLib = getParent();
    if(mLib.isNull())
        return m_szFilePaht;
    QString szLibraryPath = mLib->getLocation();
    int i = 1;

    while(szLibraryPath.left(i) == m_szFilePaht.left(i) && i < szLibraryPath.length())
        i++;
    return m_szFilePaht.mid(i);
}

bool CMediaFile::moveToPreferredLocation(QSharedPointer<CMediaFileLibrary> mediaLib)
{
    QString orgFile, newFile, newPath, orgCdg, artistName, finalName, ext;
    QDir dir;
    int n;

    if(mediaLib.isNull())
        mediaLib = m_Parent;

    orgFile = getPaht() + QDir::separator() + getFileName();
    if(orgFile.lastIndexOf(".") > 0)
        ext = orgFile.mid(orgFile.lastIndexOf("."));

    artistName = getArtist();
    if(artistName != "")
    {
        orgCdg = artistName.left(1).toUpper();
        if(orgCdg >= "0" && orgCdg < "9")
            orgCdg = "0-9";

        newFile = getArtist() + " - " + getTitle(); // No file extension (yet)
    }
    else
    {
        artistName = "Unknown";
        newFile = getTitle();

    }

    newPath = mediaLib->getLocation() + QDir::separator() + orgCdg + QDir::separator() + artistName;

    finalName = newPath + QDir::separator() + newFile + ext;
    if(finalName == orgFile)
        return true;        // Meida file is in correct place no need to move anything

    dir.mkpath(newPath);

    for(n=1; n<10; n++)
    {
        finalName = newPath + QDir::separator() + newFile;
        if(n > 1)
            finalName += QString("_%1").arg(n);

        if(QFile::rename(orgFile, finalName + ext) == true)
        {
            orgFile = orgFile.left(orgFile.lastIndexOf("."));
            if(QFile::exists(orgFile + ".cdg"))
                QFile::rename(orgFile + ".cdg", finalName + ".cdg");
            else if(QFile::exists(orgFile + ".CDG"))
                QFile::rename(orgFile + ".CDG", finalName + ".cdg");

            m_szFilePaht = newPath;
            m_szFileName = newFile;
            if(n > 1)
                m_szFileName += QString("_%1").arg(n);
            m_szFileName += ext;
            return true;
        }
    }

    return false;
}

QString CMediaFile::calcHash()
{
    QFile file(m_szFilePaht + QDir::separator() + m_szFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = file.read(DLaraoke::settings().value("MediaFile/md5Size", 10240).toInt());

        QByteArray hashData = QCryptographicHash::hash(fileData,QCryptographicHash::Md5);
        return hashData.toHex();

    }
    return "";
}
QWeakPointer<CMediaFileLibrary> CMediaFile::getParent() const
{
    return m_Parent;
}

void CMediaFile::setParent(QSharedPointer<CMediaFileLibrary> mediaLib)
{
    m_Parent = mediaLib;
}

void CMediaFile::addTag(QString szName, QString szValue)
{
    if(szName.isEmpty()) return;

    QSharedPointer<CFileTag> tag = QSharedPointer<CFileTag>(new CFileTag(szName, szValue));
    m_tags.push_back(tag);
}

void CMediaFile::updateTag(QString szName, QString szValue)
{
    if(szName.isEmpty()) return;

    QSharedPointer<CFileTag> tag = QSharedPointer<CFileTag>(new CFileTag(szName, szValue));
    int i;

    for(i=0; i<m_tags.size(); i++)
    {
        if(m_tags[i]->name() == szName)
        {
            m_tags[i] = tag;
            return;
        }
    }
    m_tags.push_back(tag);
}

bool CMediaFile::readAudioTrackInfo()
{
    QProcess info;
    QString cmd, output, outputLine;
    QTextStream builder(&cmd), reader;
    QStringList args;

    builder << "mplayer -identify -frames 0 \""<< m_szFilePaht << QDir::separator() << m_szFileName << "\" 2> /dev/null | grep ID_AUDIO_ID";
    args << "-c" << cmd;
    info.start("/bin/sh", args);
    info.waitForFinished(5000);                 // Hard coded, waiting for process to get audio info
    output = info.readAllStandardOutput();
    reader.setString(&output);

    outputLine = reader.readLine();
    while(outputLine != "")
    {
        m_audioTracks.push_back(QSharedPointer<CAudioTrack>(new CAudioTrack(outputLine.split("=").at(1).toInt())));
        outputLine = reader.readLine();
    }
    return true;
}

// Purpose: Read media file data from xml
//
void CMediaFile::readXmlData(QXmlStreamReader *reader)
{
    if(reader->isStartElement() && reader->name() == NOD_MEDIA_FILE)
    {
        QString szType;

        szType = reader->attributes().value(ATTR_FILE_TYPE).toString();
        if(szType == "VOB")
            m_FileType = MF_VOB;
        else if(szType == "AVI")
            m_FileType = MF_AVI;
        else if(szType == "CDG")
            m_FileType = MF_CDG;

        reader->readNext();
        while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_MEDIA_FILE))
        {
            if(IS_START_ELEM(reader, NOD_ARTIST))
                m_szArtist = reader->readElementText();
            else if(IS_START_ELEM(reader, NOD_TITLE))
                m_szTitle = reader->readElementText();
            else if(IS_START_ELEM(reader, NOD_PATH))
                m_szFilePaht = reader->readElementText();
            else if(IS_START_ELEM(reader, NOD_FILE))
                m_szFileName = reader->readElementText();
            else if(IS_START_ELEM(reader, NOD_AUDIO_TRACKS))
            {
                while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_AUDIO_TRACKS))
                {
                    if(IS_START_ELEM(reader, NOD_AUDIO_TRACK))
                    {
                        int id;
                        QString szATrackInfo = reader->attributes().value(ATTR_ID).toString(),
                                szTrackName = reader->readElementText();
                        QTextStream inp(&szATrackInfo);

                        inp >> id;
                        m_audioTracks.push_back(QSharedPointer<CAudioTrack>(new CAudioTrack(id, szTrackName)));
                    }
                    reader->readNext();
                }
            }
            else if(IS_START_ELEM(reader, NOD_HASH))
                m_szHash = reader->readElementText();
            else if(IS_START_ELEM(reader, NOD_SIZE))
                m_iFileSize = reader->readElementText().toInt();
            else if(IS_START_ELEM(reader, NOD_CREATED))
                m_Created = QDateTime::fromString(reader->readElementText());
            else if(IS_START_ELEM(reader, NOD_MODIFIED))
                m_Modified = QDateTime::fromString(reader->readElementText());
            else if(IS_START_ELEM(reader, NOD_TAGS))
            {
                while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_TAGS))
                {
                    if(IS_START_ELEM(reader, NOD_TAG))
                    {
                        QString name = reader->attributes().value(ATTR_NAME).toString(),
                                value = reader->readElementText();
                        addTag(name, value);
                    }
                    reader->readNext();
                }
            }

            reader->readNext();
        }
    }
    else
        reader->raiseError("Expecting media file node");
}

void CMediaFile::writeXmlData(QXmlStreamWriter *writer)
{
    unsigned int i;
    QString szTrack;

    writer->writeStartElement(NOD_MEDIA_FILE);
    switch(m_FileType)
    {
    case MF_VOB:
        szTrack = "VOB";
        break;
    case MF_AVI:
        szTrack = "AVI";
        break;
    case MF_CDG:
        szTrack = "CDG";
        break;
    default:
        szTrack = "";
        break;
    }
    if(!szTrack.isEmpty())
        writer->writeAttribute(ATTR_FILE_TYPE, szTrack);

    writer->writeTextElement(NOD_ARTIST,    m_szArtist);
    writer->writeTextElement(NOD_TITLE,     m_szTitle);
    writer->writeTextElement(NOD_PATH,      m_szFilePaht);
    writer->writeTextElement(NOD_FILE,      m_szFileName);

    if(m_FileType == MF_VOB)
    {
        writer->writeStartElement(NOD_AUDIO_TRACKS);

        for(i=0; i<m_audioTracks.size(); i++)
        {
            writer->writeStartElement(NOD_AUDIO_TRACK);
            writer->writeAttribute(ATTR_ID, QString("%1").arg(m_audioTracks[i]->getId()));
            writer->writeCharacters(m_audioTracks[i]->getName());
            writer->writeEndElement();
        }

        writer->writeEndElement();
    }
    writer->writeTextElement(NOD_HASH,      m_szHash);
    writer->writeTextElement(NOD_SIZE,      QString("%1").arg(m_iFileSize));
    writer->writeTextElement(NOD_CREATED,   m_Created.toString());
    writer->writeTextElement(NOD_MODIFIED,  m_Modified.toString());
    if(m_tags.size() > 0)
    {
        writer->writeStartElement(NOD_TAGS);
        for(i=0; i<m_tags.size(); i++)
        {
            writer->writeStartElement(NOD_TAG);
            writer->writeAttribute(ATTR_NAME, m_tags[i]->name());
            writer->writeCharacters(m_tags[i]->value());
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}





























