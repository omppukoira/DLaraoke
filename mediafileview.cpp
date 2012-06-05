#include "mediafileview.h"

#include <QKeyEvent>

MediaFileView::MediaFileView(QWidget *parent) : QTableView(parent)
{ }

void MediaFileView::keyPressEvent(QKeyEvent *event)
{
     QModelIndexList selections;

    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        selections = this->selectedIndexes();
        if(selections.count() > 0)
            emit enterPressedSelection(selections[0]);
    }

    QTableView::keyPressEvent(event);
}
