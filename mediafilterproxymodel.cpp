#include "mediafilterproxymodel.h"
#include "MediaFile.h"

MediaFilterProxyModel::MediaFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_iStars(3),
    m_pPlaylist(NULL)
{
}

int MediaFilterProxyModel::stars() const
{
    return m_iStars;
}

void MediaFilterProxyModel::setStars(int iStars)
{
    if(iStars != m_iStars)
    {
        m_iStars = iStars;
        invalidateFilter();
    }

}

CPlaylist *MediaFilterProxyModel::playlist()
{
    return m_pPlaylist;
}

void MediaFilterProxyModel::setPlaylist(CPlaylist *pList)
{
    if(m_pPlaylist != pList)
    {
        m_pPlaylist = pList;
        invalidateFilter();
    }
}

bool MediaFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, CMediaModel::MFM_Stars, sourceParent);

    if(m_pPlaylist != NULL)
    {
        QModelIndex indexA = sourceModel()->index(sourceRow, CMediaModel::MFM_Artist, sourceParent),
                    indexT = sourceModel()->index(sourceRow, CMediaModel::MFM_Title, sourceParent);

        QString artistTitle;

        artistTitle = sourceModel()->data(indexA).toString();
        artistTitle+= " - ";
        artistTitle+= sourceModel()->data(indexT).toString();

        if(!m_pPlaylist->isInPlaylist(artistTitle))
            return false;
    }

    if(stars() > sourceModel()->data(index0).toInt())
        return false;
    else
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
