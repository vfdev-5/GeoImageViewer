// Qt
#include <QFont>

// Project
#include "Highlighter.h"
#include "BuildConfigDialog.h"
#include "BuildErrorWidget.h"
#include "EditableFilterDialog.h"
#include "Filters/EditableFilter.h"

namespace Gui
{

//******************************************************************************

EditableFilterDialog::EditableFilterDialog(Filters::EditableFilter * f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui_EditableFilterDialog),
    _configDialog(new BuildConfigDialog()),
    _errorWidget(new BuildErrorWidget()),
    _model(f)
{

    ui->setupUi(this);

    setupEditor();

    // connect to model:
    connect(_model, &Filters::EditableFilter::badConfiguration, this, &EditableFilterDialog::onBadConfiguration);
    connect(_model, &Filters::EditableFilter::workFinished, this, &EditableFilterDialog::onWorkFinished);
    connect(_model, &Filters::EditableFilter::buildError, this, &EditableFilterDialog::onBuildError);

    QString program = _model->readSourceFile();
    if (!program.isEmpty())
    {
        ui->_code->setPlainText(program);
        _model->runTestCmake();
    }
    else
    {
        setUiEnabled(false);
    }
}

//******************************************************************************

EditableFilterDialog::~EditableFilterDialog()
{
    delete ui;
    delete _configDialog;
    delete _errorWidget;
}

//******************************************************************************

void EditableFilterDialog::closeEvent(QCloseEvent *)
{
    _errorWidget->close();
}

//******************************************************************************

void EditableFilterDialog::onBadConfiguration()
{
    setUiEnabled(true);
    configure();
}

//******************************************************************************

void EditableFilterDialog::onWorkFinished(bool ok)
{
    setUiEnabled(true);
}

//******************************************************************************

void EditableFilterDialog::onBuildError(const QString & err)
{
    _errorWidget->appendText(err);
    if (!_errorWidget->isVisible())
        _errorWidget->show();
}

//******************************************************************************

void EditableFilterDialog::setupEditor()
{
    QFont font;
    font.setFamily("Monospace");
    font.setFixedPitch(true);
    font.setPointSize(11);

    ui->_code->setFont(font);

    _highlighter = new Highlighter(ui->_code->document());
}

//******************************************************************************

void EditableFilterDialog::setUiEnabled(bool v)
{
    ui->_apply->setEnabled(v);
    ui->_code->setEnabled(v);
    ui->_configure->setEnabled(v);
}



//******************************************************************************

void EditableFilterDialog::on__configure_clicked()
{
    configure();
}

//******************************************************************************

void EditableFilterDialog::on__apply_clicked()
{
    setUiEnabled(false);

    _errorWidget->clean();
    _errorWidget->close();

    _model->apply(ui->_code->toPlainText());

}

//******************************************************************************

void EditableFilterDialog::configure()
{
    SD_TRACE("Start build configuration dialog");

    _configDialog->setCMakePath(_model->getCMakePath());
    _configDialog->setPATH(_model->getPATH());
    _configDialog->setGenerator(_model->getCMakeGenerator());

    if (_configDialog->exec() == QDialog::Accepted)
    {
        _model->setCMakePath(_configDialog->getCMakePath());
        _model->setPATH(_configDialog->getPATH());
        _model->setCMakeGenerator(_configDialog->getGenerator());
        if (_model->removeBuildCache())
        {
            _model->runTestCmake();
        }
    }
    SD_TRACE1("CMake path : %1", _model->getCMakePath());
    SD_TRACE1("CMake generator : %1", _model->getCMakeGenerator());
    SD_TRACE1("PATH : %1", _model->getPATH());
}

//******************************************************************************

}
