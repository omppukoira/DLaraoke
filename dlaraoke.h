#ifndef QDLARAOKE_H
#define QDLARAOKE_H

#include <QtWidgets/QApplication>
#include <QSettings>

class DLaraoke : public QApplication
{
private:
    QSettings *m_Settings;
public:
    DLaraoke(int &argc, char **argv);

    static QSettings &settings()
    {
        static QSettings set;

        return set;
    }
};

#endif // QDLARAOKE_H
