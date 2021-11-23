#ifndef WINDOW_TREE_DIALOG_HXX
#define WINDOW_TREE_DIALOG_HXX

#include <QTreeWidget>
#include <QMessageBox>
#include <QGroupBox>
#include <QSplitter>
#include <QLayout>
#include <QDialog>
#include <QAction>
#include <QMenu>

#include <QThread>
#include <QTimer>

#define ENABLE_CONSOLE

#include "qdebugconsole_widget.hpp"
#include "process_scanner.hpp"

namespace Ui {
    class WindowTreeDialog;
}

class WindowTreeDialog : public QDialog {
Q_OBJECT

signals:
    void requestProcessScannerScan(ProcessScanner::SCAN_FILTERS);

    // Emitted whenever the user chooses a window from the process/window tree.
    void processWindowChosen(const QString&);

private:
    Ui::WindowTreeDialog*           ui;

    // Debug console, and related widgets.
    QDebugConsole*                  dbgConsole;
    QGroupBox*                      grpConsole;
    QSplitter*                      splConsole;

    QMenu*                          treeWidgetContextMenu;          // Context menu for

    QTimer*                         autoScannerTimer;
    uint32_t                        autoScannerInterval;            // The default auto-scan interval for the ProcessScanner QTimer.

    bool                            scannerCurrentlyScanning;       // True when ProcessScanner is scanning, false when not. Determined by start/finished signals.
    ProcessScanner                  processScanner;                 // The ProcessScanner instance tasked with WinAPI process scanning.
    QThread                         processScannerThread;           // The thread that the ProcessScanner instance will run on.

    QList<QTreeWidgetItem*>         rootItemRemovalWhitelist;
    QList<QTreeWidgetItem*>         childItemRemovalWhitelist;
    QList<QTreeWidgetItem*>         searchExpansions;

    // Helper functions to collect QTreeWidgetItem*'s into QList's
    QList<QTreeWidgetItem*> collectTreeRoot(QTreeWidget*);
    QList<QTreeWidgetItem*> collectTreeChildren(QTreeWidgetItem*);
    void setAllTopLevelItemsHidden(bool);

private slots:
    void applyFiltersToTree();
    void applyWhitelistToTree();
    void addWindowVisCheckboxesToTree();
    void integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo);

    void emitFilteredProcessScannerScanRequest();
    void onProcessScannerScanStarted();
    void onProcessScannerScanFinished();
    void onAutoScannerTimerTimeout();

    void evaluateWindowTreeItemSelection();
    void showTreeWidgetContextMenu(const QPoint& point);

public:
    explicit WindowTreeDialog(QWidget* parent = nullptr);
    ~WindowTreeDialog() override;

};

#endif // WINDOW_TREE_DIALOG_HXX
