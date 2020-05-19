#include "Windows/breakdownDialog.h"

#include "Windows/mainWindow.h"
#include "controller.h"

#include <Stopwatch/Stopwatch.hpp>
#include <spdlog/spdlog.h>

#include <QBrush>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtCharts/QCategoryAxis>
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
    m_ui.tableView->setSortingEnabled(true);
    m_ui.tableView->sortByColumn(1, Qt::SortOrder::DescendingOrder);

    AdjustColumnWidthsToFitViewport();
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

    auto* series = new QtCharts::QLineSeries();
    const auto& distribution = m_graphModel.GetDistribution(extension);
    const auto& buckets = distribution.GetBuckets();

    for (std::size_t index{ 0 }; index < buckets.size(); ++index) {
        series->append(index, buckets[index]);
    }

    auto* chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();

    QFont font;
    font.setPixelSize(18);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush{ Qt::black });
    chart->setTitle("Size Distribution");

    m_ui.graphView->setChart(chart);
    m_ui.graphView->setRenderHint(QPainter::Antialiasing);
}
