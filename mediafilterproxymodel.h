#ifndef MEIAFILTERPROXYMODEL_H
#define MEIAFILTERPROXYMODEL_H

#include "playlist.h"

#include <QSortFilterProxyModel>

class MediaFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MediaFilterProxyModel(QObject *parent = 0);
    
    int stars() const;
    void setStars(int iStars);

    CPlaylist *playlist();
    void setPlaylist(CPlaylist *pList);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    int m_iStars;
    CPlaylist *m_pPlaylist;
};

#endif // MEIAFILTERPROXYMODEL_H
