#ifndef VKID_TABLE_DIALOG_HXX
#define VKID_TABLE_DIALOG_HXX

#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QDialog>

#include <QtGui/QClipboard>

namespace Ui {
    class VkidTableDialog;
}

class VkidTableDialog : public QDialog {
Q_OBJECT

private:
    Ui::VkidTableDialog* ui;

protected slots:
    void applySearchFilter();

    void copyItemVkid(QTableWidgetItem*);

public:
    static const QList<QPair<QString, QString>>& MsdnVkidTable;

    explicit VkidTableDialog(QWidget* parent = nullptr);
    virtual ~VkidTableDialog() override;
};

#endif // VKID_TABLE_DIALOG_HXX
