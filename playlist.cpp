#include "playlist.h"
#include "ctool.h"

#include <QFile>

const QString CPlaylist::NOD_PLAYLISTS =        "playlists";
const QString CPlaylist::NOD_PLAYLIST =         "playlist";
const QString CPlaylist::ATTR_NAME =            "name";
const QString CPlaylist::NOD_PLAYLIST_ITEM =    "playlist_item";

CPlaylist::CPlaylist(QString szName) :
    m_szName(szName)
{
}

QString CPlaylist::name()
{
    return m_szName;
}

void CPlaylist::setName(QString szName)
{
    m_szName = szName;
}

void CPlaylist::addArtistTitle(QString szArtistTitle)
{
    m_ArtistTitles.insert(szArtistTitle);
}

void CPlaylist::removeArtistTitle(QString szArtistTitle)
{
    m_ArtistTitles.erase(szArtistTitle);
}

bool CPlaylist::readXmlData(QXmlStreamReader *reader)
{
    if(IS_START_ELEM(reader, NOD_PLAYLIST))
    {
        m_szName = reader->attributes().value(ATTR_NAME).toString();
        reader->readNext();
        while(!reader->atEnd() && !(reader->isEndElement() && reader->name() == NOD_PLAYLIST))
        {
            if(IS_START_ELEM(reader, NOD_PLAYLIST_ITEM))
            {
                m_ArtistTitles.insert(reader->readElementText());
            }
            reader->readNext();
        }
    }
    return true;
}

bool CPlaylist::writeXmlData(QXmlStreamWriter *writer)
{
    tArtistTitles::iterator itAT;

    writer->writeStartElement(NOD_PLAYLIST);
    writer->writeAttribute(ATTR_NAME, m_szName);
    for(itAT = m_ArtistTitles.begin(); itAT != m_ArtistTitles.end(); itAT++)
    {
        writer->writeTextElement(NOD_PLAYLIST_ITEM, *itAT);
    }
    writer->writeEndElement();
    return true;
}

bool CPlaylist::isInPlaylist(QString szArtistTitle) const
{
    return m_ArtistTitles.find(szArtistTitle) != m_ArtistTitles.end();
}

CPlaylists::CPlaylists()
{}

CPlaylists::~CPlaylists()
{
    clear();
}

void CPlaylists::addPlaylist(CPlaylist *pPlaylist)
{
    m_Playlists.insert(tPlaylists::value_type(pPlaylist->name(), pPlaylist));
}

void CPlaylists::removePlaylist(QString szPlaylist)
{
    m_Playlists.erase(szPlaylist);
}

CPlaylist *CPlaylists::getPlaylist(QString szPlaylist)
{
    tPlaylists::iterator itPL;

    itPL = m_Playlists.find(szPlaylist);
    if(itPL != m_Playlists.end())
        return itPL->second;
    else
        return NULL;
}

CPlaylists::tPlaylists CPlaylists::getPlaylists()
{
    return m_Playlists;
}

void CPlaylists::clear()
{
    tPlaylists::iterator itPL;

    for(itPL = m_Playlists.begin(); itPL != m_Playlists.end(); itPL++)
    {
        delete itPL->second;
    }
    m_Playlists.clear();
    m_szFileName = "";
}

bool CPlaylists::readPlaylists(QString fileName)
{
    QFile fileIn(fileName);
    QXmlStreamReader reader;
    CPlaylist *pNewList = NULL;

    if(!fileIn.open(QFile::ReadOnly | QFile::Text))
    {
        return false;
    }

    reader.setDevice(&fileIn);
    while(!reader.atEnd() && !(reader.isStartElement() && reader.name() == CPlaylist::NOD_PLAYLISTS))
        reader.readNext();
    if(reader.atEnd()) return false;

    clear();

    while(!reader.atEnd() && !(reader.isEndElement() && reader.name() == CPlaylist::NOD_PLAYLISTS))
    {
        if(IS_START_ELEM(&reader, CPlaylist::NOD_PLAYLIST))
        {
            pNewList = new CPlaylist("");

            pNewList->readXmlData(&reader);
            if(!pNewList->name().isEmpty())
            {
                addPlaylist(pNewList);
            }
            else
                delete pNewList;
        }
        reader.readNext();
    }

    m_szFileName = fileName;
    return true;
}

bool CPlaylists::writePlaylists(QString fileName)
{
    QFile file(fileName);
    tPlaylists::iterator itPL;


    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        return false;   // Failed to open XML-file for writing
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeStartElement(CPlaylist::NOD_PLAYLISTS);
    for(itPL = m_Playlists.begin(); itPL != m_Playlists.end(); itPL++)
    {
        itPL->second->writeXmlData(&writer);
    }
    writer.writeEndElement();
    writer.writeEndDocument();

    m_szFileName = fileName;
    return true;
}

QString CPlaylists::fileName()
{
    return m_szFileName;
}



























