#include "scanBreakdownModel.h"

ScanBreakdownModel::ScanBreakdownModel()
{

}

QModelIndex ScanBreakdownModel::index(
   int row,
   int column,
   const QModelIndex& parent) const
{
   const auto index = createIndex(row, column, m_data[row]);
   return index;
}

QModelIndex ScanBreakdownModel::parent(const QModelIndex& child) const
{
   return { };
}

int ScanBreakdownModel::rowCount(const QModelIndex& parent) const
{
   return 0;
}

int ScanBreakdownModel::columnCount(const QModelIndex& parent) const
{
   return 0;
}

QVariant ScanBreakdownModel::data(
   const QModelIndex& index,
   int role) const
{
   return QString::fromStdWString(m_data[index.row()]->GetData().file.name);
}

void ScanBreakdownModel::insert(TreeNode<VizNode>* data)
{
   m_data.emplace_back(data);
}
