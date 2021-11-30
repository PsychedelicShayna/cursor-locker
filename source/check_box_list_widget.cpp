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

QCheckBoxList::QCheckBoxList(QWidget* parent)
    :
      QComboBox            { parent                          },
      StandardItemModel    { new QStandardItemModel { this } },
      HeaderItem           { new QStandardItem               }
{
    setModel(StandardItemModel);
}
