#include "View/Dialogs/scanBreakdownModel.h"

#include <gsl/assert>

#include <iterator>

int ScanBreakdownModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(m_fileTypeMap.size());
}

int ScanBreakdownModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 5;
}

QVariant ScanBreakdownModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Orientation::Horizontal) {
        switch (section) {
            case 0:
                return QString{ "File Type" };
            case 1:
                return QString{ "Visible Size" };
            case 2:
                return QString{ "Total Size" };
            case 3:
                return QString{ "Visible Count" };
            case 4:
                return QString{ "Total Count" };
        }
    }

    return QString::number(section);
}

QString ScanBreakdownModel::FormatVisibleNodeSize(const RowModel& data) const
{
    const auto [size, units] = Utilities::ToPrefixedSize(data.visibleSize, m_prefix);
    return QString::fromStdString(fmt::format("{:03.2f} {}", size, units));
}

QString ScanBreakdownModel::FormatVisibleNodeCount(const RowModel& data) const
{
    return QString::fromStdString(fmt::format("{:L}", data.visibleCount));
}

QVariant ScanBreakdownModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != Qt::UserRole && role != Qt::DisplayRole) {
        return {};
    }

    const auto& data = m_fileTypeVector[static_cast<std::size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(data.fileExtension);
            case 1:
                return FormatVisibleNodeSize(data);
            case 2:
                return QString::fromStdString(data.formattedTotalSize);
            case 3:
                return FormatVisibleNodeCount(data);
            case 4:
                return QString::fromStdString(data.formattedTotalCount);
        }
    }

    if (role == Qt::UserRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdString(data.fileExtension);
            case 1:
                return static_cast<qulonglong>(data.visibleSize);
            case 2:
                return static_cast<qulonglong>(data.totalSize);
            case 3:
                return static_cast<qulonglong>(data.visibleCount);
            case 4:
                return static_cast<qulonglong>(data.totalCount);
        }
    }

    return {};
}

void ScanBreakdownModel::Insert(const Tree<VizBlock>::Node& node, bool isVisible)
{
    const auto& file = node->file;
    if (file.type != FileType::Regular) {
        return;
    }

    const auto key = file.extension.empty() ? std::string{ "No Extension" } : file.extension;
    auto& entry = m_fileTypeMap[key];

    entry.visibleSize += isVisible ? file.size : 0;
    entry.visibleCount += isVisible ? 1 : 0;
    entry.totalSize += file.size;
    entry.totalCount += 1;
}

void ScanBreakdownModel::ClearData()
{
    m_fileTypeMap.clear();
    m_fileTypeVector.clear();
}

void ScanBreakdownModel::BuildModel(Constants::SizePrefix sizePrefix)
{
    Expects(!m_fileTypeMap.empty() && m_fileTypeVector.empty());

    m_prefix = sizePrefix;

    std::transform(
        std::begin(m_fileTypeMap), std::end(m_fileTypeMap), std::back_inserter(m_fileTypeVector),
        [&](const auto& entry) {
            const auto& [extension, tally] = entry;
            return RowModel{ extension, tally, sizePrefix };
        });
}
