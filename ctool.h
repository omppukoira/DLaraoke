#ifndef CTOOL_H
#define CTOOL_H

#include <QString>
#include <QXmlStreamReader>

class CTool
{
public:
    static QString xmlNodeInfo(QXmlStreamReader &reader);

    static QString makeSuffix(QString szFile, QString szSuffix);
};

#endif // CTOOL_H
