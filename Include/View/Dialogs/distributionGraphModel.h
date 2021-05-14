#ifndef DISTRIBUTIONGRAPHMODEL_H
#define DISTRIBUTIONGRAPHMODEL_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class ExtensionDistribution
{
  public:
    void AddDatapoint(std::uintmax_t datum);

    void AnalyzeDistribution();

    const std::vector<std::uint32_t>& GetBuckets() const;

    std::uintmax_t GetMaximumValueX() const
    {
        return m_maximumX;
    }

    std::uintmax_t GetMinimumValueX() const
    {
        return m_minimumX;
    }

    std::uintmax_t GetMaximumValueY() const;

    std::size_t GetBucketCount() const
    {
        return m_buckets.size();
    }

  private:
    static constexpr std::size_t defaultBucketCount = 128;

    std::uintmax_t m_minimumX = 0;
    std::uintmax_t m_maximumX = 0;
    std::vector<std::uintmax_t> m_datapoints;
    std::vector<std::uint32_t> m_buckets;
};

class DistributionGraphModel
{
  public:
    ExtensionDistribution& GetDistribution(const std::string& extension);

    void AddDatapoint(const std::string& extension, std::uintmax_t fileSize);

    void BuildModel();

    void ClearData();

  private:
    std::unordered_map<std::string, ExtensionDistribution> m_map;
};

#endif // DISTRIBUTIONGRAPHMODEL_H
