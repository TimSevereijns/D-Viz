#include "Windows/scanBreakdownModel.h"

#include <gsl/gsl_assert>

#include <iterator>

int ScanBreakdownModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(m_fileTypeMap.size());
}

int ScanBreakdownModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
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
                return QString{ "Cumulative Size" };
            case 2:
                return QString{ "Item Count" };
        }
    }

    return QString::number(section);
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
                return QString::fromStdWString(data.fileExtension);
            case 1:
                return QString::fromStdWString(data.formattedSize);
            case 2:
                return QString::fromStdWString(data.formattedCount);
        }
    }

    if (role == Qt::UserRole) {
        switch (index.column()) {
            case 0:
                return QString::fromStdWString(data.fileExtension);
            case 1:
                return static_cast<qulonglong>(data.totalSize);
            case 2:
                return static_cast<qulonglong>(data.itemCount);
        }
    }

    return {};
}

void ScanBreakdownModel::Insert(const Tree<VizBlock>::Node& node)
{
    const auto& file = node->file;
    if (file.type != FileType::Regular) {
        return;
    }

    if (file.extension.empty()) {
        auto& fileType = m_fileTypeMap[L"No Extension"];
        fileType.totalSize += file.size;
        fileType.count += 1;
    } else {
        auto& fileType = m_fileTypeMap[file.extension];
        fileType.totalSize += file.size;
        fileType.count += 1;
    }
}

void ScanBreakdownModel::ClearData()
{
    m_fileTypeMap.clear();
    m_fileTypeVector.clear();
}

void ScanBreakdownModel::Process(Constants::SizePrefix sizePrefix)
{
    Expects(!m_fileTypeMap.empty() && m_fileTypeVector.empty());

    std::transform(
        std::begin(m_fileTypeMap), std::end(m_fileTypeMap), std::back_inserter(m_fileTypeVector),
        [&](const auto& entry) {
            const auto& extension = entry.first;
            const auto& itemCount = entry.second.count;
            const auto& totalSize = entry.second.totalSize;

            return RowModel{ extension, sizePrefix, totalSize, itemCount };
        });
}
