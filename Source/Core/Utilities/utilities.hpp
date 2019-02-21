#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <experimental/filesystem>
#include <mutex>
#include <sstream>

#include "../DataStructs/vizBlock.h"

#include <Tree/Tree.hpp>

namespace Utilities
{
    namespace
    {
        std::once_flag stringStreamSetupFlag;
    }

    template <typename NumericType> static auto StringifyWithDigitSeparators(NumericType number)
    {
        static_assert(std::is_arithmetic_v<NumericType>, "Please pass in a numeric type.");

        static std::wstringstream stream;
        std::call_once(stringStreamSetupFlag, [&]() noexcept { stream.imbue(std::locale{ "" }); });

        stream.str(std::wstring{});
        stream.clear();

        stream << number;
        return stream.str();
    }

    inline static Tree<VizBlock>::Node* FindNodeUsingRelativePath(
        Tree<VizBlock>::Node* rootNode, const std::experimental::filesystem::path& path)
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

        if (filePathItr != std::end(path)) {
            return nullptr;
        }

        return node;
    }
} // namespace Utilities

#endif // UTILITIES_HPP
