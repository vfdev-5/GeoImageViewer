#ifndef EDITABLEFILTER_H
#define EDITABLEFILTER_H

// Qt
#include <QObject>
#include <QProcess>

// Project
#include "Filters/AbstractFilter.h"

class QLibrary;

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT EditableFilter : public AbstractFilter
{
    Q_OBJECT
    
    PROPERTY_ACCESSORS(QString, cmakeGenerator, getCMakeGenerator, setCMakeGenerator)

public:
    EditableFilter(QObject *parent = 0);

    QString getPATH() const;
    void setPATH(const QString & path);
	
    QString getCMakePath() const;
    void setCMakePath(const QString & path);

    void runTestCmake();
    void apply(const QString & program);
    QString readSourceFile();

    bool removeBuildCache();

signals:
    void badConfiguration();
    void workFinished(bool ok); //!< signal to notify that apply() method is done
    void buildError(QString);

protected slots:
    void onProcessStarted();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessStateChanged(QProcess::ProcessState newState);
    void onProcessReadyReadStandardError();
    void onProcessReadyReadStandardOutput();

protected:
    virtual cv::Mat filter(const cv::Mat & src) const;

private:
    void buildSourceFile();

    bool writeSourceFile(const QString & program);

    bool loadLibrary();
    bool unloadLibrary();

    void displayEnv() const;

    void processTask();

	QString _cmakePath;
    QString _sourceFilePath;
    QProcess * _process;
    QList<QStringList> _tasks;
    QLibrary * _libraryLoader;

    bool (EditableFilter::*_postExecuteFunc)();
    typedef bool (*LibFilterFunc)(uchar * idata, int iw, int ih, int itype,
                                   uchar ** odata, int * ow, int *oh, int *otype);
    LibFilterFunc _libFilterFunc;

};

//******************************************************************************

}

#endif // POWERFILTER_H
