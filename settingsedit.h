#ifndef SETTINGSEDIT_H
#define SETTINGSEDIT_H

#include <QDialog>
#include <QSettings>
#include <QTreeWidget>
#include <QItemDelegate>

namespace Ui {
class SettingsEdit;
}

class SettingsEdit : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsEdit(QString szOrganization, QString szApplication, QWidget *parent = 0);
    ~SettingsEdit();
    
private:
    Ui::SettingsEdit *ui;

    QString m_szOrganization, m_szApplication;

    void readSettings();
    void addChildSettings(QSettings &settings, QTreeWidgetItem *parent, const QString &szGroup);
    void saveSettingsItem(QSettings &settings, QTreeWidgetItem *parent);

public slots:
    void saveSettings();
};

class EditorDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    EditorDelegate(QObject *parent):QItemDelegate(parent)
    {

    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if(index.column() == 1)
        {
            return QItemDelegate::createEditor(parent, option, index);
        }
        else
        {
            return NULL;
        }
    }
};

#endif // SETTINGSEDIT_H
