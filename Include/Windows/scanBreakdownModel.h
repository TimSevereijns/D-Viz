#ifndef SCANBREAKDOWNMODEL_H
#define SCANBREAKDOWNMODEL_H

#include <QAbstractTableModel>

#include "Settings/settingsManager.h"
#include "Utilities/utilities.hpp"
#include "Visualizations/vizBlock.h"
#include "constants.h"

#include <Tree/Tree.hpp>
#include <spdlog/spdlog.h>

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
    std::wstring formattedCount;

    std::uintmax_t totalSize{ 0 };
    std::uintmax_t itemCount{ 0 };

    RowModel() = default;

    RowModel(
        std::wstring extension, Constants::FileSize::Prefix prefix, std::uintmax_t size,
        std::uintmax_t count)
        : fileExtension{ std::move(extension) }, totalSize{ size }, itemCount{ count }
    {
        const auto [prefixedSize, prefixUnits] =
            Utilities::ConvertFileSizeToNumericPrefix(totalSize, prefix);

        formattedSize = fmt::format(L"{:03.2f} {}", prefixedSize, prefixUnits).c_str();
        formattedCount = Utilities::ToStringWithNumericGrouping(itemCount);
    }
};

Q_DECLARE_METATYPE(RowModel)

struct ExtensionCountAndSize
{
    std::uintmax_t count{ 0 };
    std::uintmax_t totalSize{ 0 };
};

class ScanBreakdownModel final : public QAbstractTableModel
{
    friend class BreakdownDialog;

  public:
    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    void insert(const Tree<VizBlock>::Node& node);

  private:
    void Process(Constants::FileSize::Prefix sizePrefix);

    void ClearData();

    std::vector<RowModel> m_fileTypeVector;

    std::unordered_map<std::wstring, ExtensionCountAndSize> m_fileTypeMap;
};

#endif // SCANBREAKDOWNMODEL_H
