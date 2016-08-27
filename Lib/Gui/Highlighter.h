#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

// QT
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

// Project
#include "Core/LibExport.h"

class QTextDocument;

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;


    QTextCharFormat macrosFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat instanceFormat;
    QTextCharFormat namespaceFormat;

};

//******************************************************************************

}

#endif // HIGHLIGHTER_H
