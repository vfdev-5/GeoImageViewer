#ifndef BUILDERRORWIDGET_H
#define BUILDERRORWIDGET_H

// Qt
#include <QWidget>


// Project
#include "Core/LibExport.h"
#include "ui_BuildErrorWidget.h"

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT BuildErrorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BuildErrorWidget(QWidget *parent = 0);
    ~BuildErrorWidget();

    void clean();
    void appendText(const QString & text);

private:
    Ui_BuildErrorWidget *ui;
};

//******************************************************************************

}

#endif // BUILDERRORWIDGET_H
