#include <QString>
#include <QtTest>

#include <memory>

#include <Visualizations/squarifiedTreemap.h>

class ModelTester : public QObject
{
   Q_OBJECT

public:

   ModelTester();

private Q_SLOTS:

   void testCase1();

private:

   std::experimental::filesystem::path m_path{ L"C:\\" };
   std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};

ModelTester::ModelTester() :
   m_model{ std::make_unique<SquarifiedTreeMap>(m_path) }
{
}

void ModelTester::testCase1()
{
   QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(ModelTester)

//#include "modelTester.moc"
