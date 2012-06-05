#ifndef CTOOL_H
#define CTOOL_H

#include <QString>
#include <QXmlStreamReader>

class CTool
{
public:
    static QString xmlNodeInfo(QXmlStreamReader &reader);

};

#endif // CTOOL_H
