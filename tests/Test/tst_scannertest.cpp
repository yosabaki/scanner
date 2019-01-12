#include <QtTest>

// add necessary includes here

class scannerTest : public QObject
{
    Q_OBJECT

public:
    scannerTest();
    ~scannerTest();

private slots:
    void test_case1();

};

scannerTest::scannerTest()
{

}

scannerTest::~scannerTest()
{

}

void scannerTest::test_case1()
{

}

QTEST_APPLESS_MAIN(scannerTest)

#include "tst_scannertest.moc"
