#ifndef CHECKBOX_LIST_WIDGET_HXX
#define CHECKBOX_LIST_WIDGET_HXX

#include <QtWidgets/QComboBox>

#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtGui/QtEvents>

class QCheckBoxList : public QComboBox {
Q_OBJECT
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

    QCheckBoxList(QWidget* parent);
    QCheckBoxList(QWidget* parent, const QString& header);
};

#endif // CHECKBOX_LIST_WIDGET_HXX
