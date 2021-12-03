#include "check_box_list_widget.hpp"

bool QCheckBoxList::event(QEvent* event) {
    if(event->type() == QEvent::Type::Wheel) {
        event->accept();
        return true;
    }

    return QComboBox::event(event);
}

void QCheckBoxList::SetHeaderLabel(const QString& header) {
    HeaderItem->setText(header);
}


void QCheckBoxList::InsertCheckableItem(const qint32 &index, const QString &label, const Qt::CheckState &initial_checked_state) {
    QStandardItem* new_item { new QStandardItem };
    new_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    new_item->setData(initial_checked_state, Qt::CheckStateRole);
    new_item->setText(label);

    StandardItemModel->insertRow(index, new_item);
}

void QCheckBoxList::AddCheckableItem(const QString &label, const Qt::CheckState& initial_checked_state) {
    InsertCheckableItem(count(), label, initial_checked_state);
}

void QCheckBoxList::AddCheckableItems(const QList<QPair<QString, Qt::CheckState>>& item_value_pairs) {
    for(QPair<QString, Qt::CheckState> item_value_pair : item_value_pairs) {
        AddCheckableItem(item_value_pair.first, item_value_pair.second);
    }
}

void QCheckBoxList::AddCheckableItems(const QList<QString>& item_labels, const Qt::CheckState& initial_checked_state) {
    for(const auto& item_label : item_labels) {
        AddCheckableItem(item_label, initial_checked_state);
    }
}

QStandardItem* QCheckBoxList::ItemAt(const quint32& index) {
    return StandardItemModel->item(index);
}

QStandardItem* QCheckBoxList::ItemWithLabel(const QString& label) {
    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        if(item->text() == label) {
            return item;
        }
    }

    return nullptr;
}



QList<QStandardItem*> QCheckBoxList::ItemsWithLabel(const QString& label) {
    QList<QStandardItem*> matching_items;

    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        if(item->text() == label) {
            matching_items.emplace_back(item);
        }
    }

    return matching_items;
}

QCheckBoxList::QCheckBoxList(QWidget* parent)
    :
      QComboBox            { parent                          },
      StandardItemModel    { new QStandardItemModel { this } },
      HeaderItem           { new QStandardItem               }
{
    setModel(StandardItemModel);
}




void QKbModifierList::handleItemDataChanged(QStandardItem* item) {
    const WINAPI_MODIFIER& item_modifier { QStringToWinApiKbModifier(item->text()) };

    if(item->isCheckable() && item_modifier != WINMOD_NULLMOD)  {
        emit ModifierItemCheckStateChanged(item_modifier, item->checkState());
        emit ModifierBitmaskChanged(GetModifierCheckStateAsBitmask());
    }
}

QStandardItem* QKbModifierList::ItemAt(const quint32& index) const {
    return standardItemModel->item(index);
}

bool QKbModifierList::event(QEvent* event) {
    if(event->type() != QEvent::Wheel) {
        return QComboBox::event(event);
    } else {
        event->accept();
        return true;
    }
}

void QKbModifierList::InsertCheckableItem(const quint32& index, const QString& label, const Qt::CheckState& initial_state) {
    QStandardItem* new_checkable_item { new QStandardItem };

    new_checkable_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    new_checkable_item->setData(initial_state, Qt::CheckStateRole);
    new_checkable_item->setText(label);

    standardItemModel->insertRow(index, new_checkable_item);
}

void QKbModifierList::AddCheckableItem(const QString& label, const Qt::CheckState& initial_state) {
    InsertCheckableItem(count(), label, initial_state);
}

void QKbModifierList::AddModifierItem(const WINAPI_MODIFIER& modifier) {
    AddCheckableItem(WinApiKbModifierToQString(modifier, true));
    qDebug() << "AddModifierItem called, passing modifier to AddCheckableItem with conversion: " << WinApiKbModifierToQString(modifier);
}

quint32 QKbModifierList::GetModifierCheckStateAsBitmask() const {
    quint32 modifier_bitmask { NULL };

    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        const WINAPI_MODIFIER& modifier {
            QStringToWinApiKbModifier(item->text())
        };

        if(item->isCheckable() && modifier != WINMOD_NULLMOD) {
            if(item->checkState() == Qt::Checked) {
                modifier_bitmask |= modifier;
            }
        }
    }

    return modifier_bitmask;
}

void QKbModifierList::SetModifierCheckStateFromBitmask(const quint32& bitmask) {
    blockSignals(true);

    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        const WINAPI_MODIFIER& item_modifier {
            QStringToWinApiKbModifier(item->text())
        };

        if(item->isCheckable() && item_modifier != WINMOD_NULLMOD) {
            item->setCheckState(bitmask & item_modifier ? Qt::Checked : Qt::Unchecked);
        }
    }

    blockSignals(false);
}

void QKbModifierList::AddItemsFromBitmask(const quint32& bitmask) {
    for(const WINAPI_MODIFIER& modifier : WINAPI_MODIFIER_QLIST) {
        if(modifier & bitmask) {
            AddModifierItem(modifier);
        }
    }
}

QKbModifierList::QKbModifierList(const quint32& enabled_modifiers_bitmask) {
    for(const auto& modifier : WINAPI_MODIFIER_QLIST) {
        if(modifier & enabled_modifiers_bitmask) {
            AddModifierItem(modifier);
        }
    }
}

QKbModifierList::QKbModifierList(QWidget* parent)
    :
      QComboBox { parent },
      standardItemModel { new QStandardItemModel }
{
    connect(standardItemModel,    SIGNAL(itemChanged(QStandardItem*)),
            this,                 SLOT(handleItemDataChanged(QStandardItem*)));

    setModel(standardItemModel);
}
