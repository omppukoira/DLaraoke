#include "playlistselect.h"
#include "ui_playlistselect.h"

#include <QKeyEvent>

PlaylistSelect::PlaylistSelect(QWidget *parent, CPlaylists *pLists) :
    QDialog(parent),
    ui(new Ui::PlaylistSelect)
{
    CPlaylists::tPlaylists lists = pLists->getPlaylists();
    CPlaylists::tPlaylists::iterator itPL;
    QListWidgetItem *pItem;

    ui->setupUi(this);
    connect(ui->lstPlaylist,    SIGNAL(itemClicked(QListWidgetItem*)),  this, SLOT(selectPlaylist(QListWidgetItem*)));
    ui->lstPlaylist->installEventFilter(this);

    for(itPL = lists.begin(); itPL != lists.end(); itPL++)
    {
        pItem = new QListWidgetItem(itPL->first, ui->lstPlaylist);
    }
}

PlaylistSelect::~PlaylistSelect()
{
    delete ui;
}

void PlaylistSelect::selectPlaylist(QListWidgetItem *pItem)
{
    if(pItem)
    {
        m_szPlaylist = pItem->text();
        this->accept();
    }
}

bool PlaylistSelect::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->lstPlaylist)
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if(keyEvent && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return))
            {
                selectPlaylist(ui->lstPlaylist->currentItem());
                return true;
            }
        }
    return QDialog::eventFilter(target, event);
}
