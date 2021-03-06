/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef  FILE__SYNTAXHIGHLIGHTER
#define  FILE__SYNTAXHIGHLIGHTER

#include <QVector>
#include <QString>
#include <QColor>
#include <QHash>


struct TextField
{
    QColor m_color;
    QString m_text;
    enum {COMMENT, WORD, NUMBER, KEYWORD, CPP_KEYWORD, INC_STRING, STRING, SPACES} m_type;

    bool isHash() const { return m_text == "#" ? true : false; };
    bool isSpaces() const { return m_type == SPACES ? true : false; };
};


class SyntaxHighlighter
{
public:
    SyntaxHighlighter();
    virtual ~SyntaxHighlighter();
    
    void colorize(QString text);

    QVector<TextField*> getRow(unsigned int rowIdx);
    unsigned int getRowCount() { return m_rows.size(); };
    void reset();

    bool isCppKeyword(QString text) const;
    bool isKeyword(QString text) const;
    bool isSpecialChar(char c) const;
    bool isSpecialChar(TextField *field) const;

private:
    class Row
    {
    public:
        Row();

        TextField *getLastNonSpaceField();
        void appendField(TextField* field);
        
        bool isCppRow;
        QVector<TextField*>  m_fields;
    };
private:
    void pickColor(TextField *field);

private:
    QVector <Row*> m_rows;
    QHash <QString, bool> m_keywords;
    QHash <QString, bool> m_cppKeywords;
};

#endif // #ifndef FILE__SYNTAXHIGHLIGHTER
