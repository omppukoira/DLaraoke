#ifndef PLAYLISTEDIT_H
#define PLAYLISTEDIT_H

#include "playlist.h"

#include <map>
#include <QDialog>

using namespace std;

namespace Ui {
class PlaylistEdit;
}

class PlaylistEdit : public QDialog
{
    Q_OBJECT
    
public:
    explicit PlaylistEdit(QWidget *parent, CPlaylists *pPlaylists);
    ~PlaylistEdit();
    
private:
    Ui::PlaylistEdit *ui;
};

#endif // PLAYLISTEDIT_H
