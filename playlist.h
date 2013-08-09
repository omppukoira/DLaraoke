#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <set>
#include <map>

using namespace std;

class CPlaylist
{
public:
    static const QString    NOD_PLAYLISTS,
                            NOD_PLAYLIST,
                            ATTR_NAME,
                            NOD_PLAYLIST_ITEM;

    CPlaylist(QString szName="");

    QString name();
    void setName(QString szName);

    void addArtistTitle(QString szArtistTitle);
    void removeArtistTitle(QString szArtistTitle);

    bool readXmlData(QXmlStreamReader *reader);
    bool writeXmlData(QXmlStreamWriter *writer);

    bool isInPlaylist(QString szArtistTitle) const;
protected:
    QString m_szName;

    typedef set<QString> tArtistTitles;
    tArtistTitles m_ArtistTitles;
};

class CPlaylists
{
public:
    typedef map<QString, CPlaylist *> tPlaylists;

    CPlaylists();
    ~CPlaylists();

    void addPlaylist(CPlaylist *pPlaylist);
    void removePlaylist(QString szPlaylist);
    CPlaylist *getPlaylist(QString szPlaylist);
    tPlaylists getPlaylists();
    void clear();

    bool readPlaylists(QString fileName);
    bool writePlaylists(QString fileName);

    QString fileName();
protected:
    tPlaylists m_Playlists;
    QString m_szFileName;
private:
    CPlaylists(CPlaylists& other);
};
#endif // PLAYLIST_H
