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

const QString CMediaFile::NOD_FILEINFO              = "FileInfo";

const QString CMediaFile::TAG_DUPLICATE             = "duplicate";
const QString CMediaFile::TAG_LAST_FOLDER           = "last_folder";

const QString CMediaLibrary::m_szMediaFileTypes     = "*.VOB;*.AVI;*.MP3;*.MPG;*.FLV;*.MP4";

const QString CMediaLibrary::NOD_MEDIA_LIBRARY      = "media_library";
const QString CMediaLibrary::ATTR_TYPE              = "type";
const QString CMediaLibrary::NOD_LOCATION           = "location";
const QString CMediaLibrary::NOD_MEDIA_FILES        = "media_files";
const QString CMediaLibrary::NOD_FORE_COLOR         = "forecolor";
const QString CMediaLibrary::NOD_BACK_COLOR         = "backcolor";


// Purpose: Constructs media library object view model
//
CMediaModel::CMediaModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_pLibrary(NULL)
{}

CMediaModel::~CMediaModel()
{}

void CMediaModel::setMediaLibrary(CMediaLibrary *pMediaLibrary)
{
    m_pLibrary = pMediaLibrary;
    resetModel();
}

void CMediaModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

int CMediaModel::rowCount(const QModelIndex & /* parent*/ ) const
{
    if(m_pLibrary == NULL)
        return 0;
    else
        return m_pLibrary->count();
}

int CMediaModel::columnCount(const QModelIndex & /* parent */ ) const
{
    return MFM_Type + 1;
}

QVariant CMediaModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    if(role == Qt::TextAlignmentRole)
    {
        return (int)(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else if(role == Qt::DisplayRole)
    {
        CMediaFile *pFile;

        pFile = m_pLibrary->getMediaFile(index.row());
        if(pFile == NULL)
        {
            return QVariant();
        }

        switch(index.column())
        {
        case MFM_Artist:
            return QVariant(pFile->getArtist());
        case MFM_Title:
            return QVariant(pFile->getTitle());
        case MFM_Type:
            switch(pFile->getType())
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

    return QVariant();
}

QVariant CMediaModel::headerData(int section, Qt::Orientation orientation, int role) const
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

CDetailNode::CDetailNode(Type type, CMediaFile *pMediaFile)
    : m_Type(type)
    , m_pParent(NULL)
    , m_pMediaFile(pMediaFile)
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

CMediaFile *CDetailNode::mediaFile() const
{
    return m_pMediaFile;
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

CDetailMediaModel::CDetailMediaModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_pRoot(NULL)
{}

CDetailMediaModel::~CDetailMediaModel()
{
    delete m_pRoot;
    m_pRoot = NULL;
}

CDetailNode *CDetailMediaModel::findArtist(QString szName) const
{
    int i;

    if(m_pRoot == NULL)
        return 0;

    for(i=0; i<m_pRoot->childCount(); i++)
    {
        if(!(m_pRoot->child(i)->mediaFile() == NULL))
        {
            if(m_pRoot->child(i)->mediaFile()->getArtist() == szName)
                return m_pRoot->child(i);
        }
    }

    return NULL;
}

CDetailNode *CDetailMediaModel::nodeFromIndex(const QModelIndex &index) const
{
    if(index.isValid())
        return static_cast<CDetailNode *>(index.internalPointer());
    else
        return m_pRoot;
}

void CDetailMediaModel::setMediaLibrary(CMediaLibrary *pLib)
{
    unsigned int i, j;
    CDetailNode *pArtist, *pFile, *pAudioTracks, *pAudioTrack;
    CMediaFile *pMediaFile;
    vector<CAudioTrack> audioTracks;

    beginResetModel();
    if(m_pRoot) delete m_pRoot;
    m_pRoot = new CDetailNode(CDetailNode::Root, NULL);
    m_pRoot->setRowNo(0);

    for(i=0; i<pLib->count(); i++)
    {
        pMediaFile = pLib->getMediaFile(i);
        if(pMediaFile != NULL)
        {
            pArtist = findArtist(pMediaFile->getArtist());
            if(pArtist == NULL)
            {
                pArtist = new CDetailNode(CDetailNode::Artist, pMediaFile);
                m_pRoot->addChild(pArtist);
            }

            pFile = new CDetailNode(CDetailNode::MediaFile, pMediaFile);
            pArtist->addChild(pFile);
            if(pMediaFile->audioTracks().size() > 0)
            {
                pAudioTracks = new CDetailNode(CDetailNode::AudioTracks, pMediaFile);
                pFile->addChild(pAudioTracks);

                audioTracks = pMediaFile->audioTracks();
                for(j=0; j<audioTracks.size(); j++)
                {
                    pAudioTrack = new CDetailNode(CDetailNode::AudioTrack, pMediaFile);
                    pAudioTracks->addChild(pAudioTrack);
                }
            }
        }
    }
    endResetModel();
}

QModelIndex CDetailMediaModel::index(int row, int column, const QModelIndex &parent) const
{
    if(m_pRoot == NULL || row < 0 || column < 0)
        return QModelIndex();

    CDetailNode *pParentNode = nodeFromIndex(parent);
    CDetailNode *pChildNode = pParentNode->child(row);

    if(!pChildNode)
        return QModelIndex();

    return createIndex(row, column, pChildNode);
}

QModelIndex CDetailMediaModel::parent(const QModelIndex &child) const
{
    CDetailNode *pChild, *pParent;
    int i;

    if((pChild = nodeFromIndex(child)) == NULL)
        return QModelIndex();
    if((pParent = pChild->parent()) == NULL)
        return QModelIndex();

    return createIndex(pParent->rowNo(), 0, pParent);
}

int CDetailMediaModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column()>0)
        return 0;

    CDetailNode *pParent = nodeFromIndex(parent);

    if(pParent)
        return pParent->childCount();
    else
        return 0;
}

int CDetailMediaModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant CDetailMediaModel::data(const QModelIndex &index, int role) const
{
    CDetailNode *pNode;

    if(role != Qt::DisplayRole)
        return QVariant();

    if((pNode = nodeFromIndex(index)) == NULL || pNode->mediaFile() == NULL)
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
            vector<CAudioTrack> audioTracks;

            audioTracks = pNode->mediaFile()->audioTracks();
            if(pNode->rowNo() >= 0 && pNode->rowNo() < audioTracks.size())
                return QString("ID %1").arg(audioTracks.at(pNode->rowNo()).getId());
        }
        else if(index.column() == 1)
            return pNode->mediaFile()->audioTracks().at(index.row()).getName();
    }

    return QVariant();
}

QVariant CDetailMediaModel::headerData(int section, Qt::Orientation orientation, int role) const
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

CMediaLibrary::CMediaLibrary()
{}

CMediaLibrary::~CMediaLibrary()
{
    clear();
}


// Purpose: Add media file into library
//
//  m_MediaFiles        contains all media files in this library
//  m_UniqueMediaFiles  contains all unique (by hash) files in this library
//
void CMediaLibrary::addFile(CMediaFile *file)
{
    file->setParent(this);
    m_MediaFiles.push_back(file);
}

// Purpose: Add mediafiles from given directory and subdirectories into media lib
//
// Parameters:  szDirectory     Name of directory being added
//              bPrescan        Do not calc MD5-sum and get audio track info
//              pPrgDlg         Show progress of file collection
//
// NOTES:
// - Use bPrescan == true to determine number of files in library
// - QProgressDialog min/max must be set before calling this method
//
void CMediaLibrary::addFiles(QString szDirectory, bool bPrescan, QProgressDialog *pPrgDlg)
{
    QDir mlib(szDirectory);
    QStringList szFilters;

    szFilters = m_szMediaFileTypes.split(";", QString::SkipEmptyParts);

    if(pPrgDlg != NULL)
    {
        if(pPrgDlg->wasCanceled())
            return;
        pPrgDlg->setLabelText(szDirectory);
        DLaraoke::processEvents();
    }

    foreach (QString file, mlib.entryList(szFilters, QDir::Files))
    {
        CMediaFile *pMediaFile;
        QFileInfo fi(mlib.absolutePath() + QDir::separator() + file);

        if(!fi.exists())
        {
            QMessageBox::warning(NULL, "addFiles", QString("File [%1] does not exist").arg(mlib.dirName() + QDir::separator() + file));
        }
        
        if(!fi.isDir())
        {
            pMediaFile = new CMediaFile();

            pMediaFile->Create(fi, szDirectory);
            if(!bPrescan)
            {
                if(pMediaFile->readInfoFile() == false || pMediaFile->getHash() == "")
                {
                    pMediaFile->setHash(pMediaFile->calcHash());
                }
            }

            if(pMediaFile->getType() != CMediaFile::MF_Unknown)
            {
                addFile(pMediaFile);

                if(!bPrescan)
                {
                    if(pMediaFile->getType() == CMediaFile::MF_VOB)
                    {
                        pMediaFile->readAudioTrackInfo();
                    }

                    if(pMediaFile->isInfoChanged())
                        pMediaFile->writeInfoFile();
                }

                if(pPrgDlg != NULL)
                {
                    if(pPrgDlg->wasCanceled())
                        return;
                    pPrgDlg->setValue(m_MediaFiles.size());
                }
            }
            else
                delete pMediaFile;
        }
    }

    foreach (QString subDir, mlib.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        addFiles(szDirectory + QDir::separator() + subDir, bPrescan, pPrgDlg);
}

void CMediaLibrary::scanMediaFiles(QString szMediaLib, bool bPreScan, QProgressDialog *pPrgDlg)
{
    m_szMediaLibLocation = szMediaLib;

    addFiles(szMediaLib, bPreScan, pPrgDlg);
}

void CMediaLibrary::readXmlData(QXmlStreamReader *reader)
{
    if(IS_START_ELEM(reader, NOD_MEDIA_LIBRARY))
    {
        m_szName = reader->attributes().value("name").toString();

        while(!(reader->atEnd()) && !(reader->isEndElement() && reader->name() == NOD_MEDIA_LIBRARY))
        {
            if(IS_START_ELEM(reader, NOD_LOCATION))
            {
                m_szMediaLibLocation = reader->readElementText();
            }
            else if(IS_START_ELEM(reader, NOD_MEDIA_FILES))
            {
                while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_MEDIA_FILES))
                {
                    if(IS_START_ELEM(reader, CMediaFile::NOD_MEDIA_FILE))
                    {
                        CMediaFile *pMediaFile = new CMediaFile();

                        pMediaFile->readXmlData(reader);
                        if(!reader->atEnd())
                        {
                            addFile(pMediaFile);
                        }
                    }
                    reader->readNext();
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

void CMediaLibrary::writeXmlData(QXmlStreamWriter *writer)
{
    unsigned int i;

    writer->writeStartElement(NOD_MEDIA_LIBRARY);
    writer->writeAttribute("name", m_szName);
    writer->writeTextElement(NOD_LOCATION, m_szMediaLibLocation);
    writer->writeStartElement(NOD_MEDIA_FILES);
    for(i=0; i<m_MediaFiles.size(); i++)
    {
        m_MediaFiles[i]->writeXmlData(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}

unsigned int CMediaLibrary::count()
{
    return m_MediaFiles.size();
}

void CMediaLibrary::clear()
{
    int i;

    for(i = 0; i < m_MediaFiles.size(); i++)
    {
        delete m_MediaFiles[i];
        m_MediaFiles[i] = NULL;
    }
    m_MediaFiles.clear();
}

CMediaFile *CMediaLibrary::getMediaFile(int index) const
{
    CMediaFile *pFile;

    if(index >= 0 && index < (int)m_MediaFiles.size())
        pFile = m_MediaFiles[index];
    else
        pFile = NULL;
    qDebug() << "Index ["<< index <<"] Pos ["<< pFile <<"]";
    return pFile;
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
    , m_pMediaLibrary(NULL)
    , m_isInfoChanged(false)
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
    CMediaLibrary *mLib = getParent();

    if(mLib == NULL)
        return m_szFilePaht;

    QString szLibraryPath = mLib->getLocation();
    int i = 1;

    while(szLibraryPath.left(i) == m_szFilePaht.left(i) && i < szLibraryPath.length())
        i++;
    return m_szFilePaht.mid(i);
}

bool CMediaFile::moveToPreferredLocation(CMediaLibrary *pMediaLib)
{
    QString orgFile, newFile, newPath, orgCdg, artistName, finalName, ext, mlLocation;
    QDir dir;
    int n;

    if(pMediaLib == NULL)
        pMediaLib = m_pMediaLibrary;

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

    newPath = pMediaLib->getLocation() + QDir::separator() + orgCdg + QDir::separator() + artistName;

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

QString CMediaFile::getInfoFile()
{
    QString infoFile = m_szFilePaht + QDir::separator() + m_szFileName;

    infoFile += ".info";
    return infoFile;
}

bool CMediaFile::readInfoFile()
{
    QFile file(getInfoFile());

    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        return false;
    }

    QXmlStreamReader reader;
    reader.setDevice(&file);

    reader.readNext();
    while(!reader.atEnd() && !(reader.isEndElement() && reader.name() == NOD_FILEINFO))
    {
        if(reader.isStartElement() && reader.name() == NOD_HASH)
            setHash(reader.readElementText());
        else if(reader.isStartElement() && reader.name() == NOD_AUDIO_TRACKS)
            readXMLAudioData(&reader);

        reader.readNext();
    }
    return true;
}

bool CMediaFile::writeInfoFile()
{
    QFile file(getInfoFile());

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        return false;
    }

    QXmlStreamWriter writer(&file);
    int i;

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(NOD_FILEINFO);
    writer.writeAttribute(ATTR_NAME, m_szFileName);
    writer.writeTextElement(NOD_HASH, getHash());
    if(m_FileType == MF_VOB && m_audioTracks.size() > 0)
    {
        writer.writeStartElement(NOD_AUDIO_TRACKS);
        for(i=0; i<m_audioTracks.size(); i++)
        {
            writer.writeStartElement(NOD_AUDIO_TRACK);
            writer.writeAttribute(ATTR_ID, QString("%1").arg(m_audioTracks[i].getId()));
            writer.writeCharacters(m_audioTracks[i].getName());
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();

    file.close();
    return true;
}

// Purpose: Calculate hash of file
//
// - Use settings "MediaFile/md5Size" to determine how many bytes are read in
//
QString CMediaFile::calcHash()
{
    QFile file(m_szFilePaht + QDir::separator() + m_szFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = file.readAll(); // file.read(DLaraoke::settings().value("MediaFile/md5Size", 10240).toInt());

        QByteArray hashData = QCryptographicHash::hash(fileData,QCryptographicHash::Md5);

        m_isInfoChanged = true;
        return hashData.toHex();

    }
    return "";
}

void CMediaFile::addTag(QString szName, QString szValue)
{
    CFileTag tag(szName, szValue);

    if(szName.isEmpty()) return;

    m_tags.push_back(tag);
}

void CMediaFile::updateTag(QString szName, QString szValue)
{
    if(szName.isEmpty()) return;

    int i;

    for(i=0; i<m_tags.size(); i++)
    {
        if(m_tags[i].name() == szName)
        {
            m_tags[i].setValue(szValue);
            return;
        }
    }
    CFileTag tag(szName, szValue);
    m_tags.push_back(tag);
}

// Purpose: Read audio track information from media file
//
// - Uses: mplayer -identify -frames 0 fileName 2> /dev/null | grep ID_AUDIO_ID to get audio track info
//
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
        m_audioTracks.push_back(CAudioTrack(outputLine.split("=").at(1).toInt()));
        outputLine = reader.readLine();
    }
    m_isInfoChanged = true;
    return true;
}

void CMediaFile::readXMLAudioData(QXmlStreamReader *reader)
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
            m_audioTracks.push_back(CAudioTrack(id, szTrackName));
        }
        reader->readNext();
    }
}

// Purpose: Read media file data from xml
//
// Example of mediafile xml-node:
//    <media_file type="VOB">
//        <artist>Eppu Normaali</artist>
//        <title>Afrikka, sarvikuonojen maa</title>
//        <path>/home/timo/Karaoke/E/Eppu Normaali</path>
//        <file>Eppu Normaali - Afrikka, sarvikuonojen maa.vob</file>
//        <audio_tracks>
//            <audio_track id="0"></audio_track>
//            <audio_track id="1"></audio_track>
//        </audio_tracks>
//        <hash>35c368792eaa7462ae90ddeeba5c5b17</hash>
//        <file_size>236843008</file_size>
//        <created>Tue Dec 18 18:09:16 2012</created>
//        <modified>Wed Aug 4 21:02:10 2010</modified>
//        <tags>
//            <tag name="last_folder">Eppu Normaali</tag>
//        </tags>
//    </media_file>
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
                readXMLAudioData(reader);
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
            writer->writeAttribute(ATTR_ID, QString("%1").arg(m_audioTracks[i].getId()));
            writer->writeCharacters(m_audioTracks[i].getName());
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
            writer->writeAttribute(ATTR_NAME, m_tags[i].name());
            writer->writeCharacters(m_tags[i].value());
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

    if(m_isInfoChanged)
    {
        writeInfoFile();
        m_isInfoChanged = false;
    }
}





























