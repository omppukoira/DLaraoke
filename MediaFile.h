#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <QFileInfo>
#include <QAbstractTableModel>
#include <QXmlStreamWriter>
#include <QSharedPointer>
#include <QProgressDialog>
#include <QDir>
#include <QDateTime>

#include <vector>


using namespace std;

class CMediaFile;                   // Media file
class CMediaFileLibrary;            // Media library containig media files
class CMediaFileLibraries;          // Collection of media library

class CMediaLibraryModel : public QAbstractTableModel
{
public:
    typedef enum
    {
        MLM_Name = 0,
        MLM_BackColor = 1,
        MLM_ForeColor = 2,
        MLM_FilePaht = 3,
        MLM_MainLib = 4,
        MLM_Columns = 5
    }
    tMLMCols;

protected:
    QSharedPointer<CMediaFileLibraries> m_Libraries;

public:
    CMediaLibraryModel(QObject *parent=0);
    virtual ~CMediaLibraryModel();


    QSharedPointer<CMediaFileLibraries> getMediaLibraries();

    void setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs);
    void resetModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    // QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

// Purpose: Model for viewing media files in media libraries
//
class CMediaFileModel : public QAbstractTableModel
{
protected:
    QSharedPointer<CMediaFileLibraries> m_Libraries;

public:
    CMediaFileModel(QObject *parent=0);
    virtual ~CMediaFileModel();

    typedef enum
    {
        MFM_Artist = 0,
        MFM_Title = 1,
        MFM_Type = 2,
        MFM_SearchText = 3                  // Text to be searched
    } tMFMCols;

    void setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs);
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
    QSharedPointer<CMediaFile> m_MediaFile;
    vector<CDetailNode*> m_Children;
    CDetailNode *m_pParent;
    int m_iRowNo;

public:
    CDetailNode(Type type, QSharedPointer<CMediaFile> mediaFile);
    ~CDetailNode();

    CDetailNode *parent() const;
    void setParent(CDetailNode *pParent);

    Type type() const;
    QSharedPointer<CMediaFile> mediaFile() const;

    CDetailNode *child(int index) const;
    int childCount() const;

    int rowNo() const;
    void setRowNo(int iRowNo);

    void addChild(CDetailNode *pChild);
};

class CDetailMediaFileModel : public QAbstractItemModel
{
protected:
    CDetailNode *m_pRoot;

    CDetailNode *findArtist(QString szName) const;
    CDetailNode *nodeFromIndex(const QModelIndex &index) const;
public:
    CDetailMediaFileModel(QObject *parent=0);
    ~CDetailMediaFileModel();

    void setMediaLibraries(QSharedPointer<CMediaFileLibraries> libs);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

// Purpose: Container for media libraries
//
class CMediaFileLibraries
{
protected:
    vector<QSharedPointer<CMediaFileLibrary> > m_Libs;
    typedef vector<QSharedPointer<CMediaFile> > tMediaFiles;
    tMediaFiles m_MediaFiles, m_UniqueMediaFiles;

    bool m_bUniqueFiles;                        // Show only unique files

    bool isUniqueFile(QSharedPointer<CMediaFile> file);
    void collectMediaFiles();
public:
    // Purpose: Number on medialibraries
    //
    int count()                                 {return m_Libs.size();}

    void clear()                                {
                                                    m_Libs.clear();
                                                    collectMediaFiles();
                                                }

    int fileCount();

    bool showUniqueFiles()                      {return m_bUniqueFiles;}
    void setShowUniqueFiles(bool bUniqueFiles)  {m_bUniqueFiles = bUniqueFiles;}

    QStringList getLibraryNames();
    QSharedPointer<CMediaFile> getMediaFile(int index) const;
    QSharedPointer<CMediaFileLibrary> getMediaLibrary(int index) const;

    void addMediaLibrary(QSharedPointer<CMediaFileLibrary> mediaLib);
    void readMediaLibraries(QXmlStreamReader *reader);
    void writeMediaLibraries(QXmlStreamWriter *writer);

};

class CMediaFileLibrary
{
public:
    static const QString NOD_MEDIA_LIBRARY;
    static const QString ATTR_TYPE;
    static const QString NOD_LOCATION;
    static const QString NOD_MEDIA_FILES;
    static const QString NOD_FORE_COLOR;
    static const QString NOD_BACK_COLOR;
    static const QString m_szMediaFileTypes;// List of supported mediafiles

protected:
    QString m_szMediaLibLocation;           // Location of meida library
    QString m_szName;
    QString m_szForeColor, m_szBackColor;   // Color definition for whowing different libraries
    bool m_bUniqueFiles;                    // Show only unique files (default: yes)
    bool m_bIsMainLib;                      // Main library... copy files to this library ?

    QWeakPointer<CMediaFileLibrary> m_This;
    typedef vector<QSharedPointer<CMediaFile> > tMediaFiles;
    tMediaFiles m_MediaFiles, m_UniqueMediaFiles;

    void addFile(QSharedPointer<CMediaFile> file);

    void addFiles(QString szDirectory, bool bPreScan, QProgressDialog *pPrgDlg);
    QString setColor(QString szColor);

public:
    void setThis(QWeakPointer<CMediaFileLibrary> wpThis);

    void scanMediaFiles(QString szMediaLib, bool bPreScan, QProgressDialog *pPrgDlg);
    void readXmlData(QXmlStreamReader *reader);
    void writeXmlData(QXmlStreamWriter *writer);

    int fileCount();
    int uniqueFileCount();

    unsigned int count();

    QSharedPointer<CMediaFile> getMediaFile(int index);

    QString getName() const                     {return m_szName;}
    void setName(QString szName)                {m_szName = szName;}

    QString getForeColor() const                {return m_szForeColor;}
    void setForeColor(QString szColor)          {m_szForeColor = setColor(szColor);}

    QString getBackColor() const                {return m_szBackColor;}
    void setBackColr(QString szColor)           {m_szBackColor = setColor(szColor);}

    QString getLocation()                       {return m_szMediaLibLocation;}

    bool showUniqueFiles()                      {return m_bUniqueFiles;}
    void setShowUniqueFiles(bool bUniqueFiles)  {m_bUniqueFiles = bUniqueFiles;}

    bool isMain()                               {return m_bIsMainLib;}
    void setIsMain(bool bIsMain)                {m_bIsMainLib = bIsMain;}
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

class CMediaFile
{
public:
    static const QString NOD_MEDIA_FILE, ATTR_FILE_TYPE;
    static const QString NOD_ARTIST, NOD_TITLE;
    static const QString NOD_PATH, NOD_FILE, NOD_NAME;
    static const QString NOD_AUDIO_TRACKS, NOD_AUDIO_TRACK, ATTR_ID;
    static const QString NOD_HASH, NOD_SIZE, NOD_CREATED, NOD_MODIFIED;
    static const QString NOD_TAGS,NOD_TAG, ATTR_NAME;

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
    QWeakPointer<CMediaFileLibrary> m_Parent;

    QString m_szFilePaht,                   // Location of the file in media lib
            m_szFileName;                   // Name of the file including extension
    QString m_szArtist,                     // Name of the artist in media file
            m_szTitle;                      // Song title
    tMediaFileType m_FileType;
    vector<QSharedPointer<CAudioTrack> > m_audioTracks;
                                            // ID's of audiotracks
    vector<QSharedPointer<CFileTag> > m_tags;

    qint64 m_iFileSize;
    QString m_szHash;
    QDateTime m_Created, m_Modified;

    bool m_bIsUnique,                       // Is this file unique in Media library (m_Parent) ?
         m_bIsGlobalUnique;                 // Is this file unique in all loaded media libraries ?

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
    void setHash(QString szHash)            {m_szHash = szHash;}
    QString calcHash();

    bool isUnique() const                   {return m_bIsUnique;}
    void setIsUnique(bool bIsUnique)        {m_bIsUnique = bIsUnique;}

    bool isGlobalUnique() const             {return m_bIsGlobalUnique;}
    void setIsGlobalUnique(bool bIsGlobalUnique)
                                            {m_bIsGlobalUnique = bIsGlobalUnique;}
    QString getExecCmd() const;
    QString getPahtInLibrary() const;
    bool moveToPreferredLocation(QSharedPointer<CMediaFileLibrary> mediaLib);

    QWeakPointer<CMediaFileLibrary> getParent() const;
    void setParent(QSharedPointer<CMediaFileLibrary> mediaLib);

    void addTag(QString szName, QString szValue);
    void updateTag(QString szName, QString szValue);

    vector<QSharedPointer<CAudioTrack> > *audioTracks()
                                            {return &m_audioTracks;}
    bool readAudioTrackInfo();

    void readXmlData(QXmlStreamReader *reader);
    void writeXmlData(QXmlStreamWriter *writer);
};


#endif // MEDIAFILE_H
