#ifndef TRANSFERFUNCTIONS_H
#define TRANSFERFUNCTIONS_H


// Qt
#include <qmath.h>
#include <QString>
#include <QStringList>
#include <QList>

// Project
#include "LibExport.h"

namespace Core
{

//*************************************************************************

QStringList GIV_DLL_EXPORT GetAvailableTransferFunctionNames();

//*************************************************************************

inline double idnt(double x)
{ return x; }

inline double pow2(double x)
{ return qPow(x, 2.0); }

inline double log(double x)
{ return qLn(1.0 + x); }

class TransferFunction
{
    typedef double (*Function)(double);

public:
    TransferFunction(const QString & name="Linear", Function f = &idnt) :
        _name(name),
        _f(f)
    {}

    inline double evaluate(double x) const
    {
        return (*_f)(x);
    }

    QString getName() const
    { return _name; }

protected:

    QString _name;
    Function _f;

};

//*************************************************************************

}

#endif // TRANSFERFUNCTIONS_H
