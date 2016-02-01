
// Qt
#include <qglobal.h>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QSysInfo>
#include <QLibrary>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "EditableFilter.h"

namespace Filters
{

//******************************************************************************
/*!
  \class EditableFilter
  \brief Idea is to compile a code in runtime and execute
*/
//******************************************************************************

EditableFilter::EditableFilter(QObject *parent) :
    AbstractFilter(parent),
    _process(new QProcess(this)),
    _sourceFilePath("Resources/EditableFunction/EditableFunction.cpp"),
    _cmakePath("cmake"),
    _postExecuteFunc(0),
    _libFilterFunc(0),
    _libraryLoader(new QLibrary(this))
{
    _name = tr("Editable filter");
    _description = tr("Apply a custom code");

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    _process->setProcessEnvironment(env);

    // Configure process:
    connect(_process, &QProcess::started, this, &EditableFilter::onProcessStarted);
    connect(_process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &EditableFilter::onProcessError);
    connect(_process, static_cast<void(QProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&QProcess::finished), this, &EditableFilter::onProcessFinished);
    connect(_process, &QProcess::stateChanged, this, &EditableFilter::onProcessStateChanged);
    connect(_process, &QProcess::readyReadStandardError, this, &EditableFilter::onProcessReadyReadStandardError);
    connect(_process, &QProcess::readyReadStandardOutput, this, &EditableFilter::onProcessReadyReadStandardOutput);

}

//******************************************************************************

QString EditableFilter::getCMakePath() const
{
    QString out(_cmakePath);
    return out.replace(QString("\\\\"), QString("\\"));
}

//******************************************************************************

void EditableFilter::setCMakePath(const QString &path)
{
    _cmakePath = path;
    _cmakePath.replace(QString("\\"), QString("\\\\"));
}

//******************************************************************************

cv::Mat EditableFilter::filter(const cv::Mat &src) const
{
    int ow(0), oh(0), otype(0);
    uchar * odata = 0;

    if (!_libFilterFunc(
                src.data, src.cols, src.rows, src.type(),
                _noDataValue,
                &odata, &ow, &oh, &otype))
    {
        SD_TRACE("EditableFilter : filter function is failed");
    }
    cv::Mat out(oh, ow, otype, odata);
    return out;
}

//******************************************************************************

QString EditableFilter::getPATH() const
{
    QProcessEnvironment env = _process->processEnvironment();
    QStringList keys;
    keys << "PATH";
    foreach (QString key, keys)
    {
        if (env.contains(key)) return env.value(key).replace("\\\\", "\\");
    }
    return QString();
}

//******************************************************************************

void EditableFilter::setPATH(const QString &path)
{
    QString p(path);
    p.replace(QString("\\"), QString("\\\\"));
    QProcessEnvironment env = _process->processEnvironment();
    QStringList keys;
    keys << "PATH";
    foreach (QString key, keys)
    {
        env.insert(key, p);
    }
    _process->setProcessEnvironment(env);
}

//******************************************************************************

void EditableFilter::onProcessStarted()
{
    SD_TRACE("Process started");
}

//******************************************************************************

void EditableFilter::onProcessError(QProcess::ProcessError error)
{
    switch (error)
    {
    case QProcess::FailedToStart:
        SD_TRACE("Process error : Failed to start");
        // -> program is not found => configure
        SD_TRACE("Clear all tasks");
        _tasks.clear();
        SD_ERR(tr("CMake executable is not found. Please, configure CMake path"));
        emit badConfiguration();
        break;
    case QProcess::Crashed:
        SD_TRACE("Process error : Crashed");
        break;
    case QProcess::Timedout:
        SD_TRACE("Process error : Timedout");
        break;
    case QProcess::ReadError:
        SD_TRACE("Process error : ReadError");
        break;
    case QProcess::WriteError:
        SD_TRACE("Process error : WriteError");
        break;
    default:
        SD_TRACE("Process error : Unknown");
    }

}

//******************************************************************************

void EditableFilter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    SD_TRACE1("Process finished : exitCode=%1", exitCode);
    if (exitStatus == QProcess::NormalExit)
    {
        SD_TRACE("Process finished with status = NormalExit");

        // Under Linux if exitCode == 0 -> everything is OK
        if (exitCode == 0)
        {
            if (!_tasks.isEmpty())
            {
                processTask();
            }
            else{
                // all tasks are done
                if (_postExecuteFunc)
                {
                    if (!(this->*_postExecuteFunc)())
                    {
                        SD_TRACE("Post execute function is failed");
                        emit workFinished(false);
                    }
                    else
                    {
                        emit workFinished(true);
                    }
                }
            }
            return;
        }
    }
    else if (exitStatus == QProcess::CrashExit)
    {
        SD_TRACE("Process finished with status = CrashExit");
    }
    emit workFinished(false);
    SD_TRACE("Clear all tasks");
    _tasks.clear();
}

//******************************************************************************

void EditableFilter::onProcessStateChanged(QProcess::ProcessState newState)
{
    switch (newState)
    {
    case QProcess::NotRunning:
        SD_TRACE("Process state changed : NotRunning");
        break;
    case QProcess::Starting:
        SD_TRACE("Process state changed : Starting");
        break;
    case QProcess::Running:
        SD_TRACE("Process state changed : Running");
        break;
    }
}

//******************************************************************************

void EditableFilter::onProcessReadyReadStandardError()
{
    SD_TRACE("Process ready to read std error");

    QString err(_process->readAllStandardError());
    SD_TRACE1("Errors : \n %1", err);

    emit buildError(err);

}

//******************************************************************************

void EditableFilter::onProcessReadyReadStandardOutput()
{
    SD_TRACE("Process ready to read std output");

    QString output(_process->readAllStandardOutput());
    SD_TRACE1("Output : \n %1", output);

    if (output.contains("error", Qt::CaseInsensitive))
    {
#if (defined WIN32 || defined _WIN32 || defined WINCE)
        if (!output.contains(
                    QRegExp("(/errorReport:queue|%errorlevel%|:cmErrorLevel)",
                            Qt::CaseInsensitive)))
        {
            emit buildError(output);
        }
#else
        emit buildError(output);
#endif
    }

}

//******************************************************************************

void EditableFilter::runTestCmake()
{
    _tasks.append(QStringList() << _cmakePath << "--version");
	SD_TRACE1("Append taks : %1 --version", _cmakePath);
    processTask();
}

//******************************************************************************

void EditableFilter::apply(const QString &program)
{
    if (!writeSourceFile(program))
    {
        return;
    }

    _postExecuteFunc = &EditableFilter::loadLibrary;

    buildSourceFile();
}

//******************************************************************************

void EditableFilter::buildSourceFile()
{
    SD_TRACE("Build source file");

    if (!unloadLibrary()) return;

    QDir d("Resources/Build");
    if (!d.exists())
    {
        d.setPath("Resources");
        if (!d.mkdir("Build"))
        {
            SD_ERR("Failed to create 'Resources/Build' folder");
            return;
        }
        d.setPath("Resources/Build");
    }

    _process->setWorkingDirectory(d.absolutePath());
    d.setPath("Resources/EditableFunction");

    // Configure
    QStringList task;
    task << _cmakePath
         << "-DCMAKE_BUILD_TYPE=Release"
         << "-DCMAKE_INSTALL_PREFIX=../../";
#if (defined WIN32 || defined _WIN32 || defined WINCE)
        // Force generator choice
        task << "-G" + _cmakeGenerator;
#endif
    task << d.absolutePath();
    _tasks.append(task);
	SD_TRACE3("Append task : %1 %2 %3", task[0] + " " + task[1], task[2] + " " + task[3], task[4]);


    // Build
    SD_TRACE("2) Start process : cmake --build");
    _tasks.append(QStringList() << _cmakePath
                  << "--build"
                  << "."
                  << "--target"
                  << "install"
                  << "--config"
                  << "Release");
    task = _tasks.last();
    SD_TRACE3("Append task : %1 %2 %3", task[0] + " " + task[1], task[2], task[3]);
    processTask();
}

//******************************************************************************

QString EditableFilter::readSourceFile()
{
    QFile f(_sourceFilePath);

    if (!f.open(QIODevice::ReadOnly))
    {
        SD_TRACE("Failed to read source file");
        return "";
    }

    QTextStream ts(&f);
    QString program = ts.readAll();

    f.close();
    return program;
}

////******************************************************************************

//double EditableFilter::computeResult(double v)
//{
//    if (_libFunc)
//    {
//        return (*_libFunc)(v);
//    }
//    return -999999999999;
//}

//******************************************************************************

bool EditableFilter::removeBuildCache()
{
    QDir d("Resources/Build");
    if (!d.exists())
    {
        return true;
    }

    if (!d.removeRecursively())
    {
        SD_TRACE("Failed to remove 'Resources/Build' folder");
        return false;
    }
	_process->setWorkingDirectory("");
    return true;
}

//******************************************************************************

bool EditableFilter::writeSourceFile(const QString & program)
{

    QFile f(_sourceFilePath);

    if (!f.open(QIODevice::WriteOnly))
    {
        SD_TRACE("Failed to write source file");
        return false;
    }

    QTextStream ts(&f);
    ts << program;

    f.close();
    return true;
}

//******************************************************************************

bool EditableFilter::loadLibrary()
{
    SD_TRACE("Load library");

    if (!unloadLibrary()) return false;

    QStringList names;
    names << "EditableFunction";

    QDir d(".");
    SD_TRACE1("Working path : %1", d.absolutePath());
    foreach (QString name, names)
    {
        _libraryLoader->setFileName(d.absolutePath() + "/" + name);
        if (_libraryLoader->load())
        {
//            _libFunc = reinterpret_cast<double(*)(double)>(_libraryLoader->resolve("foo"));
            _libFilterFunc = reinterpret_cast<LibFilterFunc>(_libraryLoader->resolve("filterFunc"));
            if (!_libFilterFunc)
            {
                SD_TRACE1("Lib filter function is null : %1", _libraryLoader->errorString());
            }
            else
            {
                break;
            }
        }
        else
        {
            SD_TRACE1("Library is not loaded : %1", _libraryLoader->errorString());
            SD_ERR(tr("Editable filter library was not loaded. Probably, it the problem of build configuration, e.g x86 / x64 problem"));
        }
    }

    return _libFilterFunc != 0;
}

//******************************************************************************

bool EditableFilter::unloadLibrary()
{
    if (_libraryLoader->isLoaded())
    {
        if (!_libraryLoader->unload())
        {
            SD_TRACE1("Failed to unload library : %1", _libraryLoader->errorString());
            return false;
        }
    }
    return true;
}

//******************************************************************************

void EditableFilter::displayEnv() const
{
    SD_TRACE("Process env : ");
    foreach(QString v, _process->processEnvironment().toStringList())
    {
        SD_TRACE(v);
    }
}

//******************************************************************************

QString StringListToString(const QStringList & list)
{
    QString out;
    foreach (QString i, list)
    {
        out.append(i + " ");
    }
    return out;
}
void EditableFilter::processTask()
{
    QStringList task = _tasks.takeFirst();
	SD_TRACE2("Process task: %1 | taskCount=%2", StringListToString(task), _tasks.size());
    _process->start(task.takeFirst(), task);
}

//******************************************************************************

}

