#include "playlistedit.h"
#include "ui_playlistedit.h"

#include <QListWidgetItem>

PlaylistEdit::PlaylistEdit(QWidget *parent, CPlaylists *pPlaylists) :
    QDialog(parent),
    ui(new Ui::PlaylistEdit)
{
    QListWidgetItem *pItem;
    CPlaylists::tPlaylists playlists = pPlaylists->getPlaylists();
    CPlaylists::tPlaylists::iterator itPL;

    ui->setupUi(this);

    for(itPL = playlists.begin(); itPL != playlists.end(); itPL++)
    {
        pItem = new QListWidgetItem(itPL->first, ui->lstPlaylists);
    }

}

PlaylistEdit::~PlaylistEdit()
{
    delete ui;
}
