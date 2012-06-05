#include <QStringListModel>

#include "medialibraryedit.h"
#include "ui_medialibraryedit.h"

MediaLIbraryEdit::MediaLIbraryEdit(QSharedPointer<CMediaFileLibraries> libraries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MediaLIbraryEdit),
    m_Model(new CMediaLibraryModel)
{
    QStringListModel *libs = new QStringListModel(this);

    m_Model->setMediaLibraries(libraries);
    ui->setupUi(this);

    m_Mapper = new QDataWidgetMapper(this);
    m_Mapper->setModel(m_Model.operator ->());
    m_Mapper->addMapping(ui->leName, CMediaLibraryModel::MLM_Name);
    m_Mapper->addMapping(ui->leBackColor, CMediaLibraryModel::MLM_BackColor);
    m_Mapper->addMapping(ui->leForeColor, CMediaLibraryModel::MLM_ForeColor);
    m_Mapper->addMapping(ui->leLocation, CMediaLibraryModel::MLM_FilePaht);
    m_Mapper->toFirst();

    libs->setStringList(libraries->getLibraryNames());
    ui->lstLibraries->setModel(libs);
    connect(ui->lstLibraries->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(changeLibrary(QModelIndex,QModelIndex)));
}

MediaLIbraryEdit::~MediaLIbraryEdit()
{
    delete ui;
}

void MediaLIbraryEdit::changeLibrary(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    m_Mapper->setCurrentIndex(current.row());
}
