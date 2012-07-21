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
    Ui::MediaLIbraryEdit *ui;

    CMediaLibraryModel *m_Model;
    QDataWidgetMapper *m_Mapper;

    QSharedPointer<CMediaFileLibrary> getCurrentMediaLibrary();

public:
    explicit MediaLIbraryEdit(QSharedPointer<CMediaFileLibraries> libraries, QWidget *parent = 0);
    ~MediaLIbraryEdit();

public slots:
    void changeLibrary(const QModelIndex &current, const QModelIndex &previous);
    void exportLibraryData();
    void removeDuplicates();
    void relocateMediaFiles();

private:
};

#endif // MEDIALIBRARYEDIT_H
