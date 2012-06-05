#include "settingsedit.h"
#include "ui_settingsedit.h"

SettingsEdit::SettingsEdit(QString szOrganization, QString szApplication, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsEdit),
    m_szOrganization(szOrganization),
    m_szApplication(szApplication)
{
    ui->setupUi(this);
    ui->twSettings->setColumnCount(2);
    ui->twSettings->setHeaderLabels(QStringList()<<tr("Key") <<tr("Value"));
    ui->twSettings->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->twSettings->header()->setResizeMode(1, QHeaderView::Stretch);
    ui->twSettings->setItemDelegate(new EditorDelegate(ui->twSettings));
    readSettings();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
}

SettingsEdit::~SettingsEdit()
{
    delete ui;
}

void SettingsEdit::readSettings()
{
    QSettings settings(m_szOrganization, m_szApplication);

    setWindowTitle(tr("Settings [%1]/[%2]").arg(m_szOrganization).arg(m_szApplication));

    ui->twSettings->clear();
    addChildSettings(settings, NULL, "");
    ui->twSettings->sortByColumn(0);
    ui->twSettings->setFocus();
}

void SettingsEdit::addChildSettings(QSettings &settings, QTreeWidgetItem *parent, const QString &szGroup)
{
    QTreeWidgetItem *item;
    QString szFullSettingName;

    if(!parent)
        parent = ui->twSettings->invisibleRootItem();

    settings.beginGroup(szGroup);
    foreach(QString key, settings.childKeys())
    {
        item = new QTreeWidgetItem(parent);
        item->setText(0, key);
        item->setText(1, settings.value(key).toString());
        item->setText(2, settings.group());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    foreach(QString group, settings.childGroups())
    {
        item = new QTreeWidgetItem(parent);
        item->setText(0, group);
        addChildSettings(settings, item, group);
    }
    settings.endGroup();
}

void SettingsEdit::saveSettings()
{
    QSettings settings(m_szOrganization, m_szApplication);

    saveSettingsItem(settings, ui->twSettings->invisibleRootItem());
}

void SettingsEdit::saveSettingsItem(QSettings &settings, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item;
    QString szKey;
    int i;

    for(i=0; i<parent->childCount(); i++)
    {
        item = parent->child(i);

        if(!item->text(2).isEmpty())
        {
            szKey = QString("%1/%2").arg(item->text(2)).arg(item->text(0));
            settings.setValue(szKey, item->text(1));
        }

        if(item->childCount() > 0)
            saveSettingsItem(settings, item);
    }
}




























