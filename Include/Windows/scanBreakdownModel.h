#ifndef SCANBREAKDOWNMODEL_H
#define SCANBREAKDOWNMODEL_H

#include <QAbstractTableModel>

#include "Settings/persistentSettings.h"
#include "Utilities/utilities.hpp"
#include "Visualizations/vizBlock.h"
#include "constants.h"

#include <Tree/Tree.hpp>
#include <spdlog/spdlog.h>

#include <locale>
#include <mutex>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Settings
{
    class PersistentSettings;
}

struct ExtensionTally
{
    std::uintmax_t visibleCount{ 0 };
    std::uintmax_t totalCount{ 0 };
    std::uintmax_t visibleSize{ 0 };
    std::uintmax_t totalSize{ 0 };
};

class RowModel
{
  public:
    RowModel() = default;

    RowModel(std::wstring extension, const ExtensionTally& tally, Constants::SizePrefix prefix)
        : fileExtension{ std::move(extension) },
          visibleSize{ tally.visibleSize },
          totalSize{ tally.totalSize },
          visibleCount{ tally.visibleCount },
          totalCount{ tally.totalCount }
    {
        const auto [prefixedTotalSize, prefixTotalSizeUnits] =
            Utilities::ToPrefixedSize(totalSize, prefix);

        formattedTotalSize = fmt::format(L"{:03.2f} {}", prefixedTotalSize, prefixTotalSizeUnits);
        formattedTotalCount = fmt::format(L"{:n}", totalCount);
    }

    std::wstring fileExtension;
    std::wstring formattedTotalSize;
    std::wstring formattedTotalCount;

    std::uintmax_t visibleSize{ 0 };
    std::uintmax_t totalSize{ 0 };
    std::uintmax_t visibleCount{ 0 };
    std::uintmax_t totalCount{ 0 };
};

Q_DECLARE_METATYPE(RowModel)

class ScanBreakdownModel final : public QAbstractTableModel
{
    friend class BreakdownDialog;

  public:
    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    void Insert(const Tree<VizBlock>::Node& node, bool isVisible);

  private:
    QString FormatVisibleNodeSize(const RowModel& data) const;

    QString FormatVisibleNodeCount(const RowModel& data) const;

    void BuildModel(Constants::SizePrefix sizePrefix);

    void ClearData();

    std::vector<RowModel> m_fileTypeVector;

    std::unordered_map<std::wstring, ExtensionTally> m_fileTypeMap;

    Constants::SizePrefix m_prefix;
};

#endif // SCANBREAKDOWNMODEL_H
