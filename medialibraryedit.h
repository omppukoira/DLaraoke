#ifndef MEDIALIBRARYEDIT_H
#define MEDIALIBRARYEDIT_H

#include <QDialog>
#include <QSharedPointer>
#include <QDataWidgetMapper>

#include "MediaFile.h"

namespace Ui {
class MediaLIbraryEdit;
}

class MediaLIbraryEdit : public QDialog
{
    Q_OBJECT
private:
    CMediaLibraryModel *m_Model;
    QDataWidgetMapper *m_Mapper;

    QSharedPointer<CMediaFileLibrary> getCurrentMediaLibrary();
    Ui::MediaLIbraryEdit *ui;

public:
    explicit MediaLIbraryEdit(QSharedPointer<CMediaFileLibraries> libraries, QWidget *parent = 0);
    ~MediaLIbraryEdit();

public slots:
    void changeLibrary(const QModelIndex &current, const QModelIndex &previous);
    void exportLibraryData();

private:
//    Ui::MediaLIbraryEdit *ui;
};

#endif // MEDIALIBRARYEDIT_H
