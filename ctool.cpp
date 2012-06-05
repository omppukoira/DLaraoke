#include <QTextStream>

#include "ctool.h"

QString CTool::xmlNodeInfo(QXmlStreamReader &reader)
{
    QString szNodeInfo;
    QTextStream nodeInfo(&szNodeInfo);

    if(reader.hasError())
    {
        nodeInfo << qSetFieldWidth(5) << reader.lineNumber() << qSetFieldWidth(0) << ": ERROR [" << reader.errorString() << "]";
        return szNodeInfo;
    }

    if(reader.isStartDocument())
    {
        nodeInfo << "Start document";
    }
    else if(reader.isStartElement())
    {
        nodeInfo << qSetFieldWidth(5) << reader.lineNumber() << qSetFieldWidth(0) << ": <" << reader.name().toString() << ">";
    }
    else if(reader.isEndElement())
    {
        nodeInfo << qSetFieldWidth(5) << reader.lineNumber() << qSetFieldWidth(0) << ": </" << reader.name().toString() << ">";
    }
    else if(reader.isEndDocument())
    {
        nodeInfo << "End Document";
    }
    else if(reader.isWhitespace())
    {
        nodeInfo << "Whitespace";
    }
    else
    {
        nodeInfo << "Unkonwn state";
    }
    return szNodeInfo;
}
