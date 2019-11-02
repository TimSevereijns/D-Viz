#ifndef MODELTESTS_H
#define MODELTESTS_H

#include <QObject>

#include <Scanner/driveScanner.h>

#include "testUtilities.hpp"

struct Sandbox
{
    Sandbox()
    {
        TestUtilities::UnzipTestData(
            "../../Tests/Data/boost-asio.zip", std::filesystem::absolute("../../Tests/Sandbox"));
    }

    ~Sandbox()
    {
        std::filesystem::remove_all("../../Tests/Sandbox");
    }
};

class Data : public QObject
{
    Q_OBJECT

  public:
    Data();

    DriveScanner scanner{};

    std::uintmax_t bytesScanned{ 0 };
    std::uintmax_t filesScanned{ 0 };
    std::uintmax_t directoriesScanned{ 0 };

    std::uint32_t progressCallbackInvocations{ 0 };

    std::shared_ptr<Tree<VizBlock>> tree{ nullptr };
    std::unique_ptr<SquarifiedTreeMap> model{ nullptr };

    std::vector<FileEvent> sampleNotifications;

    std::filesystem::path sampleDirectory{ std::filesystem::absolute("../../Tests/Sandbox/asio") };

  private:
    void ScanDrive();
};

#endif // MODELTESTS_H
