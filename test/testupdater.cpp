/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#include <QtTest>

#include "updater/updater.h"
#include "updater/ocupdater.h"

using namespace OCC;

class TestUpdater : public QObject
{
    Q_OBJECT

private slots:
    void testVersionToInt()
    {
        qint64 lowVersion = Updater::Helper::versionToInt(2050,2,80);
        QCOMPARE(Updater::Helper::stringVersionToInt("2018.2-80"), lowVersion);

        qint64 highVersion = Updater::Helper::versionToInt(1999,2,80);
        qint64 currVersion = Updater::Helper::currentVersionToInt();
        QVERIFY(currVersion > 0);
        QVERIFY(currVersion > lowVersion);
        QVERIFY(currVersion < highVersion);
    }

};

QTEST_APPLESS_MAIN(TestUpdater)
#include "testupdater.moc"
