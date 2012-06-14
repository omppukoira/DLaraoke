#include "MediaFile.h"

#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QColor>
#include <QSettings>
#include <QCryptographicHash>

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

const QString CMediaFileLibrary::m_szMediaFileTypes = "*.VOB;*.AVI;*.MP3";

const QString CMediaFileLibrary::NOD_MEDIA_LIBRARY  = "media_library";
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

    qDebug() << "CMediaLibraryModel::rowCount"<< parent.row() << parent.column() << "->" << iRowCount;
    return iRowCount;
}

int CMediaLibraryModel::columnCount(const QModelIndex & /* parent */) const
{
    return MLM_FilePaht+1;
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
                qDebug() << CTool::xmlNodeInfo(*reader);

                if(IS_START_ELEM(reader, CMediaFileLibrary::NOD_MEDIA_LIBRARY))
                {
                    QSharedPointer<CMediaFileLibrary> lib = QSharedPointer<CMediaFileLibrary>(new CMediaFileLibrary());

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
        qDebug() << CTool::xmlNodeInfo(*reader);
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

    for(i=0; i<m_MediaFiles.size(); i++)
        if(m_MediaFiles[i]->getHash() == file->getHash())
        {
            isUnique = false;
            break;
        }

    file->setParent(m_This);
    m_MediaFiles.push_back(file);
    if(isUnique)
    {
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
        pPrgDlg->setLabelText(szDirectory);

    foreach (QString file, mlib.entryList(szFilters, QDir::Files))
    {
        QSharedPointer<CMediaFile> pMediaFile;
        QFileInfo fi(file);

        if(!fi.isDir())
        {
            pMediaFile = QSharedPointer<CMediaFile>(new CMediaFile());

            pMediaFile->Create(QFileInfo(file), szDirectory);
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
                    pPrgDlg->setValue(m_MediaFiles.size());
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

CMediaFile::CAudioTrack::CAudioTrack()
    : m_iId(0)
{}

CMediaFile::CAudioTrack::CAudioTrack(int iId)
    : m_iId(iId)
{}

CMediaFile::CAudioTrack::CAudioTrack(int iId, QString szName)
    : m_iId(iId), m_szName(szName)
{}

CMediaFile::CAudioTrack &CMediaFile::CAudioTrack::operator =(int iId)
{
    m_iId = iId;
    m_szName = QString("Audio track [%1]").arg(iId);
    return *this;
}

// Purpose: Constructs media file object
//
CMediaFile::CMediaFile()
    : m_FileType(MF_Unknown)
{}

bool CMediaFile::Create(QFileInfo mediaFile, QString szPath)
{
    m_szFilePaht = szPath;
    m_szFileName = mediaFile.fileName();

    if(mediaFile.suffix().toUpper() == "VOB")
        m_FileType = MF_VOB;
    else if(mediaFile.suffix().toUpper() == "AVI")
        m_FileType = MF_AVI;
    else if(mediaFile.suffix().toUpper() == "MP3")
        m_FileType = MF_CDG;
    else
        m_FileType = MF_Unknown;

    int iPos = m_szFileName.lastIndexOf("-");
    if(iPos > 0)
    {
        m_szTitle = m_szFileName.left(iPos).simplified();
        m_szArtist = m_szFileName.mid(iPos+1).simplified();
    }
    else
        m_szTitle = m_szFileName;

    iPos = m_szArtist.lastIndexOf(".");     // Artist name contains now file extension -> Get rid of it
    if(iPos > 0)
        m_szArtist = m_szArtist.left(iPos).simplified();

    iPos = m_szTitle.lastIndexOf(".");      // When artist is not given file extension is in title...
    if(iPos > 0)
        m_szTitle = m_szTitle.left(iPos).simplified();

    if(m_szArtist == "" && m_szTitle == "")
    {
        qDebug() << "ERROR: File"<< m_szFileName <<"Artist and Title is empty";
        m_FileType = MF_Unknown;
    }
    return true;
}

QString CMediaFile::getExecCmd()
{
    QString cmd, player;
    QSettings settings("Duo Liukko", "Karaoke");

    switch(m_FileType)
    {
    case MF_VOB:
        player = settings.value("Media Player/VOB").toString();
        break;
    case MF_AVI:
        player = settings.value("Media Player/AVI").toString();
        break;
    case MF_CDG:
        player = settings.value("Media Player/CDG").toString();
        break;
    default:
        return "";
    }

    cmd = player.arg(m_szFilePaht + QDir::separator() + m_szFileName);
    return cmd;
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
QWeakPointer<CMediaFileLibrary> CMediaFile::getParent()
{
    return m_Parent;
}

void CMediaFile::setParent(QSharedPointer<CMediaFileLibrary> mediaLib)
{
    m_Parent = mediaLib;
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
    writer->writeEndElement();
}





























