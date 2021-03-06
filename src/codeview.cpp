/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "codeview.h"
#include <QPainter>
#include <QDebug>
#include <QPaintEvent>
#include <QColor>
#include "log.h"
#include <assert.h>

#include "syntaxhighlighter.h"
#include "util.h"


static const int BORDER_WIDTH = 50;
 


CodeView::CodeView()
  : m_cfg(0)
 {
    m_font = QFont("Monospace", 8);
    m_fontInfo = new QFontMetrics(m_font);
    m_cursorY = 0;
}


CodeView::~CodeView()
{
    delete m_fontInfo;
}


void CodeView::setPlainText(QString text)
{
    text.replace("\r", "");

    m_highlighter.colorize(text);

//    m_rows = text.split("\n");

    setMinimumSize(4000,getRowHeight()*m_highlighter.getRowCount());

    update();
}


/**
 * @brief Returns the height of a text row in pixels.
 */
int CodeView::getRowHeight()
{
    int rowHeight = m_fontInfo->lineSpacing()+2;
    return rowHeight;
}


void CodeView::paintEvent ( QPaintEvent * event )
{
    int rowHeight = getRowHeight();
    QColor darkRed(100,0,0);
    QPainter painter(this);

    // Draw background
    painter.fillRect(event->rect(), Qt::black);



    // Border
    QRect rect = event->rect();
    rect.setRight(BORDER_WIDTH);
    QColor borderColor;
    borderColor = QColor(60,60,60);
    painter.fillRect(rect, borderColor);


    // Show breakpoints
    for(int j = 0;j < m_breakpointList.size();j++)
    {
        int lineNo = m_breakpointList[j];
        int rowIdx = lineNo-1;
        int y = rowHeight*rowIdx;
        QRect rect2(5,y,BORDER_WIDTH-10,rowHeight);
        painter.fillRect(rect2, Qt::blue);
    }

    
    // Draw content
    painter.setFont(m_font);
    for(size_t rowIdx = 0;rowIdx < m_highlighter.getRowCount();rowIdx++)
    {
        //int x = BORDER_WIDTH+10;
        int y = rowHeight*rowIdx;
        QString nrText;


    if((int)rowIdx == m_cursorY-1)
    {
        QRect rect2(BORDER_WIDTH,y,event->rect().width()-1,rowHeight);
        painter.fillRect(rect2, darkRed);
    }

        int fontY = y+(rowHeight-(m_fontInfo->ascent()+m_fontInfo->descent()))/2+m_fontInfo->ascent();
        painter.setPen(Qt::white);
        nrText = QString("%1").arg(rowIdx+1);
        painter.drawText(4, fontY, nrText);

        QVector<TextField*> cols = m_highlighter.getRow(rowIdx);
        
        int x = BORDER_WIDTH+10;
        for(int j = 0;j < cols.size();j++)
        {

            TextField *field = cols[j];            

            painter.setPen(field->m_color);
            painter.drawText(x, fontY, field->m_text);

            x += m_fontInfo->width(field->m_text);

        }
    }

}


void CodeView::disableCurrentLine()
{
    m_cursorY = -1;
    update();
}


/**
 * @brief Sets the current line.
 * @param lineNo   The line (1=first).
 */
void CodeView::setCurrentLine(int lineNo)
{
    m_cursorY = lineNo;
    update();
}


void CodeView::mouseReleaseEvent( QMouseEvent * event )
{
    Q_UNUSED(event);

}


void CodeView::mousePressEvent( QMouseEvent * event )
{
    Q_UNUSED(event);
    int j;
    
    if(event->button() == Qt::RightButton)
    {
        QPoint pos = event->globalPos();
            
        // Clicked on a text row?
        int rowHeight = getRowHeight();
        int rowIdx = event->pos().y() / rowHeight;
        int lineNo = rowIdx+1;
        if(rowIdx >= 0 && rowIdx < (int)m_highlighter.getRowCount())
        {
            // Get the words in the line
            QVector<TextField*> cols = m_highlighter.getRow(rowIdx);
            
            // Find the word under the cursor
            int x = BORDER_WIDTH+10;
            int foundPos = -1;
            for(j = 0;j < cols.size() && foundPos == -1;j++)
            {
                TextField *field = cols[j];            
                int w = m_fontInfo->width(field->m_text);
                if(x <= event->pos().x() && event->pos().x() <= x+w)
                {
                    foundPos = j;
                }
                x += w;
            }

            // Go to the left until a word is found
            if(foundPos != -1)
            {
                
                while(foundPos >= 0)
                {
                    if(cols[foundPos]->isSpaces() ||
                        m_highlighter.isKeyword(cols[foundPos]->m_text)
                        || m_highlighter.isSpecialChar(cols[foundPos]))
                    {
                        foundPos--;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            // Found anything under the cursor?
            QString incFile;
            QStringList list;
            if(foundPos != -1)
            {
                // Found a include file?
                if(cols[foundPos]->m_type == TextField::INC_STRING)
                {
                    incFile = cols[foundPos]->m_text.trimmed();
                    if(incFile.length() > 2)
                        incFile = incFile.mid(1, incFile.length()-2);
                    else
                        incFile = "";
                }
                 // or a variable?
                else if(cols[foundPos]->m_type == TextField::WORD)
                {
                    QStringList partList = cols[foundPos]->m_text.split('.');

                    // Remove the last word if it is a function
                    if(foundPos+1 < cols.size())
                    {
                        if(cols[foundPos+1]->m_text == "(" && partList.size() > 1)
                            partList.removeLast();
                    }
                    
                    
                    for(int partIdx = 1;partIdx <= partList.size();partIdx++)
                    {
                        QStringList subList = partList.mid(0, partIdx);
                        list += subList.join(".");
                    }

                    // A '[...]' section to the right of the variable?
                    if(foundPos+1 < cols.size())
                    {
                        if(cols[foundPos+1]->m_text == "[")
                        {
                            // Add the entire '[...]' section to the variable name
                            QString extraString = "[";
                            for(int j = foundPos+2;foundPos < cols.size() && cols[j]->m_text != "]";j++)
                            {
                                extraString += cols[j]->m_text;
                            }
                            extraString += ']';
                            list += partList.join(".") + extraString;

                        }
                    }

                }
            }
            
            // Inform the listener
            if(m_inf)
            {
                if(incFile.isEmpty())
                    m_inf->ICodeView_onContextMenu(pos, lineNo, list);
                else
                    m_inf->ICodeView_onContextMenuIncFile(pos,lineNo, incFile);
            
            }
        }
    }
}


void CodeView::mouseDoubleClickEvent( QMouseEvent * event )
{
    int rowHeight = m_fontInfo->lineSpacing()+2;

    if(event->x() < BORDER_WIDTH)
    {
        int lineNo = (event->y()/rowHeight)+1;

        if(m_inf)
            m_inf->ICodeView_onRowDoubleClick(lineNo);
    }
}


void CodeView::setBreakpoints(QVector<int> numList)
{
    m_breakpointList = numList;
    update();
}   

void CodeView::setConfig(Settings *cfg)
{
    m_cfg = cfg;

    assert(cfg != NULL);

    m_font = QFont(m_cfg->m_fontFamily, m_cfg->m_fontSize);
    delete m_fontInfo;
    m_fontInfo = new QFontMetrics(m_font);

    update();
}

    
