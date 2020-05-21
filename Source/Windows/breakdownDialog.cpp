#include "Windows/breakdownDialog.h"

#include "Windows/mainWindow.h"
#include "controller.h"

#include <Stopwatch/Stopwatch.hpp>
#include <spdlog/spdlog.h>

#include <QBrush>
#include <QGraphicsLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>

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

void BreakdownDialog::GenerateGraph(const std::wstring& extension)
{
    auto* series = new QtCharts::QLineSeries{};
    const auto& distribution = m_graphModel.GetDistribution(extension);
    const auto& buckets = distribution.GetBuckets();

    for (std::size_t index = 0; index < buckets.size(); ++index) {
        series->append(index, buckets[index]);
    }

    auto* chart = new QtCharts::QChart{};
    chart->addSeries(series);
    chart->layout()->setContentsMargins(0, 0, 16, 0); //< Add space on the right for last label.
    chart->legend()->hide();
    chart->setBackgroundRoundness(0);

    QFont font;
    font.setPixelSize(12);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush{ Qt::black });
    chart->setTitle("Size Distribution");

    auto* axisX = new QtCharts::QCategoryAxis{};
    axisX->setLabelsPosition(QtCharts::QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setTitleText("Size");

    const auto tickCount = axisX->tickCount();
    const auto tickInterval = distribution.GetBucketCount() / tickCount;
    axisX->setTickInterval(tickInterval);

    const auto prefix = m_mainWindow.GetController().GetSessionSettings().GetActiveNumericPrefix();

    axisX->append("0", 0);

    for (int index = 1; index <= tickCount; ++index) {
        const auto value = index / static_cast<double>(tickCount) * distribution.GetMaximumValueX();
        const auto [size, units] = Utilities::ToPrefixedSize(value, prefix);
        const auto label = QString::fromStdWString(fmt::format(L"{:.0f} {}", size, units));

        axisX->append(label, index * tickInterval);
    }

    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QtCharts::QValueAxis();
    axisY->setRange(0, std::min(256ull, distribution.GetMaximumValueY()));
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Count");
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_ui.graphView->setChart(chart);
    m_ui.graphView->setBackgroundBrush(Qt::white);
    m_ui.graphView->setRenderHint(QPainter::Antialiasing);
}
