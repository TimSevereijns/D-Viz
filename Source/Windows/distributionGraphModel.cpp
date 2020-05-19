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

    m_minimum = static_cast<long double>(*min);
    m_maximum = static_cast<long double>(*max);

    if (m_maximum - m_minimum == 0) {
        m_insuffientData = true;
        return;
    }

    std::vector<std::uint32_t> bins(bucketCount, 0);
    for (const auto& fileSize : m_datapoints) {
        const auto index = static_cast<std::size_t>(
            std::floor((fileSize - m_minimum) / (m_maximum - m_minimum) * (bucketCount - 1)));

        ++bins[index];
    }

    m_buckets = std::move(bins);
}

const std::vector<std::uint32_t>& ExtensionDistribution::GetBuckets() const
{
    return m_buckets;
}
