#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <filesystem>
#include <mutex>
#include <string_view>

#include "Model/vizBlock.h"
#include "constants.h"
#include "literals.h"

#include <Tree/Tree.hpp>
#include <gsl/assert>

namespace Utilities
{
    /**
     * @brief Locates the matching tree node that matches the given a relative filesystem path.
     *
     * @param[in] rootNode          The node at which to start the search.
     * @param[in] path              The filesystem path used to locate the node.
     */
    inline static Tree<VizBlock>::Node*
    FindNodeViaRelativePath(Tree<VizBlock>::Node* rootNode, const std::filesystem::path& path)
    {
        if (path == ".") {
            return rootNode;
        }

        auto* node = rootNode;

        auto filePathItr = std::begin(path);
        while (filePathItr != std::end(path)) {
            auto matchingNodeItr = std::find_if(
                Tree<VizBlock>::SiblingIterator{ node->GetFirstChild() },
                Tree<VizBlock>::SiblingIterator{}, [&](const auto& childNode) {
                    const auto pathElement = filePathItr->string();
                    const auto fileName = childNode->file.name + childNode->file.extension;

                    return fileName == pathElement;
                });

            if (matchingNodeItr != Tree<VizBlock>::SiblingIterator{}) {
                node = std::addressof(*matchingNodeItr);
                ++filePathItr;
            } else {
                break;
            }
        }

        if (filePathItr != std::end(path)) {
            return nullptr;
        }

        return node;
    }

    /**
     * @brief Locates the matching tree node that matches the given an absolute filesystem path.
     *
     * @param[in] rootNode          The node at which to start the search.
     * @param[in] path              The filesystem path used to locate the node.
     */
    inline static Tree<VizBlock>::Node*
    FindNodeViaAbsolutePath(Tree<VizBlock>::Node* rootNode, const std::filesystem::path& path)
    {
        std::filesystem::path rootPath = rootNode->GetData().file.name;
        const auto relativePath = std::filesystem::relative(path, rootPath);

        return FindNodeViaRelativePath(rootNode, relativePath);
    }

    /**
     * @brief Converts bytes to binary prefix size and notation.
     *
     * @param[in] sizeInBytes       The size in bytes to be converted.
     *
     * @returns A pair containing the numeric file size, and the associated units.
     */
    inline std::pair<double, std::string> ConvertToBinaryPrefix(double sizeInBytes)
    {
        using namespace Literals::Numeric::Binary;

        if (sizeInBytes < 1_KiB) {
            return std::make_pair(sizeInBytes, std::string{ " " } + Constants::Units::Bytes);
        }

        if (sizeInBytes < 1_MiB) {
            return std::make_pair(sizeInBytes / 1_KiB, " KiB");
        }

        if (sizeInBytes < 1_GiB) {
            return std::make_pair(sizeInBytes / 1_MiB, " MiB");
        }

        if (sizeInBytes < 1_TiB) {
            return std::make_pair(sizeInBytes / 1_GiB, " GiB");
        }

        return std::make_pair(sizeInBytes / 1_TiB, " TiB");
    }

    /**
     * @brief Converts bytes to decimal prefix size and notation.
     *
     * @param[in] sizeInBytes       The size in bytes to be converted.
     *
     * @returns A pair containing the numeric file size, and the associated units.
     */
    inline std::pair<double, std::string> ConvertToDecimalPrefix(double sizeInBytes)
    {
        using namespace Literals::Numeric::Decimal;

        if (sizeInBytes < 1_KB) {
            return std::make_pair(sizeInBytes, std::string{ " " } + Constants::Units::Bytes);
        }

        if (sizeInBytes < 1_MB) {
            return std::make_pair(sizeInBytes / 1_KB, " KB");
        }

        if (sizeInBytes < 1_GB) {
            return std::make_pair(sizeInBytes / 1_MB, " MB");
        }

        if (sizeInBytes < 1_TB) {
            return std::make_pair(sizeInBytes / 1_GB, " GB");
        }

        return std::make_pair(sizeInBytes / 1_TB, " TB");
    }

    /**
     * @brief Converts the given size of the file from bytes to the most human readable units.
     *
     * @param[in] sizeInBytes     The size (in bytes) to be converted to a more appropriate unit.
     * @param[in] prefix          The desired prefix.
     *
     * @returns A std::pair encapsulating the converted file size, and corresponding unit readout
     * string.
     */
    inline std::pair<double, std::string>
    ToPrefixedSize(std::uintmax_t sizeInBytes, Constants::SizePrefix prefix)
    {
        switch (prefix) {
            case Constants::SizePrefix::Binary: {
                return ConvertToBinaryPrefix(sizeInBytes);
            }
            case Constants::SizePrefix::Decimal: {
                return ConvertToDecimalPrefix(sizeInBytes);
            }
        }

        GSL_ASSUME(false);
    }
} // namespace Utilities

#endif // UTILITIES_HPP
