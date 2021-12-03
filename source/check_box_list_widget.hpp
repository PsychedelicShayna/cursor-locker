#ifndef CHECKBOX_LIST_WIDGET_HXX
#define CHECKBOX_LIST_WIDGET_HXX

#include <QtWidgets/QComboBox>

#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtGui/QtEvents>

#include "winapi_utilities.hpp"

class QCheckBoxList : public QComboBox {
Q_OBJECT
signals:
    void hello();

protected:
    virtual bool event(QEvent*) override;

public:
    QStandardItemModel* StandardItemModel;
    QStandardItem* HeaderItem;

    void SetHeaderLabel(const QString& header);

    void InsertCheckableItem(const qint32& index, const QString& label, const Qt::CheckState& initial_checked_state = Qt::Checked);
    void AddCheckableItem(const QString& label, const Qt::CheckState& initial_checked_state = Qt::Checked);

    void AddCheckableItems(const QList<QPair<QString, Qt::CheckState>>& item_value_pairs);
    void AddCheckableItems(const QList<QString>& item_labels, const Qt::CheckState& initial_checked_state = Qt::Checked);

    QStandardItem* ItemAt(const quint32& index);
    QStandardItem* ItemWithLabel(const QString& label);
    QList<QStandardItem*> ItemsWithLabel(const QString& label);

    QCheckBoxList(QWidget* parent);
    QCheckBoxList(QWidget* parent, const QString& header);
};


class QKbModifierList : public QComboBox {
Q_OBJECT

signals:
    void ModifierItemCheckStateChanged(const WINAPI_MODIFIER& modifier, const Qt::CheckState& new_state);
    void ModifierBitmaskChanged(const quint32& bitmask);

protected slots:
    void handleItemDataChanged(QStandardItem* item);

protected:
    QStandardItemModel* standardItemModel;
    virtual bool event(QEvent* event) override;

public:
    QStandardItem* ItemAt(const quint32& index) const;

    void InsertCheckableItem(const quint32& index, const QString& label, const Qt::CheckState& initial_state);
    void AddCheckableItem(const QString& label, const Qt::CheckState& initial_state = Qt::Unchecked);
    void AddModifierItem(const WINAPI_MODIFIER& modifier);

    quint32 GetModifierCheckStateAsBitmask() const;
    void SetModifierCheckStateFromBitmask(const quint32& bitmask);

    void AddItemsFromBitmask(const quint32& bitmask);

    QKbModifierList(const quint32&);
    QKbModifierList(QWidget* parent);
};


#endif // CHECKBOX_LIST_WIDGET_HXX
