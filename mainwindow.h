#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QProcess>
#include <QLabel>
#include <QSharedPointer>

#include "MediaFile.h"

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
    CMediaFileModel *m_pFiles;                  // Media file view information
    CDetailMediaFileModel *m_pDetailFiles;      // Detailled file view info

    QSharedPointer<CMediaFileLibraries> m_Libraries;
    QSortFilterProxyModel *m_pSortModel;        // Sort & Search filter for media files
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
    void editMediaLibraries();
    void editSettings();

    void playFile(const QModelIndex &index);
    void searchTextChange(QString szText);
};

#endif // MAINWINDOW_H
