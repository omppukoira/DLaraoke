#ifndef MEIAFILTERPROXYMODEL_H
#define MEIAFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class MediaFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MediaFilterProxyModel(QObject *parent = 0);
    
    int stars() const;
    void setStars(int iStars);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    int m_iStars;
};

#endif // MEIAFILTERPROXYMODEL_H
