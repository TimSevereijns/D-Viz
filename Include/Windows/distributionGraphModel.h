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

  private:
    static constexpr auto bucketCount = 512;

    bool m_insuffientData{ false };

    long double m_minimum{ 0 };
    long double m_maximum{ 0 };
    std::vector<std::uintmax_t> m_datapoints;
    std::vector<std::uint32_t> m_buckets;
};

class DistributionGraphModel
{
  public:
    ExtensionDistribution& GetDistribution(const std::wstring& extension);

    void AddDatapoint(const std::wstring& extension, std::uintmax_t fileSize);

    void BuildModel();

  private:
    std::unordered_map<std::wstring, ExtensionDistribution> m_map;
};

#endif // DISTRIBUTIONGRAPHMODEL_H
