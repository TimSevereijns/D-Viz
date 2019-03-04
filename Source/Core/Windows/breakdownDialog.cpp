#include "Windows/breakdownDialog.h"
#include "Windows/mainWindow.h"

#include <Stopwatch/Stopwatch.hpp>
#include <spdlog/spdlog.h>

#include <QResizeEvent>
#include <QScrollBar>

#include <functional>
#include <iostream>
#include <vector>

BreakdownDialog::BreakdownDialog(QWidget* parent)
    : QDialog{ parent }, m_mainWindow{ *(dynamic_cast<MainWindow*>(parent)) }, m_ui{}
{
    m_ui.setupUi(this);

    ReloadData();
}

void BreakdownDialog::ReloadData()
{
    m_model.ClearData();

    const auto& controller = m_mainWindow.GetController();
    const auto& tree = controller.GetTree();

    if (tree.GetRoot()->GetChildCount() == 0) {
        return;
    }

    const auto& parameters = controller.GetSettingsManager().GetVisualizationParameters();

    Stopwatch<std::chrono::milliseconds>(
        [&] {
            for (const auto& node : tree) {
                const auto fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
                if (fileIsTooSmall) {
                    continue;
                }

                if (node->file.type != FileType::DIRECTORY) {
                    m_model.insert(node);
                }
            }
        },
        [](const auto& elapsed, const auto& units) noexcept {
            spdlog::get(Constants::Logging::DEFAULT_LOG)
                ->info(fmt::format("Built break-down model in: {} {}", elapsed.count(), units));
        });

    m_model.FinalizeInsertion(controller.GetSettingsManager());

    m_proxyModel.setSourceModel(&m_model);
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

    m_ui.tableView->setColumnWidth(0, tableWidth / 2);
    m_ui.tableView->setColumnWidth(1, tableWidth / 2);
}

void BreakdownDialog::resizeEvent(QResizeEvent* /*event*/)
{
    AdjustColumnWidthsToFitViewport();
}
