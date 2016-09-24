#ifndef SCANBREAKDOWNENTRY_H
#define SCANBREAKDOWNENTRY_H

#include <QAbstractItemModel>

#include "../DataStructs/vizNode.h"
#include "../ThirdParty/Tree.hpp"

#include <vector>

class ScanBreakdownModel : public QAbstractItemModel
{
   friend class BreakdownDialog;

   public:
      ScanBreakdownModel();

      QModelIndex index(
         int row,
         int column,
         const QModelIndex &parent) const override;

      QModelIndex parent(const QModelIndex &child) const override;

      int rowCount(const QModelIndex &parent) const override;

      int columnCount(const QModelIndex &parent) const override;

      QVariant data(
         const QModelIndex &index,
         int role) const override;

      void insert(TreeNode<VizNode>* data);

   private:
      std::vector<TreeNode<VizNode>*> m_data;
};

#endif // SCANBREAKDOWNENTRY_H
