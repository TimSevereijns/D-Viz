#include "View/Dialogs/distributionGraphModel.h"

ExtensionDistribution& DistributionGraphModel::GetDistribution(const std::string& extension)
{
    return m_map[extension];
}

void DistributionGraphModel::AddDatapoint(const std::string& extension, std::uintmax_t fileSize)
{
    m_map[extension].AddDatapoint(fileSize);
}

void DistributionGraphModel::BuildModel()
{
    for (auto& pair : m_map) {
        pair.second.AnalyzeDistribution();
    }
}

void DistributionGraphModel::ClearData()
{
    m_map.clear();
}

void ExtensionDistribution::AddDatapoint(std::uintmax_t datum)
{
    m_datapoints.emplace_back(datum);
}

void ExtensionDistribution::AnalyzeDistribution()
{
    if (m_datapoints.empty()) {
        return;
    }

    const auto [min, max] = std::minmax_element(std::begin(m_datapoints), std::end(m_datapoints));
    m_minimumX = *min;
    m_maximumX = *max;

    if (m_maximumX - m_minimumX == 0) {
        m_buckets.clear();
        m_buckets.push_back(static_cast<std::uint32_t>(m_datapoints.size()));

        return;
    }

    std::vector<std::uint32_t> bins(defaultBucketCount, 0);
    for (const auto& fileSize : m_datapoints) {
        const auto index = static_cast<std::size_t>(std::floor(
            (fileSize - m_minimumX) / static_cast<long double>(m_maximumX - m_minimumX) *
            (defaultBucketCount - 1)));

        ++bins[index];
    }

    m_buckets = std::move(bins);
}

std::uintmax_t ExtensionDistribution::GetMaximumValueY() const
{
    const auto itr = std::max_element(std::begin(m_buckets), std::end(m_buckets));
    if (itr == std::end(m_buckets)) {
        return 0;
    }

    return *itr;
}

const std::vector<std::uint32_t>& ExtensionDistribution::GetBuckets() const
{
    return m_buckets;
}
