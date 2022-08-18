#include "View/Dialogs/breakdownDialog.h"

#include "View/mainWindow.h"
#include "controller.h"

#include <spdlog/spdlog.h>
#include <stopwatch.h>

#include <QBarSet>
#include <QBrush>
#include <QGraphicsLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtCharts/QBarSeries>

#include <functional>
#include <vector>

BreakdownDialog::BreakdownDialog(QWidget* parent)
    : QDialog{ parent }, m_mainWindow{ *(dynamic_cast<MainWindow*>(parent)) }
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
    const auto stopwatch = Stopwatch<std::chrono::milliseconds>([&] { BuildModel(); });

    const auto& log = spdlog::get(Constants::Logging::DefaultLog);
    log->info(
        "Built break-down model in: {:L} {}", stopwatch.GetElapsedTime().count(),
        stopwatch.GetUnitsAsString());

    m_proxyModel.invalidate();
    m_proxyModel.setSourceModel(&m_tableModel);

    m_ui.tableView->setModel(&m_proxyModel);
    m_ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui.tableView->sortByColumn(1, Qt::SortOrder::DescendingOrder);
    m_ui.tableView->setSortingEnabled(true);
    m_ui.tableView->selectRow(0);

    AdjustColumnWidthsToFitViewport();

    const auto extensionVariant = m_proxyModel.index(0, 0).data(Qt::UserRole);
    const auto extension = extensionVariant.toString().toStdString();
    GenerateGraph(extension);
}

void BreakdownDialog::BuildModel()
{
    m_tableModel.ClearData();
    m_graphModel.ClearData();

    const auto& controller = m_mainWindow.GetController();
    const auto& tree = controller.GetTree();

    if (tree.GetRoot()->GetChildCount() == 0) {
        return;
    }

    const auto& options = controller.GetSessionSettings().GetVisualizationOptions();

    std::for_each(
        Tree<VizBlock>::LeafIterator{ tree.GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if (node->file.type == FileType::Regular) {
                m_tableModel.Insert(node, options.IsNodeVisible(node.GetData()));

                if (node->file.extension.empty()) {
                    m_graphModel.AddDatapoint("No Extension", node->file.size);
                } else {
                    m_graphModel.AddDatapoint(node->file.extension, node->file.size);
                }
            }
        });

    m_tableModel.BuildModel(controller.GetSessionSettings().GetActiveNumericPrefix());
    m_graphModel.BuildModel();
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
        controller.HighlightAllMatchingExtensions(extension.toStdString(), highlightCallback);
    });

    const QPoint globalPoint = m_ui.tableView->viewport()->mapToGlobal(point);
    menu.exec(globalPoint);
}

void BreakdownDialog::HandleDoubleClick(const QModelIndex& index)
{
    const auto extensionVariant = m_proxyModel.index(index.row(), 0).data(Qt::UserRole);
    const auto extension = extensionVariant.toString().toStdString();

    GenerateGraph(extension);
}

std::unique_ptr<QtCharts::QCategoryAxis>
BreakdownDialog::SetupAxisX(const ExtensionDistribution& distribution, const QColor& color) const
{
    auto axisX = std::make_unique<QtCharts::QCategoryAxis>();
    axisX->setLabelsPosition(QtCharts::QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setTitleText("Size");
    axisX->setTitleBrush(color);
    axisX->setLinePenColor(color);
    axisX->setLabelsBrush(color);

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
        const auto label = QString::fromStdString(fmt::format("{:.0f} {}", size, units));

        axisX->append(label, index * tickInterval);
    }

    return axisX;
}

std::unique_ptr<QtCharts::QValueAxis> BreakdownDialog::SetupLinearAxisY(
    const ExtensionDistribution& distribution, const QColor& color) const
{
    const auto tallestBarHeight = distribution.GetMaximumValueY();

    constexpr auto minimumTickCount = 2;
    const auto tickCount =
        tallestBarHeight <= 2 ? minimumTickCount : std::min(static_cast<int>(tallestBarHeight), 4);

    auto axisY = std::make_unique<QtCharts::QValueAxis>();
    axisY->setRange(0, tallestBarHeight);
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Count");
    axisY->setTickCount(tickCount);
    axisY->setTitleBrush(color);
    axisY->setLinePenColor(color);
    axisY->setLabelsBrush(color);

    return axisY;
}

std::unique_ptr<QtCharts::QLogValueAxis> BreakdownDialog::SetupLogarithmAxisY(
    const ExtensionDistribution& distribution, const QColor& color) const
{
    const auto tallestBarHeight = distribution.GetMaximumValueY();

    auto axisY = std::make_unique<QtCharts::QLogValueAxis>();
    axisY->setRange(0.5, tallestBarHeight);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Count");
    axisY->setTitleBrush(color);
    axisY->setLinePenColor(color);
    axisY->setLabelsBrush(color);

    return axisY;
}

std::unique_ptr<QtCharts::QAbstractAxis>
BreakdownDialog::SetupAxisY(const ExtensionDistribution& distribution, const QColor& color) const
{
    if (distribution.GetMaximumValueY() > 32) {
        return SetupLogarithmAxisY(distribution, color);
    }

    return SetupLinearAxisY(distribution, color);
}

void BreakdownDialog::GenerateGraph(const std::string& extension)
{
    const auto& settings = m_mainWindow.GetController().GetPersistentSettings();
    const auto textColor = settings.ShouldUseDarkMode() ? Qt::white : Qt::black;
    const auto backgroundColor = settings.ShouldUseDarkMode() ? QColor{ 50, 65, 75 } : Qt::white;

    const auto& distribution = m_graphModel.GetDistribution(extension);
    const auto& buckets = distribution.GetBuckets();

    auto set = std::make_unique<QtCharts::QBarSet>("Distribution");

    QColor barColor{ 20, 100, 160 };
    set->setColor(barColor);

    for (const auto& count : buckets) {
        set->append(count);
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
    chart->setTitleBrush(QBrush{ textColor });
    chart->setTitle("Size Distribution");

    QBrush backgroundBrush{ backgroundColor };
    chart->setBackgroundBrush(backgroundBrush);

    auto axisX = SetupAxisX(distribution, textColor);
    chart->addAxis(axisX.get(), Qt::AlignBottom);
    series->attachAxis(axisX.release());

    auto axisY = SetupAxisY(distribution, textColor);
    chart->addAxis(axisY.get(), Qt::AlignLeft);
    series->attachAxis(axisY.release());

    // Since `QChart::addSeries(...)` takes ownership of the series resource, one would reasonably
    // assume that the `chart->addSeries(series.release())` call could be moved down after the last
    // use of the `series` pointer. Unfortunately, if one were to do that, and axis labels no longer
    // render. This seems like a quirk in the Qt API design.
    series.release();

    // Another quirk in the API stems from the fact that while the documentation claims "the
    // ownership of the new chart is passed to the chart view," one apparently still has to clean
    // up the previous chart (which we'll do at the end of the current function using RAII).
    const auto previousChart = std::unique_ptr<QtCharts::QChart>(m_ui.graphView->chart());

    m_ui.graphView->setChart(chart.release());
    m_ui.graphView->setBackgroundBrush(backgroundBrush);
    m_ui.graphView->setRenderHint(QPainter::Antialiasing);
}
