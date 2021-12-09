#ifndef CHECKBOX_LIST_WIDGET_HXX
#define CHECKBOX_LIST_WIDGET_HXX

#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QListView>
#include <QtWidgets/QComboBox>

#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtGui/QtEvents>

#include "winapi_utilities.hpp"

class ExtendedQListView : public QListView {
Q_OBJECT

signals:
    void ModelItemClicked(QModelIndex);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* mouse_event) override;
};

class QKbModifierList : public QComboBox {
Q_OBJECT

signals:
    void ModifierItemCheckStateChanged(const WINAPI_MODIFIER& modifier, const Qt::CheckState& new_state);
    void ModifierBitmaskChanged(const quint32& bitmask);

protected slots:
    void handleItemDataChanged(QStandardItem* item);
    void handleModelItemClicked(QModelIndex index);

protected:
    ExtendedQListView* extendedQListView;
    QStandardItemModel* standardItemModel;

    virtual void wheelEvent(QWheelEvent* wheel_event) override;

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
