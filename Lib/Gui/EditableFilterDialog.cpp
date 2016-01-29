// Qt
#include <QFont>
#include <QSettings>

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
    BaseFilterDialog(tr("Editable filter dialog"), parent),
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

    // Configure the model:
    getConfigFromSettings();
    _model->runTestCmake();

    refreshCode();
}

//******************************************************************************

EditableFilterDialog::~EditableFilterDialog()
{
    delete ui;
    delete _configDialog;
    delete _errorWidget;
}

//******************************************************************************

void EditableFilterDialog::closeEvent(QCloseEvent * event)
{
    _errorWidget->close();
    QWidget::closeEvent(event);
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
    if (ok)
    {
        emit applyFilter();
    }
}

//******************************************************************************

void EditableFilterDialog::onBuildError(const QString & err)
{
    _errorWidget->appendText(err);
    if (!_errorWidget->isVisible())
        _errorWidget->show();
}

//******************************************************************************

void EditableFilterDialog::refreshCode()
{
    QString program = _model->readSourceFile();
    if (!program.isEmpty())
    {
        ui->_code->setPlainText(program);
    }
    else
    {
        setUiEnabled(false);
    }
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

void EditableFilterDialog::on__refresh_clicked()
{
    refreshCode();
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
        if (_model->getCMakeGenerator() != _configDialog->getGenerator())
        {
            if (!_model->removeBuildCache())
            {
                SD_ERR(tr("Failed to remove build cache. Please, remove manually the folder 'Build' at '<INSTALLATION_FOLDER>/Resources/Build'"));
                return;
            }
        }
        _model->setCMakePath(_configDialog->getCMakePath());
        _model->setPATH(_configDialog->getPATH());
        _model->setCMakeGenerator(_configDialog->getGenerator());
		setConfigToSettings();
		_model->runTestCmake();
    }
}

//******************************************************************************

void EditableFilterDialog::getConfigFromSettings()
{
    // Restore settings:
    QSettings settings("GeoImageViewer_dot_com", "GIV");
    if (settings.contains("EditableFilter/CMakePath"))
    {
        _model->setCMakePath(settings.value("EditableFilter/CMakePath").toString());
    }
    if (settings.contains("EditableFilter/PATH"))
    {
        _model->setPATH(settings.value("EditableFilter/PATH").toString());
    }

#if (defined WIN32 || defined _WIN32 || defined WINCE)
    // Restore the generator
    if (settings.contains("EditableFilter/CMakeGenerator"))
    {
        _model->setCMakeGenerator(settings.value("EditableFilter/CMakeGenerator").toString());
    }
    else
    {
        SD_TRACE("WIN32 : GENERATOR IS EMTPY");
        configure();
    }
#endif

}

//******************************************************************************

void EditableFilterDialog::setConfigToSettings()
{
    QSettings settings("GeoImageViewer_dot_com", "GIV");
    settings.setValue("EditableFilter/CMakePath", _model->getCMakePath());
    settings.setValue("EditableFilter/PATH", _model->getPATH());
    settings.setValue("EditableFilter/CMakeGenerator", _model->getCMakeGenerator());
}

//******************************************************************************
}
