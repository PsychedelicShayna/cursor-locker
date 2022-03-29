#ifndef VKID_TABLE_WIDGET_DIALOG_HXX
#define VKID_TABLE_WIDGET_DIALOG_HXX

#include <QtWidgets/QDialog>

#include <vkid_table_widget.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class VkidTableWidgetDialog; }
QT_END_NAMESPACE

class VkidTableWidgetDialog : public QDialog {
Q_OBJECT
protected:
    Ui::VkidTableWidgetDialog*    ui;
    VkidTableWidget*              vkidTableWidget;

public:
    explicit VkidTableWidgetDialog(QWidget* parent = nullptr);
    virtual ~VkidTableWidgetDialog() override;
};

#endif // VKID_TABLE_WIDGET_DIALOG_HXX
