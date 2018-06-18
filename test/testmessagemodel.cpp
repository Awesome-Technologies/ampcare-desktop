/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#include <QtTest>

#include <common/utility.h>
#include <messages/messagemodel.h>

using namespace OCC;

class TestMessageModel : public QObject
{
    Q_OBJECT

    QTemporaryDir root;
    QString rootPath;
    MessageModel *model;

public:
    TestMessageModel()
        : model(nullptr)
    {
        QDir rootDir(root.path());
        rootPath = rootDir.canonicalPath();
        qInfo() << "creating test directory tree in " << rootPath;

        rootDir.mkpath("AMP/Arzt1/assets");
        rootDir.mkpath("AMP/Arzt1/messages");
        rootDir.mkpath("AMP/Arzt2/messages");
        rootDir.mkpath("AMP/Arzt3/messages");
        rootDir.mkpath("AMP/Arzt3/assets");
        rootDir.mkpath("forbidden");
        Utility::writeRandomFile(rootPath + "/AMP/random.bin");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt1/assets/someimage.jpg");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt1/messages/nomessage.txt");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt1/messages/somemessage.json");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt3/messages/somemessage.json");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt1/messages/someothermessage.json");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt1/messages/somethirdmessage.json");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt2/messages/somemessage.json");
        Utility::writeRandomFile(rootPath + "/AMP/Arzt2/messages/anothermessage.json");
        Utility::writeRandomFile(rootPath + "/forbidden/secret.json");
    }

private slots:
    void init()
    {
        model = new MessageModel(rootPath + "/AMP", "myself");
    }

    void cleanup()
    {
        delete model;
        model = nullptr;
    }

    void testMessageModel()
    {
        QCOMPARE(model->rowCount(), 6);
        QCOMPARE(model->columnCount(), 4);
        QAbstractItemModelTester tester(model);
    }
};

QTEST_MAIN(TestMessageModel)
#include "testmessagemodel.moc"
