#ifndef MULTITESTHARNESS_H
#define MULTITESTHARNESS_H

#include <QObject>
#include <QTest>

#include "Utilities/ignoreUnused.hpp"

#include <memory>
#include <unordered_map>

#include <gsl/gsl_assert>

namespace MultiTest
{
   namespace Detail
   {
      using TestMap = std::unordered_map<std::string, std::unique_ptr<QObject>>;

      inline TestMap& GetTestMap()
      {
         static TestMap instance;
         return instance;
      }
   }

   template<typename TestClassType>
   class TestRegistrar
   {
   public:

      TestRegistrar(std::string testName)
      {
         auto& testMap = Detail::GetTestMap();

         const auto itr = testMap.find(testName);
         if (itr != std::end(testMap))
         {
            Expects(!"Test already registered under that name."); // NOLINT
         }

         testMap.emplace(
            std::move(testName),
            std::make_unique<TestClassType>());
      }
   };

   inline int RunAllTests(int /*argc*/, char** /*argv*/)
   {
      auto result{ 0 };

      for (const auto& [name, test] : Detail::GetTestMap())
      {
         IgnoreUnused(name);
         result += QTest::qExec(test.get());
      }

      return result;
   }
}

#define NONEXPANDING_CONCAT(A, B) A ## B

#define CONCAT(A, B) NONEXPANDING_CONCAT(A, B)

#define REGISTER_TEST(classType) \
   MultiTest::TestRegistrar<classType> CONCAT(test, __LINE__){ #classType };

#endif // MULTITESTHARNESS_H
