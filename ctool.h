#ifndef CTOOL_H
#define CTOOL_H

#include <QString>
#include <QXmlStreamReader>

class CTool
{
public:
    static QString xmlNodeInfo(QXmlStreamReader &reader);

    static QString makeSuffix(QString szFile, QString szSuffix);
    static QString baseName(QString szFile);
};

// Purpose: Check if there is start element named n
//
// Usage:       if(IS_START_ELEM(reader, NOD_ARTIST))
// instead of:  if(reader->isStartElement()) && reader->name() == NOD_ARTIST)
//
#define IS_START_ELEM(r, n) ((*r).isStartElement()) && ((*r).name() == n)

#endif // CTOOL_H
