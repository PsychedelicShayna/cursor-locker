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

#include "qdebugconsole_widget.hpp"
#include "process_scanner.hpp"

namespace Ui {
    class WindowTreeDialog;
}

class WindowTreeDialog : public QDialog {
Q_OBJECT

signals:
    void requestProcessScannerScan();

private:
    Ui::WindowTreeDialog*   ui;

    // Debug console, and related widgets.
    QDebugConsole*          dbgConsole;
    QGroupBox*              grpConsole;
    QSplitter*              splConsole;

    // Helper functions to collect QTreeWidgetItem*'s into QList's
    QList<QTreeWidgetItem*> collectTreeRoot(QTreeWidget*);
    QList<QTreeWidgetItem*> collectTreeChildren(QTreeWidgetItem*);

    QTimer* autoScannerTimer;
    const uint32_t autoScannerInterval;     // The default auto-scan interval for the ProcessScanner QTimer.

    ProcessScanner processScanner;          // The ProcessScanner instance tasked with WinAPI process scanning.
    QThread processScannerThread;           // The thread that the ProcessScanner instance will run on.

    QList<QTreeWidgetItem*> rootItemRemovalWhitelist;
    QList<QTreeWidgetItem*> childItemRemovalWhitelist;

    bool expandAll;

    void setAllTopLevelItemsHidden(bool);

private slots:
    void applyFiltersToTree();
    void applyWhitelistToTree();
    void addWindowVisCheckboxesToTree();
    void mergeRootItemWithTree(QTreeWidgetItem*);

    void onProcessScannerScanStarted();
    void onProcessScannerScanFinished();

    void onAutoScannerTimerTimeout();
    void startAutoScanner();
    void stopAutoScanner();

    void memoryLeakTest();

    void showTreeWidgetContextMenu(const QPoint& point);

protected:
    virtual void closeEvent(QCloseEvent*) override;

public:
    explicit WindowTreeDialog(QWidget* parent = nullptr);
    ~WindowTreeDialog() override;

};

#endif // WINDOW_TREE_DIALOG_HXX
