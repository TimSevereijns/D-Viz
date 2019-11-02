#include "modelTests.h"

#include <QSignalSpy>

#include <catch2/catch.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

#include <Scanner/Monitor/fileChangeNotification.hpp>
#include <Scanner/scanningParameters.h>
#include <Scanner/scanningProgress.hpp>
#include <Utilities/operatingSystemSpecific.hpp>
#include <Visualizations/squarifiedTreemap.h>
#include <constants.h>

#include "mockFileMonitor.h"
#include "testUtilities.hpp"

#include <QSignalSpy>

Data::Data()
{
    ScanDrive();
}

void Data::ScanDrive()
{
    const auto progressCallback = [&](const ScanningProgress& /*progress*/) {
        ++progressCallbackInvocations;
    };

    const auto completionCallback = [&](const ScanningProgress& progress,
                                        std::shared_ptr<Tree<VizBlock>> completedTree) {
        bytesScanned = progress.bytesProcessed.load();
        filesScanned = progress.filesScanned.load();
        directoriesScanned = progress.directoriesScanned.load();

        tree = std::move(completedTree);
    };

    // QSignalSpy completionSpy{ &scanner, &DriveScanner::Finished };
    // assert(completionSpy.isValid());

    scanner.StartScanning(
        ScanningParameters{ sampleDirectory, progressCallback, completionCallback });

    // completionSpy.wait(10'000);
}

namespace
{
    std::filesystem::path PathFromRootToNode(const Tree<VizBlock>::Node& node)
    {
        std::vector<std::reference_wrapper<const std::wstring>> reversePath;
        reversePath.reserve(Tree<VizBlock>::Depth(node));
        reversePath.emplace_back(node->file.name);

        const auto* currentNode = &node;
        while (currentNode->GetParent() && currentNode->GetParent()->GetParent()) {
            currentNode = currentNode->GetParent();
            reversePath.emplace_back(currentNode->GetData().file.name);
        }

        const auto pathFromRoot = std::accumulate(
            std::rbegin(reversePath), std::rend(reversePath), std::wstring{},
            [](const std::wstring& path, const std::wstring& file) {
                constexpr auto slash = L'/';

                if (!path.empty() && path.back() != slash) {
                    return path + slash + file;
                }

                return path + file;
            });

        auto finalPath = std::filesystem::path{ pathFromRoot };
        finalPath.make_preferred();

        return finalPath;
    }

    std::vector<FileEvent> SelectAllFiles(
        const typename Tree<VizBlock>::Node& rootNode, const std::wstring_view& fileExtension,
        FileEventType eventType)
    {
        std::vector<FileEvent> allEvents;

        std::for_each(
            Tree<VizBlock>::LeafIterator{ &rootNode }, Tree<VizBlock>::LeafIterator{},
            [&](const auto& node) {
                if (node->file.extension == fileExtension) {
                    const auto path = PathFromRootToNode(node);

                    allEvents.emplace_back(
                        FileEvent{ path.wstring() + node->file.extension, eventType });
                }
            });

        return allEvents;
    }

    Sandbox sandbox;
    Data data;
} // namespace

TEST_CASE("Data Model")
{
    const auto notificationGenerator = [&]() -> std::optional<FileEvent> {
        if (data.sampleNotifications.empty()) {
            return std::nullopt;
        }

        auto nextNotification = data.sampleNotifications.back();
        data.sampleNotifications.pop_back();

        return std::move(nextNotification);
    };

    data.model = std::make_unique<SquarifiedTreeMap>(
        std::make_unique<MockFileMonitor>(notificationGenerator), data.sampleDirectory);

    data.model->Parse(data.tree);

    SECTION("Scanning callbacks are invoked when progress is made")
    {
        REQUIRE(data.progressCallbackInvocations > 0); //< Scanning time determines exact count.
    }

    SECTION("Model contains the correct directory count")
    {
        const auto& tree = data.model->GetTree();
        REQUIRE(tree.Size() == 490ul);
    }

    SECTION("Verify file and directory counts")
    {
        // Counts as seen in Windows File Explorer:
        REQUIRE(data.bytesScanned == 3'407'665ul);
        REQUIRE(data.filesScanned == 469ul);
        REQUIRE(data.directoriesScanned == 20ul);
    }

    SECTION("Selecting nodes")
    {
        REQUIRE(data.model->GetSelectedNode() == nullptr);

        const Tree<VizBlock>::Node* sampleNode = data.tree->GetRoot();
        data.model->SelectNode(*sampleNode);
        REQUIRE(data.model->GetSelectedNode() == sampleNode);

        data.model->ClearSelectedNode();
        REQUIRE(data.model->GetSelectedNode() == nullptr);
    }

    SECTION("Highlighting Descendents")
    {
        REQUIRE(data.model->GetHighlightedNodes().size() == 0);

        auto visualizationParameters = Settings::VisualizationParameters{};
        visualizationParameters.rootDirectory = L"";
        visualizationParameters.minimumFileSize = 0u;
        visualizationParameters.onlyShowDirectories = false;

        const Tree<VizBlock>::Node* rootNode = data.tree->GetRoot();
        data.model->HighlightDescendants(*rootNode, visualizationParameters);

        const auto leafCount = std::count_if(
            Tree<VizBlock>::LeafIterator{ rootNode }, Tree<VizBlock>::LeafIterator{},
            [](const auto&) { return true; });

        REQUIRE(data.model->GetHighlightedNodes().size() == leafCount);
    }

    SECTION("Highlighting Ancestors")
    {
        REQUIRE(data.model->GetHighlightedNodes().size() == 0);

        const auto target = std::find_if(
            Tree<VizBlock>::LeafIterator{ data.tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
            [](const auto& node) {
                return (node->file.name + node->file.extension) == L"endpoint.ipp";
            });

        data.model->HighlightAncestors(*target);

        REQUIRE(data.model->GetHighlightedNodes().size() == 4);
    }

    SECTION("Highlight All Matching Extensions")
    {
        REQUIRE(data.model->GetHighlightedNodes().size() == 0);

        auto visualizationParameters = Settings::VisualizationParameters{};
        visualizationParameters.rootDirectory = L"";
        visualizationParameters.minimumFileSize = 0u;
        visualizationParameters.onlyShowDirectories = false;

        constexpr auto shouldSearchFiles{ true };
        constexpr auto shouldSearchDirectories{ false };

        data.model->HighlightMatchingFileName(
            L".hpp", visualizationParameters, shouldSearchFiles, shouldSearchDirectories);

        const auto headerCount = std::count_if(
            Tree<VizBlock>::PostOrderIterator{ data.tree->GetRoot() },
            Tree<VizBlock>::PostOrderIterator{},
            [](const auto& node) { return node->file.extension == L".hpp"; });

        REQUIRE(data.model->GetHighlightedNodes().size() == headerCount);
    }

    SECTION("Toggle File Monitoring")
    {
        data.sampleNotifications =
            std::vector<FileEvent>{ { "spawn.hpp", FileEventType::TOUCHED } };

        REQUIRE(data.model->IsFileSystemBeingMonitored() == false);
        data.model->StartMonitoringFileSystem();
        REQUIRE(data.model->IsFileSystemBeingMonitored() == true);
        data.model->StopMonitoringFileSystem();
        REQUIRE(data.model->IsFileSystemBeingMonitored() == false);
    }

    SECTION("Test Single Notification")
    {
        REQUIRE(data.tree != nullptr);

        std::filesystem::path absolutePathToRoot =
            data.model->GetTree().GetRoot()->GetData().file.name;
        std::filesystem::path targetFile = absolutePathToRoot / "spawn.hpp";

        data.sampleNotifications = std::vector<FileEvent>{ { targetFile, FileEventType::TOUCHED } };

        data.model->StartMonitoringFileSystem();
        data.model->WaitForNextModelChange();
        data.model->StopMonitoringFileSystem();

        const auto possibleNotification = data.model->FetchNextModelChange();
        REQUIRE(possibleNotification.has_value());

        REQUIRE(possibleNotification->path == targetFile);
        REQUIRE(possibleNotification->eventType == FileEventType::TOUCHED);
    }

    SECTION("Track Multiple Deletions")
    {
        REQUIRE(data.tree != nullptr);

        data.sampleNotifications =
            SelectAllFiles(*data.tree->GetRoot(), L".ipp", FileEventType::DELETED);

        data.model->StartMonitoringFileSystem();

        const auto totalNotifications = data.sampleNotifications.size();
        auto processedNotifications{ 0u };

        const auto startTime = std::chrono::high_resolution_clock::now();

        while (processedNotifications != totalNotifications) {
            std::optional<FileEvent> notification = data.model->FetchNextModelChange();

            if (notification) {
                ++processedNotifications;

                REQUIRE(notification->eventType == FileEventType::DELETED);
                REQUIRE(notification->path.extension() == L".ipp");
            }

            const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - startTime);

            if (elapsedTime > std::chrono::milliseconds(500)) {
                break;
            }
        }

        data.model->StopMonitoringFileSystem();

        REQUIRE(processedNotifications == totalNotifications);
    }

    SECTION("Apply File Deletions")
    {
        std::filesystem::path absolutePathToRoot =
            data.model->GetTree().GetRoot()->GetData().file.name;
        std::filesystem::path targetFile = absolutePathToRoot / "basic_socket.hpp";

        data.sampleNotifications =
            std::vector<FileEvent>{ { targetFile.string(), FileEventType::DELETED } };

        const auto foundTargetNode = std::any_of(
            Tree<VizBlock>::PostOrderIterator{ data.model->GetTree().GetRoot() },
            Tree<VizBlock>::PostOrderIterator{},
            [&](const auto& node) { return node->file.name == L"basic_socket"; });

        REQUIRE(foundTargetNode == true);

        data.model->StartMonitoringFileSystem();
        data.model->WaitForNextModelChange();
        data.model->RefreshTreemap();
        data.model->StopMonitoringFileSystem();

        const auto wasTargetNodeRemoved = std::none_of(
            Tree<VizBlock>::PostOrderIterator{ data.model->GetTree().GetRoot() },
            Tree<VizBlock>::PostOrderIterator{},
            [&](const auto& node) { return node->file.name == L"basic_socket"; });

        REQUIRE(wasTargetNodeRemoved == true);
    }

    SECTION("Apply File Creation")
    {
        std::filesystem::path absolutePathToRoot =
            data.model->GetTree().GetRoot()->GetData().file.name;

        std::filesystem::path targetFile = absolutePathToRoot / "fake_file.hpp";

        data.sampleNotifications = std::vector<FileEvent>{ { targetFile, FileEventType::CREATED } };

        const auto nodeDoesNotExist = std::none_of(
            Tree<VizBlock>::PostOrderIterator{ data.model->GetTree().GetRoot() },
            Tree<VizBlock>::PostOrderIterator{},
            [&](const auto& node) { return node->file.name == L"fake_file"; });

        REQUIRE(nodeDoesNotExist == true);

        data.model->StartMonitoringFileSystem();
        data.model->WaitForNextModelChange();
        data.model->RefreshTreemap();
        data.model->StopMonitoringFileSystem();

        const auto nodeWasAdded = std::any_of(
            Tree<VizBlock>::PostOrderIterator{ data.model->GetTree().GetRoot() },
            Tree<VizBlock>::PostOrderIterator{},
            [&](const auto& node) { return node->file.name == L"fake_file"; });

        REQUIRE(nodeWasAdded == true);
    }
}
