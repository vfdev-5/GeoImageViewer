
// QT
#include <QValidator>
#include <QFile>


// Project
#include "BuildConfigDialog.h"

namespace Gui
{

//******************************************************************************

QMap<int, QString> versionyears;

BuildConfigDialog::BuildConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui_BuildConfigDialog)
{
    ui->setupUi(this);

#if (defined WIN32 || defined _WIN32 || defined WINCE)
    ui->_isWin->setVisible(true);
#else
    ui->_isWin->setVisible(false);
#endif

    if ((int)QSysInfo::WordSize == 32)
    {
        ui->_arch64->setVisible(false);
    }

    versionyears.insert(8, "2005");
    versionyears.insert(9, "2008");
    versionyears.insert(10, "2010");
    versionyears.insert(11, "2012");
    versionyears.insert(12, "2013");
    versionyears.insert(14, "2015");

}

//******************************************************************************

BuildConfigDialog::~BuildConfigDialog()
{
    delete ui;
}

//******************************************************************************

QString BuildConfigDialog::getCMakePath() const
{
    return ui->_cmakePath->text();
}

//******************************************************************************

QString BuildConfigDialog::getPATH() const
{
    return ui->_path->text();
}

//******************************************************************************

QString BuildConfigDialog::getGenerator() const
{
    QString g = QString("Visual Studio %1 %2")
            .arg(ui->_vsversion->value())
            .arg(ui->_vsyear->text());
    if (ui->_arch64->isVisible())
        g+=QString(" %1").arg(ui->_arch64->text());

    return g;
}

//******************************************************************************

void BuildConfigDialog::setCMakePath(const QString &path)
{
    ui->_cmakePath->setText(path);
}

//******************************************************************************

void BuildConfigDialog::setPATH(const QString &path)
{
    ui->_path->setText(path);
}

//******************************************************************************

void BuildConfigDialog::setGenerator(const QString &g)
{
    if (!g.isEmpty() && g.contains("Visual Studio "))
    {
        QStringList splt = g.split(" ");
        if (splt.size() > 2)
        {
            ui->_vsversion->setValue(g.split(" ")[2].toInt());
            return;
        }
    }
    ui->_vsversion->setValue(12);
}

//******************************************************************************

void BuildConfigDialog::on__buttons_accepted()
{
    accept();
}

//******************************************************************************

void BuildConfigDialog::on__vsversion_valueChanged(int i)
{
    ui->_vsyear->setText(QString("%1").arg(versionyears.value(i, " ")));
}

//******************************************************************************

}
