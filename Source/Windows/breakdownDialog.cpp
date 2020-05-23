#include "Windows/breakdownDialog.h"

#include "Windows/mainWindow.h"
#include "controller.h"

#include <Stopwatch/Stopwatch.hpp>
#include <spdlog/spdlog.h>

#include <QBarSet>
#include <QBrush>
#include <QGraphicsLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtCharts/QBarSeries>
#include <QtCharts/QLineSeries>

#include <functional>
#include <vector>

BreakdownDialog::BreakdownDialog(QWidget* parent)
    : QDialog{ parent }, m_mainWindow{ *(dynamic_cast<MainWindow*>(parent)) }, m_ui{}
{
    m_ui.setupUi(this);

    m_ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(
        m_ui.tableView, SIGNAL(customContextMenuRequested(QPoint)),
        SLOT(DisplayContextMenu(QPoint)));

    connect(
        m_ui.tableView, SIGNAL(doubleClicked(const QModelIndex&)),
        SLOT(HandleDoubleClick(const QModelIndex&)));

    ReloadData();
}

void BreakdownDialog::ReloadData()
{
    m_tableModel.ClearData();
    m_graphModel.ClearData();

    const auto& controller = m_mainWindow.GetController();
    const auto& tree = controller.GetTree();

    if (tree.GetRoot()->GetChildCount() == 0) {
        return;
    }

    const auto stopwatch = Stopwatch<std::chrono::milliseconds>([&] {
        // @todo Use a leaf iterator instead...
        for (const auto& node : tree) {
            if (node->file.type == FileType::Regular) {
                m_tableModel.Insert(node, controller.IsNodeVisible(node.GetData()));
            }

            m_graphModel.AddDatapoint(node->file.extension, node->file.size);
        }

        m_tableModel.BuildModel(controller.GetSessionSettings().GetActiveNumericPrefix());
        m_graphModel.BuildModel();
    });

    spdlog::get(Constants::Logging::DefaultLog)
        ->info(fmt::format(
            "Built break-down model in: {:n} {}", stopwatch.GetElapsedTime().count(),
            stopwatch.GetUnitsAsCharacterArray()));

    m_proxyModel.invalidate();
    m_proxyModel.setSourceModel(&m_tableModel);

    m_ui.tableView->setModel(&m_proxyModel);
    m_ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui.tableView->sortByColumn(1, Qt::SortOrder::DescendingOrder);
    m_ui.tableView->setSortingEnabled(true);
    m_ui.tableView->selectRow(0);

    AdjustColumnWidthsToFitViewport();

    const auto extensionVariant = m_proxyModel.index(0, 0).data(Qt::UserRole);
    const auto extension = extensionVariant.toString().toStdWString();
    GenerateGraph(extension);
}

void BreakdownDialog::AdjustColumnWidthsToFitViewport()
{
    const auto headerWidth = m_ui.tableView->verticalHeader()->width();

    const auto scrollbarWidth = m_ui.tableView->verticalScrollBar()->isVisible()
                                    ? m_ui.tableView->verticalScrollBar()->width()
                                    : 0;

    const auto tableWidth = m_ui.tableView->width() - headerWidth - scrollbarWidth;

    const auto columnCount = m_tableModel.columnCount(QModelIndex{});
    for (int index = 0; index < columnCount; ++index) {
        m_ui.tableView->setColumnWidth(index, tableWidth / columnCount);
    }
}

void BreakdownDialog::resizeEvent(QResizeEvent* /*event*/)
{
    AdjustColumnWidthsToFitViewport();
}

void BreakdownDialog::DisplayContextMenu(const QPoint& point)
{
    const auto unhighlightCallback = [&](auto& nodes) {
        m_mainWindow.GetCanvas().RestoreHighlightedNodes(nodes);
    };

    const auto highlightCallback = [&](auto& nodes) {
        m_mainWindow.GetCanvas().HighlightNodes(nodes);
    };

    const QModelIndex index = m_ui.tableView->indexAt(point);

    const auto extensionVariant = m_proxyModel.index(index.row(), 0).data(Qt::UserRole);
    const auto extension = extensionVariant.toString();

    QMenu menu;

    menu.addAction("Highlight All \"" + extension + "\" Files", [&] {
        auto& controller = m_mainWindow.GetController();

        controller.ClearHighlightedNodes(unhighlightCallback);
        controller.HighlightAllMatchingExtensions(extension.toStdWString(), highlightCallback);
    });

    const QPoint globalPoint = m_ui.tableView->viewport()->mapToGlobal(point);
    menu.exec(globalPoint);
}

void BreakdownDialog::HandleDoubleClick(const QModelIndex& index)
{
    const auto extensionVariant = m_proxyModel.index(index.row(), 0).data(Qt::UserRole);
    const auto extension = extensionVariant.toString().toStdWString();

    GenerateGraph(extension);
}

std::unique_ptr<QtCharts::QCategoryAxis>
BreakdownDialog::SetupAxisX(const ExtensionDistribution& distribution) const
{
    auto axisX = std::make_unique<QtCharts::QCategoryAxis>();
    axisX->setLabelsPosition(QtCharts::QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setTitleText("Size");

    const auto bucketCount = static_cast<int>(distribution.GetBucketCount());

    const auto tickCount = std::min(4, bucketCount);
    axisX->setTickCount(tickCount);

    const auto tickInterval = bucketCount / tickCount;
    axisX->setTickInterval(tickInterval);

    const auto prefix = m_mainWindow.GetController().GetSessionSettings().GetActiveNumericPrefix();
    const auto largestFile = distribution.GetMaximumValueX();

    for (int index = 1; index < tickCount + 1; ++index) {
        const auto value = index / static_cast<double>(tickCount) * largestFile;
        const auto [size, units] = Utilities::ToPrefixedSize(value, prefix);
        const auto label = QString::fromStdWString(fmt::format(L"{:.0f} {}", size, units));

        axisX->append(label, index * tickInterval);
    }

    return axisX;
}

std::unique_ptr<QtCharts::QLogValueAxis>
BreakdownDialog::SetupAxisY(const ExtensionDistribution& distribution) const
{
    auto axisY = std::make_unique<QtCharts::QLogValueAxis>();
    axisY->setRange(0, distribution.GetMaximumValueY());
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Count");

    return axisY;
}

void BreakdownDialog::GenerateGraph(const std::wstring& extension)
{
    const auto& distribution = m_graphModel.GetDistribution(extension);
    const auto& buckets = distribution.GetBuckets();

    auto set = std::make_unique<QtCharts::QBarSet>("Distribution");
    set->setColor(Qt::blue);

    for (std::size_t index = 0; index < buckets.size(); ++index) {
        // Since the log axis won't let us start at 0, we'll need to work around the fact that we'd
        // still like to visualize bars of height one. We'll cheat a little by adding half a unit.
        set->append(buckets[index] == 1 ? 1.5 : buckets[index]);
    }

    auto series = std::make_unique<QtCharts::QBarSeries>();
    series->append(set.release());
    series->setBarWidth(series->barWidth() * 2);

    auto chart = std::make_unique<QtCharts::QChart>();
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    chart->layout()->setContentsMargins(0, 0, 16, 0); //< Add space on the right for the last label.
    chart->legend()->hide();
    chart->setBackgroundRoundness(0);
    chart->addSeries(series.get());

    QFont font;
    font.setPixelSize(12);
    font.setBold(true);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush{ Qt::black });
    chart->setTitle("Size Distribution");

    auto axisX = SetupAxisX(distribution);
    chart->addAxis(axisX.get(), Qt::AlignBottom);
    series->attachAxis(axisX.get());

    auto axisY = SetupAxisY(distribution);
    chart->addAxis(axisY.get(), Qt::AlignLeft);
    series->attachAxis(axisY.get());

    // Since `QChart::addSeries(...)` takes ownership of the series resource, one would reasonably
    // assume that the `chart->addSeries(series.release())` call could be moved down after the last
    // use of the `series` pointer. Unfortunately, if one were to do that, and axis labels no longer
    // render. This seems like a bug in the Qt API design. As a work-around, we'll release here. The
    // other resources suffer from similar bizarre behavior.
    axisX.release();
    axisY.release();
    series.release();

    // Another quirk in the API stems from the fact that while the documentation claims "the
    // ownership of the new chart is passed to the chart view," one apparently still has to clean
    // up the previous chart.
    [[maybe_unused]] const auto previousChart =
        std::unique_ptr<QtCharts::QChart>(m_ui.graphView->chart());

    m_ui.graphView->setChart(chart.release());

    m_ui.graphView->setBackgroundBrush(Qt::white);
    m_ui.graphView->setRenderHint(QPainter::Antialiasing);
}
