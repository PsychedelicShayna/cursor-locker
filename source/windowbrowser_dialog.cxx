#include "windowbrowser_dialog.hxx"
#include "ui_windowbrowser_dialog.h"


QList<QTreeWidgetItem*> WindowTreeDialog::collectTreeRoot(QTreeWidget* tree) {
    QList<QTreeWidgetItem*> root_items_list;
    root_items_list.reserve(tree->topLevelItemCount());

    for(int32_t i { 0 }; i < tree->topLevelItemCount(); ++i) {
        root_items_list.append(tree->topLevelItem(i));
    }

    return root_items_list;
}

QList<QTreeWidgetItem*> WindowTreeDialog::collectTreeChildren(QTreeWidgetItem* root_item) {
    QList<QTreeWidgetItem*> child_items_list;
    child_items_list.reserve(root_item->childCount());

    for(int32_t i { 0 }; i < root_item->childCount(); ++i) {
        child_items_list.append(root_item->child(i));
    }

    return child_items_list;
}

void WindowTreeDialog::setAllTopLevelItemsHidden(bool hidden) {
    auto* tree = ui->trwWindowTree;

    for(auto iter_root_item : collectTreeRoot(tree)) {
        iter_root_item->setHidden(hidden);

        for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
            iter_child_item->setHidden(hidden);
        }
    }
}

void WindowTreeDialog::integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo process_info) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "integrateProcessInfoIntoTree" };

    auto* tree = ui->trwWindowTree;
    QTreeWidgetItem* existing_root_item { nullptr };

    for(auto iter_root_item : collectTreeRoot(tree)) {
        if(iter_root_item->text(1) == process_info.ProcessId) {
            existing_root_item = iter_root_item;
            break;
        }
    }

    // If item is unique, add it to the tree.
    if(existing_root_item == nullptr) {
        auto new_root_item { process_info.MakeRootItem() };

        // Whitelist the new root item, to prevent it from being deleted when the scan is finished.
        rootItemRemovalWhitelist.append(new_root_item);
        tree->addTopLevelItem(new_root_item);

        childItemRemovalWhitelist += collectTreeChildren(new_root_item);
    }

    // If it already exists, check to see if children differ.
    else {
        // Whitelist the existing root item, to prevent it from being deleted when the scan is finished.
        rootItemRemovalWhitelist.append(existing_root_item);

        for(auto process_window : process_info.ProcessWindows) {
            QTreeWidgetItem* existing_child_item { nullptr };

            for(auto iter_child_item : collectTreeChildren(existing_root_item)) {
                if(iter_child_item->text(1) == process_window.WindowHash) {
                    existing_child_item = iter_child_item;
                    break;
                }
            }

            if(existing_child_item == nullptr) {
                auto new_child_item { process_window.MakeChildItem() };
                childItemRemovalWhitelist.append(new_child_item);
                existing_root_item->addChild(new_child_item);
            } else {
                childItemRemovalWhitelist.append(existing_child_item);

                if(existing_child_item->text(2) != process_window.WindowVisibility) {
                    existing_child_item->setText(2, process_window.WindowVisibility);
                }
            }
        }
    }
}

void WindowTreeDialog::applyWhitelistToTree() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "applyWhitelistToTree" };

    auto* tree = ui->trwWindowTree;

    for(auto iter_root_item : collectTreeRoot(tree)) {

        if(!rootItemRemovalWhitelist.contains(iter_root_item)) {
            dbgConsole->log({"Deleting root item: ", iter_root_item->text(0)});
            delete iter_root_item;
        } else {
            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                if(!childItemRemovalWhitelist.contains(iter_child_item)) {
                    dbgConsole->log({"Deleting child item: ", iter_child_item->text(0)});
                    delete iter_child_item;
                }
            }
        }
    }
}

void WindowTreeDialog::onProcessScannerScanStarted() {
    ui->btnScan->setText("Scanning...");
    ui->btnScan->setEnabled(false);
    scannerCurrentlyScanning = true;

    rootItemRemovalWhitelist.clear();
    childItemRemovalWhitelist.clear();
}

void WindowTreeDialog::onProcessScannerScanFinished() {
    applyWhitelistToTree();
    ui->btnScan->setText("Scan Processes");
    ui->btnScan->setEnabled(true);
    scannerCurrentlyScanning = false;
}

void WindowTreeDialog::emitFilteredProcessScannerScanRequest() {
    if(scannerCurrentlyScanning) return;

    QDebugConsoleContext { dbgConsole, "(filteredProcessScanRequest)" };

    dbgConsole->log("Filtered process scan has been requested.");

    uint32_t scan_filters = NULL;

    if(ui->cbFilterInvisibleWindows->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_INVISIBLE_WINDOWS;
        dbgConsole->log("Added FILTER_INVISIBLE_WINDOWS to filter bitmask.");
    }

    if(ui->cbFilterDuplicateWindows->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_DUPLICATE_WINDOWS;
        dbgConsole->log("Added FILTER_DUPLICATE_WINDOWS to filter bitmask.");
    }

    if(ui->cbFilterDuplicateProcesses->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_DUPLICATE_PROCESSES;
        dbgConsole->log("Added FILTER_DUPLICATE_PROCESSES to filter bitmask.");
    }

    if(scan_filters == NULL) {
        dbgConsole->log("No scan filters have been provided.");
    }

    emit requestProcessScannerScan(static_cast<ProcessScanner::SCAN_FILTERS>(scan_filters));
}

void WindowTreeDialog::onAutoScannerTimerTimeout() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "AutoScannerTimeout" };

    emitFilteredProcessScannerScanRequest();
    autoScannerTimer->stop();

    if(ui->cbAutoScanner->isChecked()) {
        autoScannerTimer->start(autoScannerInterval);
        dbgConsole->log({"Autoscan timer started with interval: ", QString::number(autoScannerInterval)});
    } else {
        dbgConsole->log("Autoscan timer has been deactivated.");
    }
}

void WindowTreeDialog::evaluateWindowTreeItemSelection() {
    auto tree { ui->trwWindowTree };
    auto selected_items { tree->selectedItems() };

    if(selected_items.size()) {
        auto selected_item { selected_items.first() };

        if(!selected_item->isHidden() && !selected_item->childCount()) {
            emit processWindowChosen(selected_item->text(0));
        }
    }
}

void WindowTreeDialog::showTreeWidgetContextMenu(const QPoint& point) {
    const QPoint global_point { ui->trwWindowTree->mapToGlobal(point) };
    treeWidgetContextMenu->popup(global_point);
}


WindowTreeDialog::WindowTreeDialog(QWidget *parent)
    :
      // WindowTreeDialog Ui initialization.
      QDialog               { parent },
      ui                    { new Ui::WindowTreeDialog   },

      // Debug console & related widgets initialization.
      dbgConsole            { new QDebugConsole { this } },
      grpConsole            { new QGroupBox },
      splConsole            { new QSplitter },

      treeWidgetContextMenu { new QMenu { this } },

      // Member variable initialization.
      autoScannerTimer          { new QTimer },
      autoScannerInterval       { 2500 },
      scannerCurrentlyScanning  { false }
{
    ui->setupUi(this);

    /* Initialize & Configure Debug Console * * * * * * * * * * * * */
    QVBoxLayout* grpConsoleLayout { new QVBoxLayout(grpConsole) };
    grpConsoleLayout->addWidget(dbgConsole);
    grpConsole->setLayout(grpConsoleLayout);
    grpConsole->setTitle("Debug Console");

    splConsole->setOrientation(Qt::Orientation::Vertical);
    splConsole->addWidget(ui->trwWindowTree);
    splConsole->addWidget(grpConsole);

    splConsole->setCollapsible(0, false);
    splConsole->setCollapsible(1, true);
    splConsole->setStretchFactor(0, 0);
    splConsole->setStretchFactor(0, 0);
    splConsole->setSizes({400, 0});

    ui->vlTree->addWidget(splConsole, 0);
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // Set initial tree widget header section sizes.
    ui->trwWindowTree->header()->resizeSection(0, 250);
    ui->trwWindowTree->header()->resizeSection(1, 130);
    ui->trwWindowTree->header()->resizeSection(2, 40);

    // Context Menu Signals -----------------------------------------------------------------------
    connect(ui->trwWindowTree,          SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                       SLOT(showTreeWidgetContextMenu(const QPoint&)));

    connect(treeWidgetContextMenu->addAction("Expand All"), &QAction::triggered, [this]() -> void {
        ui->trwWindowTree->expandAll();
    });

    connect(treeWidgetContextMenu->addAction("Collapse All"), &QAction::triggered, [this]() -> void {
        ui->trwWindowTree->collapseAll();
    });

    // Search Filter Signal -----------------------------------------------------------------------
    connect(ui->linSearchFilter,        SIGNAL(textChanged(QString)),
            this,                       SLOT(applyFiltersToTree()));

    // Item Selection Signals ---------------------------------------------------------------------
    connect(ui->btnOkay,                SIGNAL(clicked()),
            this,                       SLOT(evaluateWindowTreeItemSelection()));

    connect(ui->trwWindowTree,          SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this,                       SLOT(evaluateWindowTreeItemSelection()));

    // ProcessScanner Signals  --------------------------------------------------------------------
    connect(this,                       SIGNAL(requestProcessScannerScan(ProcessScanner::SCAN_FILTERS)),
            &processScanner,            SLOT(PerformScan(ProcessScanner::SCAN_FILTERS)));

    connect(ui->btnScan,                SIGNAL(clicked()),
            this,                       SLOT(emitFilteredProcessScannerScanRequest()));

    connect(ui->btnCancel,              SIGNAL(clicked()),
            this,                       SLOT(close()));

   connect(&processScannerThread,       SIGNAL(started()),
           this,                        SLOT(emitFilteredProcessScannerScanRequest()));

    connect(&processScanner,            SIGNAL(ProcessInformationReady(ProcessScanner::ProcessInfo)),
            this,                       SLOT(integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo)));

    connect(&processScanner,            SIGNAL(ScanStarted()),
            this,                       SLOT(onProcessScannerScanStarted()));

    connect(&processScanner,            SIGNAL(ScanFinished()),
            this,                       SLOT(onProcessScannerScanFinished()));

    // AutoScanner Signals ------------------------------------------------------------------------
    connect(autoScannerTimer,           SIGNAL(timeout()),
            this,                       SLOT(onAutoScannerTimerTimeout()));

    connect(ui->cbAutoScanner,          SIGNAL(stateChanged(int)),
            this,                       SLOT(onAutoScannerTimerTimeout()));

    connect(ui->hsAutoScannerInterval,  SIGNAL(sliderMoved(int)),
            ui->sbAutoScannerInterval,  SLOT(setValue(int)));

    connect(ui->sbAutoScannerInterval,  SIGNAL(valueChanged(int)),
            ui->hsAutoScannerInterval,  SLOT(setValue(int)));

    connect(ui->sbAutoScannerInterval,  &QSpinBox::valueChanged, [this](int interval) -> void {
        autoScannerInterval = interval;
    });

    // Set initial filter checkbox states, before thread is started.
    ui->cbFilterInvisibleWindows->setChecked(true);
    ui->cbFilterDuplicateWindows->setChecked(true);

    processScanner.moveToThread(&processScannerThread);     // Move the ProcessScanner instance to the QThread.
    processScannerThread.start();                           // Start the QThread attached to the instance.
}

WindowTreeDialog::~WindowTreeDialog() {
    autoScannerTimer->stop();
    ui->cbAutoScanner->setChecked(false);
    processScannerThread.quit();
    processScannerThread.terminate();
    processScannerThread.wait();
    delete ui;
}



