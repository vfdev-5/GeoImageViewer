
// Project
#include "TransferFunctions.h"


namespace Core
{

//*************************************************************************

/*!
    \class TransferFunction
    \ingroup TransferFunctions
    \brief This is an class of transfer functions.
 */

//*************************************************************************

QStringList GetAvailableTransferFunctionNames()
{
    return QStringList()
            << "Linear"
            << "Square"
            << "Log";
}

//*************************************************************************

}



