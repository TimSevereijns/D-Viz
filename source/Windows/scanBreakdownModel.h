#ifndef SCANBREAKDOWNMODEL_H
#define SCANBREAKDOWNMODEL_H

#include <QAbstractTableModel>

#include "../constants.h"
#include "../controller.h"
#include "../DataStructs/vizBlock.h"
#include "../Utilities/utilities.hpp"

#include <Tree/Tree.hpp>

#include <mutex>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Settings
{
   class Manager;
}

struct RowModel
{
   std::wstring fileExtension;
   std::wstring formattedSize;
   std::uintmax_t totalSize;

   RowModel() = default;

   RowModel(
      std::wstring extension,
      std::wstring formattedSize,
      std::uintmax_t size)
      :
      fileExtension{ std::move(extension) },
      formattedSize{ std::move(formattedSize) },
      totalSize{ std::move(size) }
   {
   }
};

Q_DECLARE_METATYPE( RowModel );

class ScanBreakdownModel final : public QAbstractTableModel
{
   friend class BreakdownDialog;

   public:

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant headerData(
         int section,
         Qt::Orientation orientation,
         int role) const override;

      QVariant data(
         const QModelIndex& index,
         int role) const override;

      void insert(const Tree<VizBlock>::Node& node);

   private:

      void FinalizeInsertion(const Settings::Manager& settingsManager);

      void ClearData();

      std::vector<RowModel> m_fileTypeVector;

      std::unordered_map<std::wstring, std::uintmax_t> m_fileTypeMap;
};

#endif // SCANBREAKDOWNMODEL_H
