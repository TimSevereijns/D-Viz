#ifndef SCANBREAKDOWNENTRY_H
#define SCANBREAKDOWNENTRY_H

#include <QAbstractTableModel>

#include "../DataStructs/vizNode.h"
#include "../ThirdParty/Tree.hpp"

#include <unordered_map>
#include <vector>

class ScanBreakdownModel : public QAbstractTableModel
{
   friend class BreakdownDialog;

   public:

      int rowCount(const QModelIndex& parent) const final override;

      int columnCount(const QModelIndex& parent) const final override;

      QVariant headerData(
         int section,
         Qt::Orientation orientation,
         int role) const final override;

      QVariant data(
         const QModelIndex &index,
         int role) const final override;

      void insert(const TreeNode<VizNode>& node);

   private:

      std::vector<std::wstring> m_data;

      std::unordered_map<std::wstring, std::uintmax_t> m_fileTypeMap;
};

#endif // SCANBREAKDOWNENTRY_H
