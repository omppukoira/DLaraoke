#include "mediafilterproxymodel.h"
#include "MediaFile.h"

MediaFilterProxyModel::MediaFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_iStars(3)
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

bool MediaFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, CMediaModel::MFM_Stars, sourceParent);

    if(stars() > sourceModel()->data(index0).toInt())
        return false;
    else
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
