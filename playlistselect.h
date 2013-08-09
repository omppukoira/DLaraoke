#ifndef PLAYLISTSELECT_H
#define PLAYLISTSELECT_H

#include "playlist.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class PlaylistSelect;
}

class PlaylistSelect : public QDialog
{
    Q_OBJECT
    
public:
    explicit PlaylistSelect(QWidget *parent, CPlaylists *pLists);
    ~PlaylistSelect();

    QString playlist()                      {return m_szPlaylist;}

protected:
    virtual bool eventFilter(QObject *target, QEvent *event);

private slots:
    void selectPlaylist(QListWidgetItem *pItem);

private:
    QString m_szPlaylist;
    Ui::PlaylistSelect *ui;
};

#endif // PLAYLISTSELECT_H
