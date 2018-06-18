/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#include <QtTest>
#include <QDebug>

#include <common/utility.h>
#include <messages/messageobject.h>

using namespace OCC;

class TestMessageObject : public QObject
{
    Q_OBJECT

private slots:
    void testMessageObject()
    {
        QString testFile = QFINDTESTDATA("testmessageobject.json");
        MessageObject obj(testFile);
        QCOMPARE(obj.path, QFileInfo(testFile).absolutePath());
        QCOMPARE(obj.messageId, QUuid("{13e29128-5130-40ad-909c-e7f7c36ac69f}"));
        QCOMPARE(obj.status, MessageObject::SentStatus);

        QCOMPARE(obj.title, "Ich hab nur ne Frage");
        QCOMPARE(obj.initials, "Hans");
        QCOMPARE(obj.messageBody, "");
        QVERIFY(!obj.note.isEmpty());
        QCOMPARE(obj.recipient, "Arzt 1");

        QCOMPARE(obj.authoredOn, QDateTime(QDate(2018, 10, 17), QTime(13, 34, 15)));
        QCOMPARE(obj.sender, "Pflegeheim");

        // patient data
        QCOMPARE(obj.patientName, "");
        QCOMPARE(obj.birthday, QDate(2018, 10, 17));
        QCOMPARE(obj.gender, MessageObject::FEMALE);

        // vital data
        QCOMPARE(obj.bpSys, 0);
        QCOMPARE(obj.bpDia, 0);
        QCOMPARE(obj.pulse, 0);
        QCOMPARE(obj.temp, 0);
        QCOMPARE(obj.sugar, 0);
        QCOMPARE(obj.weight, 0);
        QCOMPARE(obj.misc, "");
        QCOMPARE(obj.pain, "");
        QCOMPARE(obj.response, "");
        QCOMPARE(obj.dtPulse, QDateTime(QDate(2018, 10, 17)));
        QCOMPARE(obj.dtBp, QDateTime(QDate(2018, 10, 17)));
        QCOMPARE(obj.dtSugar, QDateTime(QDate(2018, 10, 17)));
        QCOMPARE(obj.dtWeight, QDateTime(QDate(2018, 10, 17)));
        QCOMPARE(obj.dtTemp, QDateTime(QDate(2018, 10, 17)));
        QCOMPARE(obj.dtDefac, QDateTime(QDate(2018, 10, 17)));

        QCOMPARE(obj.imagesList.size(), 0);
        QCOMPARE(obj.medicationList.size(), 0);
    }
};


QTEST_APPLESS_MAIN(TestMessageObject)
#include "testmessageobject.moc"
