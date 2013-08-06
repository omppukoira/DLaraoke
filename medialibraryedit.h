#ifndef MEDIALIBRARYEDIT_H
#define MEDIALIBRARYEDIT_H

#include "MediaFile.h"

#include <QDialog>

namespace Ui {
class CMediaLibraryEdit;
}

class CMediaLibraryEdit : public QDialog
{
    Q_OBJECT
    
public:
    explicit CMediaLibraryEdit(QWidget *parent = 0);
    ~CMediaLibraryEdit();
    
private:
    Ui::CMediaLibraryEdit *ui;

    CMediaLibrary   *m_pSrcLib,
                    *m_pDestLib;

    void updateCommandButtons();
    CMediaLibrary *selectMediaLibrary(QString szcaption);
private slots:
    void selectSrcLib();
    void selectDestLib();
    void copyNewFiles();
    void renameDestFiles();
};

#endif // MEDIALIBRARYEDIT_H
