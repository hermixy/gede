/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__VAR_CTL_H
#define FILE__VAR_CTL_H


#include <QString>
#include <QMap>
#include <QObject>


class VarCtl : public QObject
{
    Q_OBJECT

public:
    VarCtl(){};
    

    enum DispFormat
    {
        DISP_NATIVE,
        DISP_DEC,
        DISP_BIN,
        DISP_HEX,
        DISP_CHAR,
    };
    typedef struct
    {
        QString orgValue;
        DispFormat orgFormat;
        DispFormat dispFormat;
        bool isExpanded;
    }DispInfo;

    typedef QMap<QString, DispInfo>  DispInfoMap;


    static DispFormat findVarType(QString dataString);
    static QString valueDisplay(long long value, DispFormat format);

};

#endif // FILE__VAR_CTL_H
