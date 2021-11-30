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

// #include <functional>

#include <QThread>
#include <QTimer>

#define ENABLE_CONSOLE

#include "debug_console_widget.hpp"
#include "process_scanner.hpp"

namespace Ui {
    class ProcessScannerDialog;
}

class ProcessScannerDialog : public QDialog {
Q_OBJECT

signals:
    void requestProcessScannerScan(ProcessScanner::SCAN_SCOPE, ProcessScanner::SCAN_FILTERS);

    // Emitted whenever the user chooses a window from the process/window tree.
    void treeSelectionMade(const QString&);

private:
    Ui::ProcessScannerDialog*           ui;

    // Debug console, and related widgets.
    QDebugConsole*                  dbgConsole;
    QGroupBox*                      grpConsole;
    QSplitter*                      splConsole;

    QMenu*                          treeWidgetContextMenu;          // Context menu for

    QTimer*                         autoScannerTimer;
    uint32_t                        autoScannerInterval;            // The default auto-scan interval for the ProcessScanner QTimer.

    bool                            scannerCurrentlyScanning;       // True when ProcessScanner is scanning, false when not. Determined by start/finished signals.

    ProcessScanner                  processScanner;                 // The ProcessScanner instance tasked with WinAPI process scanning.
    ProcessScanner::SCAN_SCOPE      processScannerScope;            // The scope of the ProcessScanner instance (what to scan for).
    QThread                         processScannerThread;           // The thread that the ProcessScanner instance will run on.

    QList<QTreeWidgetItem*>         rootItemRemovalWhitelist;
    QList<QTreeWidgetItem*>         childItemRemovalWhitelist;
    QList<QTreeWidgetItem*>         searchExpansions;

    // Helper functions to collect QTreeWidgetItem*'s into QList's
    QList<QTreeWidgetItem*> collectTreeRoot(QTreeWidget*);
    QList<QTreeWidgetItem*> collectTreeChildren(QTreeWidgetItem*);

    template<typename T>
    void applyFuncToQList(QList<T> list, std::function<void(T&)> func) {
        std::for_each(list.begin(), list.end(), func);
    }

    void setAllTopLevelItemsHidden(bool);

private slots:
    void applySearchFilterToTree();
    void integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo);
    void applyWhitelistToTree();

    void emitFilteredProcessScannerScanRequest();
    void onProcessScannerScanStarted();
    void onProcessScannerScanFinished();
    void onAutoScannerTimerTimeout();

    void evaluateWindowTreeItemSelection();
    void showTreeWidgetContextMenu(const QPoint& point);

public:
    explicit ProcessScannerDialog(const ProcessScanner::SCAN_SCOPE&, QWidget* parent = nullptr);
    ~ProcessScannerDialog() override;

};

#endif // WINDOW_TREE_DIALOG_HXX
