
// Qt
#include <QStack>
#include <QTime>
#include <QString>
#include <QPair>

// Project
#include "Global.h"

//******************************************************************************

#ifdef TIME_PROFILER_ON
QStack<QPair<QString, QTime*> > TIMERS;
void StartTimer(const QString & message)
{
    QTime * timer = new QTime;
    TIMERS.push(QPair<QString, QTime*>(message, timer));
    timer->start();
    SD_TRACE( message );
}

double StopTimer()
{
    if (TIMERS.size() < 1)
        return -1;

    QPair<QString, QTime*> timer = TIMERS.pop();
    QString message = timer.first;
    double tt = timer.second->elapsed();
    SD_TRACE( QString("Processing time (msec) : %1 = %2 ").arg(message).arg( tt ) );
    delete timer.second;
    return tt;
}
#endif

//******************************************************************************
