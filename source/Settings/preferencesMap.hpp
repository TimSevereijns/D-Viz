#ifndef PREFERENCESMAP_H
#define PREFERENCESMAP_H

#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <QVector3D>

namespace Settings
{
   /**
    * @brief The Preferences Map class provides a neat little wrapper around an underlying map so
    * that value retrieval can be better handled.
    *
    * @note The class declaration is split from the function definitions so that the compiler sees
    * a full class declaration before encountering the template meta functions used to generate
    * better error messages via the static asserts seen later in the file.
    */
   class PreferencesMap
   {
      public:

         using Entry = std::variant<bool, int, float, std::wstring, QVector3D>;

         /**
          * @brief Inserts a new entry into the preferences map.
          *
          * @param[in] name         The name that the preference should be stored under.
          * @param[in] data         The PreferencesMap::Entry.
          */
         template<typename DataType>
         void Emplace(
            std::wstring name,
            DataType&& data);

         /**
          * @brief Extracts the value named by the query string if it exists.
          *
          * @param[in] query        The name of the desired preference.
          * @param[in] defaultValue The value to be returned if the desired entry doesn't exist.
          *
          * @returns The value if found, or the default value if no matching entry exists in the
          * map.
          */
         template<typename RequestedType>
         RequestedType GetValueOrDefault(
            std::wstring_view query,
            const RequestedType& defaultValue) const;

         /**
          * @overload
          */
         template<typename RequestedType>
         RequestedType GetValueOrDefault(
            std::wstring_view query,
            RequestedType&& defaultValue) &&;

      private:

         std::unordered_map<std::wstring, Entry> m_map;
   };
}

namespace
{
   /**
    * @brief Base case for the detection of preference map supported data types.
    */
   template<
      typename,
      typename = void
   >
   struct IsSupportedType : std::false_type
   {
   };

   /**
    * @brief Specialization to be selected if the data type is one that can be inserted into
    * the preference map.
    */
   template<typename Type>
   struct IsSupportedType<
      Type,
      std::void_t<decltype(Settings::PreferencesMap::Entry{ std::declval<Type&>() })>
   > : std::true_type
   {
   };
}

namespace Settings
{
   template<typename DataType>
   void PreferencesMap::Emplace(
      std::wstring name,
      DataType&& data)
   {
      static_assert(
         IsSupportedType<DataType>::value,
         "The preferences map doesn't support the insertion of the given type.");

      m_map.emplace(std::move(name), std::forward<decltype(data)>(data));
   }

   template<typename RequestedType>
   RequestedType PreferencesMap::GetValueOrDefault(
      std::wstring_view query,
      const RequestedType& defaultValue) const
   {
      static_assert(
         IsSupportedType<RequestedType>::value,
         "The preferences map doesn't support the retrieval of the given type.");

      const auto itr = m_map.find(query.data());
      if (itr == std::end(m_map))
      {
         return defaultValue;
      }

      const auto* encapsulatedValue = std::get_if<RequestedType>(&itr->second);
      return encapsulatedValue ? *encapsulatedValue : defaultValue;
   }

   template<typename RequestedType>
   RequestedType PreferencesMap::GetValueOrDefault(
      std::wstring_view query,
      RequestedType&& defaultValue) &&
   {
      static_assert(
         IsSupportedType<RequestedType>::value,
         "The preferences map doesn't support the retrieval of the given type.");

      const auto itr = m_map.find(query.data());
      if (itr == std::end(m_map))
      {
         return std::forward<decltype(defaultValue)>(defaultValue);
      }

      const auto* encapsulatedValue = std::get_if<RequestedType>(&itr->second);

      return encapsulatedValue
         ? *encapsulatedValue
         : std::forward<decltype(defaultValue)>(defaultValue);
   }
}

#endif // PREFERENCESMAP_H
