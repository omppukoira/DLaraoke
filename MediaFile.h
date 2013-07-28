#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <QFileInfo>
#include <QAbstractTableModel>
#include <QXmlStreamWriter>
#include <QProgressDialog>
#include <QDir>
#include <QDateTime>

#include <vector>


using namespace std;

class CMediaFile;                   // Media file
class CMediaLibrary;                // Media library containig media files

// Purpose: Model for viewing loaded media files
//
class CMediaModel : public QAbstractTableModel
{
protected:
    CMediaLibrary *m_pLibrary;
public:
    CMediaModel(QObject *parent=0);
    virtual ~CMediaModel();

    typedef enum
    {
        MFM_Artist = 0,
        MFM_Title = 1,
        MFM_Type = 2,
        MFM_SearchText = 3,                  // Text to be searched
        MFM_Date = 4
    } tMFMCols;

    void setMediaLibrary(CMediaLibrary *pLib);
    void resetModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

class CDetailNode
{
public:
    enum Type       {Root, Artist, MediaFile, MediaFileInfo, AudioTracks, AudioTrack, Tags, Tag};
    enum InfoType   {PathInLib, Size, Hash};

protected:
    Type m_Type;
    CMediaFile* m_pMediaFile;
    vector<CDetailNode*> m_Children;
    CDetailNode *m_pParent;
    int m_iRowNo;

public:
    CDetailNode(Type type, CMediaFile *pMediaFile);
    ~CDetailNode();

    CDetailNode *parent() const;
    void setParent(CDetailNode *pParent);

    Type type() const;
    CMediaFile *mediaFile() const;

    CDetailNode *child(int index) const;
    int childCount() const;

    int rowNo() const;
    void setRowNo(int iRowNo);

    void addChild(CDetailNode *pChild);
};

class CDetailMediaModel : public QAbstractItemModel
{
protected:
    CDetailNode *m_pRoot;

    CDetailNode *findArtist(QString szName) const;
    CDetailNode *nodeFromIndex(const QModelIndex &index) const;
public:
    CDetailMediaModel(QObject *parent=0);
    ~CDetailMediaModel();

    void setMediaLibrary(CMediaLibrary *pLib);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

// Contains all media files found from given path
//
class CMediaLibrary
{
public:
    // consts for XML-nodes
    //
    static const QString NOD_MEDIA_LIBRARY;
    static const QString ATTR_TYPE;
    static const QString NOD_LOCATION;
    static const QString NOD_MEDIA_FILES;
    static const QString NOD_FORE_COLOR;
    static const QString NOD_BACK_COLOR;

    static const QString m_szMediaFileTypes;    // List of supported mediafiles

protected:
    QString m_szMediaLibLocation,               // Location of meida library
            m_szName;                           // Media library name
    
    typedef vector<CMediaFile *> tMediaFiles;
    tMediaFiles m_MediaFiles;                   // All media files in library

    void addFile(CMediaFile *pFile);

    void addFiles(QString szDirectory, bool bPreScan, QProgressDialog *pPrgDlg);

public:
    CMediaLibrary();
    ~CMediaLibrary();

    void scanMediaFiles(QString szMediaLib, bool bPreScan, QProgressDialog *pPrgDlg);

    unsigned int count();
    void clear();

    CMediaFile *getMediaFile(int index) const;

    void readXmlData(QXmlStreamReader *reader);
    void writeXmlData(QXmlStreamWriter *writer);

    QString getName() const                     {return m_szName;}
    void setName(QString szName)                {m_szName = szName;}

    QString getLocation()                       {return m_szMediaLibLocation;}
};

class CAudioTrack
{
private:
    int m_iId;
    QString m_szName;
public:
    CAudioTrack();
    CAudioTrack(int iId);
    CAudioTrack(int iId, QString szName);

    CAudioTrack &operator = (int iId);

    int getId()                         {return m_iId;}
    QString getName()                   {return m_szName;}
    void setName(QString szName)        {m_szName = szName;}
};

class CFileTag
{
private:
    QString m_szName, m_szValue;
public:
    CFileTag();
    CFileTag(QString szName, QString szValue);

    QString name() const;
    void setName(QString szName);

    QString value() const;
    void setValue(QString szValue);
};

// Purpose: One media file file in media library
//
class CMediaFile
{
public:
    static const QString NOD_MEDIA_FILE, ATTR_FILE_TYPE;
    static const QString NOD_ARTIST, NOD_TITLE;
    static const QString NOD_PATH, NOD_FILE, NOD_NAME;
    static const QString NOD_AUDIO_TRACKS, NOD_AUDIO_TRACK, ATTR_ID;
    static const QString NOD_HASH, NOD_SIZE, NOD_CREATED, NOD_MODIFIED;
    static const QString NOD_TAGS,NOD_TAG, ATTR_NAME;

    static const QString NOD_FILEINFO;

    static const QString TAG_DUPLICATE;     // Full name (ie. absolute path to) of duplicate file
    static const QString TAG_LAST_FOLDER;   // Folder name where file or its duplicata is

    typedef enum
    {
        MF_Unknown = 0,                     // File format is unknown for this class
        MF_VOB = 1,                         // VOB: Ripped from karaoke-dvd, may contain several sound tracks
        MF_AVI = 2,                         // AVI: Ripped from karaoke dvd, contains only one sound track
        MF_CDG = 3                          // CDG: Consist from two files .cdg and .mp3
    } tMediaFileType;

private:
    QString m_szFilePaht,                   // Location of the file in media lib
            m_szFileName;                   // Name of the file including extension
    QString m_szArtist,                     // Name of the artist in media file
            m_szTitle;                      // Song title
    tMediaFileType m_FileType;
    vector<CAudioTrack> m_audioTracks;
                                            // ID's of audiotracks
    vector<CFileTag> m_tags;

    qint64 m_iFileSize;
    QString m_szHash;
    QDateTime m_Created, m_Modified;

    CMediaLibrary *m_pMediaLibrary;

    QString getInfoFile();                  // Get full location of info file
    void readXMLAudioData(QXmlStreamReader *reader);

    bool m_isInfoChanged;
public:
    CMediaFile();

    bool Create(QFileInfo &mediaFile, QString szPath);

    tMediaFileType getType() const          {return m_FileType;}
    QString getPaht() const                 {return m_szFilePaht;}
    QString getFileName() const             {return m_szFileName;}
    QString getFullName() const             {return m_szFilePaht + QDir::separator() + m_szFileName;}
    QString getArtist() const               {return m_szArtist;}
    QString getTitle() const                {return m_szTitle;}

    int getSize() const                     {return m_iFileSize;}
    QDateTime getCreated() const            {return m_Created;}
    QDateTime getModified() const           {return m_Modified;}
    QString getHash() const                 {return m_szHash;}
    void setHash(QString szHash)            {
                                                m_szHash = szHash;
                                                m_isInfoChanged = true;
                                            }
    QString calcHash();

    CMediaLibrary *getParent() const        {return m_pMediaLibrary;}
    void setParent(CMediaLibrary *pParent)  {m_pMediaLibrary = pParent;}

    QString getExecCmd() const;
    QString getPahtInLibrary() const;

    void addTag(QString szName, QString szValue);
    void updateTag(QString szName, QString szValue);

    vector<CAudioTrack> audioTracks() const {return m_audioTracks;}
    bool readAudioTrackInfo();

    void readXmlData(QXmlStreamReader *reader);
    void writeXmlData(QXmlStreamWriter *writer);
    bool readInfoFile();                    // Read data from info file
    bool writeInfoFile();                   // Write Info file

    bool isInfoChanged()                    {return m_isInfoChanged;}

    bool moveToPreferredLocation(CMediaLibrary *pMediaLib);
};


#endif // MEDIAFILE_H
