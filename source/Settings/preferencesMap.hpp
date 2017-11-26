#ifndef PREFERENCESMAP_H
#define PREFERENCESMAP_H

#include <string>
#include <string_view>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace Settings
{
   class PreferencesMap;
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
    * @brief Specialization to be selected if the data type is the one that can be inserted into
    * the preference map.
    */
   template<typename Type>
   struct IsSupportedType<
      Type,
      std::void_t<decltype(Settings::PreferencesMap::Entry{ std::declval<std::decay_t<Type>&>() })>
   > : std::true_type
   {
   };
}

namespace Settings
{
   class PreferencesMap
   {
      public:

         using Entry = std::variant<bool, int, float, std::wstring>;

         /**
          * @brief Inserts a new entry into the preferences map.
          *
          * @param[in] name         The name that the preference should be stored under.
          * @param[in] data         The PreferencesMap::Entry.
          */
         template<typename DataType>
         void Emplace(
            std::wstring name,
            DataType&& data)
         {
            static_assert(
               IsSupportedType<DataType>::value,
               "The preferences map doesn't support the insertion of the given type.");

            m_map.emplace(std::move(name), std::forward<DataType>(data));
         }

         /**
          * @brief Extracts the value named by the query string if it exists.
          *
          * @param[in] query        The name of the desired preference.
          * @param[in] defaultValue The value to be returned if the desired entry doesn't exist.
          *
          * @returns The value if found, or the default value if nothing was found.
          */
         template<typename ReturnType>
         ReturnType GetValueOrDefault(
            std::wstring_view query,
            ReturnType&& defaultValue) const
         {
            static_assert(
               IsSupportedType<ReturnType>::value,
               "The preferences map doesn't support the retrieval of the given type.");

            const auto itr = m_map.find(query.data());
            if (itr == std::end(m_map))
            {
               return defaultValue;
            }

            const auto* encapsulatedValue = std::get_if<ReturnType>(&itr->second);
            return encapsulatedValue ? *encapsulatedValue : defaultValue;
         }

      private:

         std::unordered_map<std::wstring, Entry> m_map;
   };
}

#endif // PREFERENCESMAP_H
