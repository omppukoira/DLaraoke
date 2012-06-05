#ifndef MEDIAFILEVIEW_H
#define MEDIAFILEVIEW_H

#include <QTableView>

class MediaFileView : public QTableView
{
    Q_OBJECT
public:
    explicit MediaFileView(QWidget *parent = 0);

signals:
    void enterPressedSelection(QModelIndex index);
public slots:

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // MEDIAFILEVIEW_H
