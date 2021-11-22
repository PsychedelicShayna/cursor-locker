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

void WindowTreeDialog::applyFiltersToTree() {
    const QString& search_filter    { ui->linSearchFilter->text() };
    const bool& filter_invisible    { ui->cbFilterInvisible->isChecked() };
    const bool& filter_duplicates   { ui->cbFilterDuplicates->isChecked() };
    auto* tree                      { ui->trwWindowTree };

    setAllTopLevelItemsHidden(false);

    if(search_filter.size()) {
        setAllTopLevelItemsHidden(true);

        for(auto iter_root_item : collectTreeRoot(tree)) {
            bool child_found_filter { false };

            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                if(iter_child_item->text(0).contains(search_filter, Qt::CaseInsensitive)) {
                    iter_child_item->setHidden(false);
                    child_found_filter = true;
                }
            }

            if(!child_found_filter) {
                if(iter_root_item->text(0).contains(search_filter, Qt::CaseInsensitive)) {
                    iter_root_item->setHidden(false);

                    for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                        iter_child_item->setHidden(false);
                    }
                }
            } else {
                iter_root_item->setHidden(false);
            }
        }
    }

    if(filter_invisible) {
        for(auto iter_root_item : collectTreeRoot(tree)) {
            if(iter_root_item->isHidden()) {
                continue;
            }

            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                if(iter_child_item->isHidden()) {
                    continue;
                }

                auto child_checkbox_widget {
                    qobject_cast<QCheckBox*>(tree->itemWidget(iter_child_item, 2))
                };

                if(child_checkbox_widget) {
                    if(!child_checkbox_widget->isChecked()) {
                        iter_child_item->setHidden(true);
                    }
                } else {
                    dbgConsole->log({"(VisibilityFilter) Encountered Null QCheckBox* for child: ", iter_child_item->text(0)}, QDebugConsole::LL_ERROR);
                }
            }
        }
    }

    if(filter_duplicates) {
        for(auto iter_root_item : collectTreeRoot(tree)) {
            if(iter_root_item->isHidden()) {
                continue;
            }

            QList<QString> existing_window_names;
            existing_window_names.reserve(iter_root_item->childCount());

            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                const auto& window_title { iter_child_item->text(0) };

                if(iter_child_item->isHidden()) {
                    continue;
                }

                if(existing_window_names.contains(window_title)) {
                    iter_child_item->setHidden(true);
                } else {
                    existing_window_names.append(window_title);
                }
            }
        }
    }

    // Hide root items that don't have any visible children.
    for(auto iter_root_item : collectTreeRoot(tree)) {
        if(iter_root_item->isHidden()) continue;

        int32_t visible_child_counter { 0 };

        for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
            if(!iter_child_item->isHidden()) ++visible_child_counter;
        }

        if(!visible_child_counter) {
            iter_root_item->setHidden(true);
        }
    }

    if(expandAll) {
        tree->expandAll();
    }
}

void WindowTreeDialog::mergeRootItemWithTree(QTreeWidgetItem* new_root_item) {
    auto* tree = ui->trwWindowTree;
    QTreeWidgetItem* existing_root_item { nullptr };

    for(auto iter_root_item : collectTreeRoot(tree)) {
        if(new_root_item->text(1) == iter_root_item->text(1)) {
            existing_root_item = iter_root_item;
            break;
        }
    }

    // If item is unique, add it to the tree.
    if(existing_root_item == nullptr) {
        // Whitelist the new root item, to prevent it from being deleted when the scan is finished.
        rootItemRemovalWhitelist.append(new_root_item);
        tree->addTopLevelItem(new_root_item);
        // dbgConsole->log({"(mergeRootItemWithTree) Added new root item: ", new_root_item->text(0)});

        childItemRemovalWhitelist += collectTreeChildren(new_root_item);
    }

    // If it already exists, check to see if children differ.
    else {
        // Whitelist the existing root item, to prevent it from being deleted when the scan is finished.
        rootItemRemovalWhitelist.append(existing_root_item);

        // dbgConsole->log({"(mergeRootItemWithTree) Root item already exists: ", new_root_item->text(0)});

        // TODO: DELETE ITEM WHEN ALREADY EXISTS, TO PREVENT MEMORY LEAK

        for(auto new_child_item : collectTreeChildren(new_root_item)) {
            QTreeWidgetItem* existing_child_item { nullptr };

            for(auto iter_child_item : collectTreeChildren(existing_root_item)) {
                if(new_child_item->text(1) == iter_child_item->text(1)) {
                    existing_child_item = iter_child_item;
                    break;
                }
            }

            if(existing_child_item == nullptr) {
                childItemRemovalWhitelist.append(new_child_item);

                int new_child_index { new_root_item->indexOfChild(new_child_item) };
                existing_root_item->addChild(new_root_item->takeChild(new_child_index));

                // dbgConsole->log({"(mergeRootItemWithTree) Added new child item to existing root item: ",
                //                    existing_root_item->text(0),
                //                    "@",
                //                    new_child_item->text(0)
                //                   });
            } else {
                childItemRemovalWhitelist.append(existing_child_item);

                QWidget* existing_child_widget { tree->itemWidget(existing_child_item, 2) };
                QCheckBox* existing_child_checkbox { qobject_cast<QCheckBox*>(existing_child_widget) };

                if(existing_child_checkbox) {
                    if (existing_child_checkbox->isChecked() != (new_child_item->text(2) == "Visible")) {
                        existing_child_checkbox->setChecked(new_child_item->text(2) == "Visible");

                        // dbgConsole->log({"(mergeRootItemWithTree) Modified existing child item in existing root item: ",
                        //                    existing_root_item->text(0),
                        //                    "@",
                        //                    existing_child_item->text(0),
                        //                    " - new visibility state is ",
                        //                    existing_child_item->text(2)
                        //                   });
                    }
                } else {
                    dbgConsole->log({"(mergeRootItemWithTree) Found existing child item without a checkbox? ",
                                       existing_root_item->text(0),
                                       "@",
                                       existing_child_item->text(0)
                                      }, QDebugConsole::LL_WARNING);
                }
            }
        }
    }
}

void WindowTreeDialog::applyWhitelistToTree() {
    auto* tree = ui->trwWindowTree;

    for(auto iter_root_item : collectTreeRoot(tree)) {

        if(!rootItemRemovalWhitelist.contains(iter_root_item)) {
            dbgConsole->log({"(applyWhitelistToTree) Deleting root item: ", iter_root_item->text(0)});
            delete iter_root_item;
        } else {
            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                if(!childItemRemovalWhitelist.contains(iter_child_item)) {
                    dbgConsole->log({"(applyWhitelistToTree) Deleting child item: ", iter_child_item->text(0)});
                    delete iter_child_item;
                }
            }
        }
    }
}

void WindowTreeDialog::addWindowVisCheckboxesToTree() {
    auto* tree = ui->trwWindowTree;

    for(auto iter_root_item : collectTreeRoot(tree)) {
        for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
            const QString& visibility_text { iter_child_item->text(2) };

            if(visibility_text == "Visible" || visibility_text == "Invisible") {
                QCheckBox* visible_checkbox { new QCheckBox(tree) };
                visible_checkbox->setChecked(visibility_text == "Visible");
                visible_checkbox->setText(visibility_text);
                visible_checkbox->setAttribute(Qt::WA_TransparentForMouseEvents);
                visible_checkbox->setFocusPolicy(Qt::NoFocus);

                tree->setItemWidget(iter_child_item, 2, visible_checkbox);
                iter_child_item->setText(2, "");
            }
        }
    }
}

void WindowTreeDialog::onProcessScannerScanStarted() {
    ui->btnScan->setText("Scanning...");
    ui->btnScan->setEnabled(false);

    rootItemRemovalWhitelist.clear();
    childItemRemovalWhitelist.clear();
}

void WindowTreeDialog::onProcessScannerScanFinished() {
    applyWhitelistToTree();
    addWindowVisCheckboxesToTree();
    applyFiltersToTree();

    ui->btnScan->setText("Scan Processes");
    ui->btnScan->setEnabled(true);
}

void WindowTreeDialog::onAutoScannerTimerTimeout() {

}

void WindowTreeDialog::startAutoScanner() {

}

void WindowTreeDialog::stopAutoScanner() {

}

void WindowTreeDialog::memoryLeakTest() {
    for(int i { 0 }; i < ui->trwWindowTree->topLevelItemCount(); ++i) {
        auto iter_root_item = ui->trwWindowTree->topLevelItem(i);

        for(int c { 0 }; c < iter_root_item->childCount(); ++c) {
            auto iter_child_item = iter_root_item->child(c);
        }
    }

    for(auto iter_root_item : collectTreeRoot(ui->trwWindowTree)) {
        for(auto iter_child_item : collectTreeChildren(iter_root_item)) {

        }
    }
}

void WindowTreeDialog::showTreeWidgetContextMenu(const QPoint& point) {
    static QMenu* context_menu { nullptr };
    const QPoint global_point { ui->trwWindowTree->mapToGlobal(point) };

    if(context_menu == nullptr) {
        context_menu = new QMenu { this };

        QAction* action_collapse_all { context_menu->addAction("Collapse All") };
        QAction* action_expand_all { context_menu->addAction("Expand All") };

        connect(action_expand_all, &QAction::triggered, [this]() -> void {
            ui->trwWindowTree->expandAll();
            expandAll = true;
        });

        connect(action_collapse_all, &QAction::triggered, [this]() -> void {
            ui->trwWindowTree->collapseAll();
            expandAll = false;
        });

    }

    context_menu->popup(global_point);
}

void WindowTreeDialog::closeEvent(QCloseEvent* close_event)  {
    ui->cbAutoScanner->setChecked(false);
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

      // Member variable initialization.
      autoScannerTimer      { new QTimer },
      autoScannerInterval   { 2000 },
      expandAll             { true }
{
    ui->setupUi(this);

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

    // splConsole->setHandleWidth()
    ui->vlTree->addWidget(splConsole, 0);

    // ui->vlTree->insertWidget(2, dbgConsole, 0);

    ui->trwWindowTree->header()->resizeSection(0, 250);
    ui->trwWindowTree->header()->resizeSection(1, 130);
    ui->trwWindowTree->header()->resizeSection(2, 40);

    // Context Menu
    connect(ui->trwWindowTree,          SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                       SLOT(showTreeWidgetContextMenu(const QPoint&)));

    connect(ui->linSearchFilter,        SIGNAL(textChanged(QString)),
            this,                       SLOT(applyFiltersToTree()));

    connect(ui->cbFilterInvisible,      SIGNAL(stateChanged(int)),
            this,                       SLOT(applyFiltersToTree()));

    connect(ui->cbFilterDuplicates,     SIGNAL(stateChanged(int)),
            this,                       SLOT(applyFiltersToTree()));

    processScanner.moveToThread(&processScannerThread);

    // ProcessScanner & Thread Signals ------------------------------------------------------------
    connect(this,                       SIGNAL(requestProcessScannerScan()),
            &processScanner,            SLOT(PerformScan()));

    connect(ui->btnScan,                SIGNAL(clicked()),
            this,                       SIGNAL(requestProcessScannerScan()));

    connect(&processScannerThread,      SIGNAL(started()),
            this,                       SIGNAL(requestProcessScannerScan()));

    connect(&processScanner,            SIGNAL(RootItemReady(QTreeWidgetItem*)),
            this,                       SLOT(mergeRootItemWithTree(QTreeWidgetItem*)));

    connect(&processScanner,            SIGNAL(ScanStarted()),
            this,                       SLOT(onProcessScannerScanStarted()));

    connect(&processScanner,            SIGNAL(ScanFinished()),
            this,                       SLOT(onProcessScannerScanFinished()));

    connect(ui->cbAutoScanner, &QCheckBox::stateChanged, [this](int state) -> void {
        if(state) {
            startAutoScanner();
        } else {
            stopAutoScanner();
        }
    });

    connect(ui->btnMemory,              SIGNAL(clicked()),
            this,                       SLOT(memoryLeakTest));

    processScannerThread.start();

    ui->cbFilterDuplicates->setChecked(true);
    ui->cbFilterInvisible->setChecked(true);
}

WindowTreeDialog::~WindowTreeDialog() {
    stopAutoScanner();
    processScannerThread.quit();
    processScannerThread.terminate();
    processScannerThread.wait();
    delete ui;
}



