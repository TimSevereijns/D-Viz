#ifndef DISTRIBUTIONGRAPHMODEL_H
#define DISTRIBUTIONGRAPHMODEL_H

#include <algorithm>
#include <string>
#include <unordered_map>

class ExtensionDistribution
{
  public:
    void AddDatapoint(std::uintmax_t datum)
    {
        m_datapoints.emplace_back(datum);
    }

    void AnalyzeDistribution()
    {
        if (m_datapoints.empty()) {
            return;
        }

        const auto [min, max] =
            std::minmax_element(std::begin(m_datapoints), std::end(m_datapoints));

        m_minimum = *min;
        m_maximum = *max;

        if (m_maximum - m_minimum == 0) {
            m_insuffientData = true;
            return;
        }

        std::vector<std::uint32_t> bins(bucketCount, 0);
        for (const auto& fileSize : m_datapoints) {
            const auto index =
                std::floor((fileSize - m_minimum) / (m_maximum - m_minimum) * (bucketCount - 1));

            ++bins[index];
        }

        m_buckets = std::move(bins);
    }

    const std::vector<std::uint32_t>& GetBuckets() const
    {
        return m_buckets;
    }

  private:
    static constexpr auto bucketCount = 256;

    bool m_insuffientData{ false };

    long double m_minimum{ 0 };
    long double m_maximum{ 0 };
    std::vector<std::uintmax_t> m_datapoints;
    std::vector<std::uint32_t> m_buckets;
};

class DistributionGraphModel
{
  public:
    DistributionGraphModel();

    ExtensionDistribution& GetDistribution(const std::wstring& extension)
    {
        return m_map[extension];
    }

    void AddDatapoint(const std::wstring& extension, std::uintmax_t fileSize)
    {
        m_map[extension].AddDatapoint(fileSize);
    }

    void BuildModel()
    {
        for (auto& pair : m_map) {
            pair.second.AnalyzeDistribution();
        }
    }

  private:
    std::unordered_map<std::wstring, ExtensionDistribution> m_map;
};

#endif // DISTRIBUTIONGRAPHMODEL_H
