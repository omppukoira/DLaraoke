#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QProcess>
#include <QLabel>

#include "MediaFile.h"
#include "mediafilterproxymodel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    CMediaModel *m_pFiles;                      // Media file view information
    CMediaLibrary *m_pLibrary;                  // Current media library

    MediaFilterProxyModel *m_pSortModel;        // Sort & Search filter for media files
    QProcess m_player;
    QLabel *m_pInfo;

    void readMediaData(QString szFileName);
    void writeMediaData(QString fileName);

protected:
    virtual bool eventFilter(QObject *target, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

    void readSettings();
    void writeSettings();

    bool appendReadCancel();

    void setFilters();

private slots:
    void scanMediaFiles();
    void readMediaData();
    void writeMediaData();
    void editSettings();
    void editMediaLibraries();

    void playCurrent();
    void playFile(const QModelIndex &index);
    void searchTextChange(QString szText);

    void setViewStars();

    void renameCurrent();
};

#endif // MAINWINDOW_H
