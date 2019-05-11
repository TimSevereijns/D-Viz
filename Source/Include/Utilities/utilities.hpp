#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <filesystem>
#include <mutex>
#include <sstream>

#include "Visualizations/vizBlock.h"

#include <Tree/Tree.hpp>

namespace Utilities
{
    template <typename NumericType> static auto StringifyWithDigitSeparators(NumericType number)
    {
        static_assert(std::is_arithmetic_v<NumericType>, "Please pass in a numeric type.");

        std::wstringstream stream;
        stream.imbue(std::locale{ "" });

        stream.str(std::wstring{});
        stream.clear();

        stream << number;
        return stream.str();
    }

    inline static Tree<VizBlock>::Node*
    FindNodeUsingRelativePath(Tree<VizBlock>::Node* rootNode, const std::filesystem::path& path)
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
                    const auto pathElement = filePathItr->wstring();
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

    inline static Tree<VizBlock>::Node*
    FindNodeViaPath(Tree<VizBlock>::Node* rootNode, const std::filesystem::path& path)
    {
        assert(rootNode->GetData().file.type == FileType::DIRECTORY);

        std::filesystem::path rootPath = rootNode->GetData().file.name;
        const auto relativePath = std::filesystem::relative(path, rootPath);

        return FindNodeUsingRelativePath(rootNode, relativePath);
    }

    inline static Tree<VizBlock>::Node* FindClosestNodeUsingRelativePath(
        Tree<VizBlock>::Node* rootNode, const std::filesystem::path& path)
    {
        auto* node = rootNode;

        auto filePathItr = std::begin(path);
        while (filePathItr != std::end(path)) {
            auto matchingNodeItr = std::find_if(
                Tree<VizBlock>::SiblingIterator{ node->GetFirstChild() },
                Tree<VizBlock>::SiblingIterator{}, [&](const auto& childNode) {
                    const auto pathElement = filePathItr->wstring();
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

        return node;
    }
} // namespace Utilities

#endif // UTILITIES_HPP
