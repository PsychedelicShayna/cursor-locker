#include "vkid_table_widget_dialog.hxx"
#include "ui_vkid_table_widget_dialog.h"

VkidTableWidgetDialog::VkidTableWidgetDialog(QWidget* parent)
    :
      QDialog            { parent                        },
      ui                 { new Ui::VkidTableWidgetDialog },
      vkidTableWidget    { new VkidTableWidget { this }  }

{
    ui->setupUi(this);

    setWindowFlags(
                Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                | Qt::WindowMinimizeButtonHint
                | Qt::WindowMaximizeButtonHint
                );

    ui->verticalLayoutVkidTableWidgetPlaceholder->addLayout(vkidTableWidget);
    setAttribute(Qt::WA_DeleteOnClose);
}

VkidTableWidgetDialog::~VkidTableWidgetDialog() {
    delete ui;
}
