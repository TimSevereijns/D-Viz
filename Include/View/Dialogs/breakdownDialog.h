#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLogValueAxis>

#include <functional>

#include "View/Dialogs/distributionGraphModel.h"
#include "View/Dialogs/scanBreakdownModel.h"
#include "ui_breakdownDialog.h"

#include <gsl/gsl_assert>

class MainWindow;

class ScanBreakdownFilterProxyModel final : public QSortFilterProxyModel
{
    bool lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const override
    {
        const auto lhsData = sourceModel()->data(lhs, Qt::UserRole);
        const auto rhsData = sourceModel()->data(rhs, Qt::UserRole);

        switch (lhs.column()) {
            case 0: {
                const auto lhsExtension = lhsData.value<QString>();
                const auto rhsExtension = rhsData.value<QString>();

                return lhsExtension < rhsExtension;
            }
            case 1: {
                const auto lhsVisibleSize = lhsData.value<std::uintmax_t>();
                const auto rhsVisibleSize = rhsData.value<std::uintmax_t>();

                return lhsVisibleSize < rhsVisibleSize;
            }
            case 2: {
                const auto lhsTotalSize = lhsData.value<std::uintmax_t>();
                const auto rhsTotalSize = rhsData.value<std::uintmax_t>();

                return lhsTotalSize < rhsTotalSize;
            }
            case 3: {
                const auto lhsVisibleCount = lhsData.value<std::uintmax_t>();
                const auto rhsVisibleCount = rhsData.value<std::uintmax_t>();

                return lhsVisibleCount < rhsVisibleCount;
            }
            case 4: {
                const auto lhsTotalCount = lhsData.value<std::uintmax_t>();
                const auto rhsTotalCount = rhsData.value<std::uintmax_t>();

                return lhsTotalCount < rhsTotalCount;
            }
            default: {
                Expects(false);
                return false;
            }
        }
    }
};

/**
 * @brief A pop-out dialog that displays scan metadata and statistics.
 */
class BreakdownDialog final : public QDialog
{
    Q_OBJECT

  public:
    explicit BreakdownDialog(QWidget* parent = nullptr);

    void ReloadData();

  protected:
    void resizeEvent(QResizeEvent* event) override;

  private slots:
    void DisplayContextMenu(const QPoint& point);

    void HandleDoubleClick(const QModelIndex& index);

  private:
    void BuildModel();

    void AdjustColumnWidthsToFitViewport();

    std::unique_ptr<QtCharts::QCategoryAxis>
    SetupAxisX(const ExtensionDistribution& distribution, const QColor& color) const;

    std::unique_ptr<QtCharts::QValueAxis>
    SetupLinearAxisY(const ExtensionDistribution& distribution, const QColor& color) const;

    std::unique_ptr<QtCharts::QLogValueAxis>
    SetupLogarithmAxisY(const ExtensionDistribution& distribution, const QColor& color) const;

    std::unique_ptr<QtCharts::QAbstractAxis>
    SetupAxisY(const ExtensionDistribution& distribution, const QColor& color) const;

    void GenerateGraph(const std::wstring& extension);

    MainWindow& m_mainWindow;

    Ui::breakdownDialog m_ui;

    DistributionGraphModel m_graphModel;

    ScanBreakdownModel m_tableModel;
    ScanBreakdownFilterProxyModel m_proxyModel;
};

#endif // BREAKDOWNDIALOG_H
