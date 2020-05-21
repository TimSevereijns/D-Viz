#include "Windows/distributionGraphModel.h"

ExtensionDistribution& DistributionGraphModel::GetDistribution(const std::wstring& extension)
{
    return m_map[extension];
}

void DistributionGraphModel::AddDatapoint(const std::wstring& extension, std::uintmax_t fileSize)
{
    m_map[extension].AddDatapoint(fileSize);
}

void DistributionGraphModel::BuildModel()
{
    for (auto& pair : m_map) {
        pair.second.AnalyzeDistribution();
    }
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
        m_insuffientData = true;
        return;
    }

    std::vector<std::uint32_t> bins(bucketCount, 0);
    for (const auto& fileSize : m_datapoints) {
        const auto index = static_cast<std::size_t>(std::floor(
            (fileSize - m_minimumX) / static_cast<long double>(m_maximumX - m_minimumX) *
            (bucketCount - 1)));

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
