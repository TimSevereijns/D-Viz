#include <QString>
#include <QtTest>

#include "Visualizations/squarifiedTreemap.h"

#include <memory>

class VisualizationModelTests : public QObject
{
   Q_OBJECT

public:

   VisualizationModelTests();

private Q_SLOTS:

   void testCase1();

private:

   std::unique_ptr<VisualizationModel> m_model{ nullptr };
};

VisualizationModelTests::VisualizationModelTests() :
   m_model{ std::make_unique<SquarifiedTreeMap>(L"") }
{
}

void VisualizationModelTests::testCase1()
{
   QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(VisualizationModelTests)

#include "visualizationModel.moc"
