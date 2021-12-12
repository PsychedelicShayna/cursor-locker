#include "vkid_table_dialog.hxx"
#include "ui_vkid_table_dialog.h"

const QList<QPair<QString, QString>>& VkidTableDialog::MsdnVkidTable = {
    { "Left Mouse Button",                      "0x01" },
    { "Right Mouse Button",                     "0x02" },
    { "Control-break Processing",               "0x03" },
    { "Middle Mouse Button",                    "0x04" },
    { "X1 Mouse Button",                        "0x05" },
    { "X2 Mouse Button",                        "0x06" },
    { "BACKSPACE Key",                          "0x08" },
    { "TAB Key",                                "0x09" },
    { "CLEAR Key",                              "0x0C" },
    { "ENTER Key",                              "0x0D" },
    { "SHIFT Key",                              "0x10" },
    { "CTRL Key",                               "0x11" },
    { "ALT Key",                                "0x12" },
    { "PAUSE Key",                              "0x13" },
    { "CAPS LOCK Key",                          "0x14" },
    { "IME Kana Mode",                          "0x15" },
    { "IME Hangul Mode",                        "0x15" },
    { "IME On",                                 "0x16" },
    { "IME Junja Mode",                         "0x17" },
    { "IME Final Mode",                         "0x18" },
    { "IME Hanja Mode",                         "0x19" },
    { "IME Kanji Mode",                         "0x19" },
    { "IME Off",                                "0x1A" },
    { "ESC Key",                                "0x1B" },
    { "IME Convert",                            "0x1C" },
    { "IME Nonconvert",                         "0x1D" },
    { "IME Accept",                             "0x1E" },
    { "IME Mode Change Request",                "0x1F" },
    { "SPACEBAR",                               "0x20" },
    { "PAGE UP Key",                            "0x21" },
    { "PAGE DOWN Key",                          "0x22" },
    { "END Key",                                "0x23" },
    { "HOME Key",                               "0x24" },
    { "LEFT ARROW Key",                         "0x25" },
    { "UP ARROW Key",                           "0x26" },
    { "RIGHT ARROW Key",                        "0x27" },
    { "DOWN ARROW Key",                         "0x28" },
    { "SELECT Key",                             "0x29" },
    { "PRINT Key",                              "0x2A" },
    { "EXECUTE Key",                            "0x2B" },
    { "PRINT SCREEN Key",                       "0x2C" },
    { "INS Key",                                "0x2D" },
    { "DEL Key",                                "0x2E" },
    { "HELP Key",                               "0x2F" },
    { "0 Key",                                  "0x30" },
    { "1 Key",                                  "0x31" },
    { "2 Key",                                  "0x32" },
    { "3 Key",                                  "0x33" },
    { "4 Key",                                  "0x34" },
    { "5 Key",                                  "0x35" },
    { "6 Key",                                  "0x36" },
    { "7 Key",                                  "0x37" },
    { "8 Key",                                  "0x38" },
    { "9 Key",                                  "0x39" },
    { "A Key",                                  "0x41" },
    { "B Key",                                  "0x42" },
    { "C Key",                                  "0x43" },
    { "D Key",                                  "0x44" },
    { "E Key",                                  "0x45" },
    { "F Key",                                  "0x46" },
    { "G Key",                                  "0x47" },
    { "H Key",                                  "0x48" },
    { "I Key",                                  "0x49" },
    { "J Key",                                  "0x4A" },
    { "K Key",                                  "0x4B" },
    { "L Key",                                  "0x4C" },
    { "M Key",                                  "0x4D" },
    { "N Key",                                  "0x4E" },
    { "O Key",                                  "0x4F" },
    { "P Key",                                  "0x50" },
    { "Q Key",                                  "0x51" },
    { "R Key",                                  "0x52" },
    { "S Key",                                  "0x53" },
    { "T Key",                                  "0x54" },
    { "U Key",                                  "0x55" },
    { "V Key",                                  "0x56" },
    { "W Key",                                  "0x57" },
    { "X Key",                                  "0x58" },
    { "Y Key",                                  "0x59" },
    { "Z Key",                                  "0x5A" },
    { "Left Windows Key",                       "0x5B" },
    { "Right Windows Key",                      "0x5C" },
    { "Applications Key",                       "0x5D" },
    { "Computer Sleep Key",                     "0x5F" },
    { "Numeric Keypad 0 Key",                   "0x60" },
    { "Numeric Keypad 1 Key",                   "0x61" },
    { "Numeric Keypad 2 Key",                   "0x62" },
    { "Numeric Keypad 3 Key",                   "0x63" },
    { "Numeric Keypad 4 Key",                   "0x64" },
    { "Numeric Keypad 5 Key",                   "0x65" },
    { "Numeric Keypad 6 Key",                   "0x66" },
    { "Numeric Keypad 7 Key",                   "0x67" },
    { "Numeric Keypad 8 Key",                   "0x68" },
    { "Numeric Keypad 9 Key",                   "0x69" },
    { "Multiply Key",                           "0x6A" },
    { "Add Key",                                "0x6B" },
    { "Separator Key",                          "0x6C" },
    { "Subtract Key",                           "0x6D" },
    { "Decimal Key",                            "0x6E" },
    { "Divide Key",                             "0x6F" },
    { "F1 Key",                                 "0x70" },
    { "F2 Key",                                 "0x71" },
    { "F3 Key",                                 "0x72" },
    { "F4 Key",                                 "0x73" },
    { "F5 Key",                                 "0x74" },
    { "F6 Key",                                 "0x75" },
    { "F7 Key",                                 "0x76" },
    { "F8 Key",                                 "0x77" },
    { "F9 Key",                                 "0x78" },
    { "F10 Key",                                "0x79" },
    { "F11 Key",                                "0x7A" },
    { "F12 Key",                                "0x7B" },
    { "F13 Key",                                "0x7C" },
    { "F14 Key",                                "0x7D" },
    { "F15 Key",                                "0x7E" },
    { "F16 Key",                                "0x7F" },
    { "F17 Key",                                "0x80" },
    { "F18 Key",                                "0x81" },
    { "F19 Key",                                "0x82" },
    { "F20 Key",                                "0x83" },
    { "F21 Key",                                "0x84" },
    { "F22 Key",                                "0x85" },
    { "F23 Key",                                "0x86" },
    { "F24 Key",                                "0x87" },
    { "NUM LOCK Key",                           "0x90" },
    { "SCROLL LOCK Key",                        "0x91" },
    { "Left SHIFT Key",                         "0xA0" },
    { "Right SHIFT Key",                        "0xA1" },
    { "Left CONTROL Key",                       "0xA2" },
    { "Right CONTROL Key",                      "0xA3" },
    { "Left MENU Key",                          "0xA4" },
    { "Right MENU Key",                         "0xA5" },
    { "Browser Back Key",                       "0xA6" },
    { "Browser Forward Key",                    "0xA7" },
    { "Browser Refresh Key",                    "0xA8" },
    { "Browser Stop Key",                       "0xA9" },
    { "Browser Search Key",                     "0xAA" },
    { "Browser Favorites Key",                  "0xAB" },
    { "Browser Start And Home Key",             "0xAC" },
    { "Volume Mute Key",                        "0xAD" },
    { "Volume Down Key",                        "0xAE" },
    { "Volume Up Key",                          "0xAF" },
    { "Next Track Key",                         "0xB0" },
    { "Previous Track Key",                     "0xB1" },
    { "Stop Media Key",                         "0xB2" },
    { "Play/Pause Media Key",                   "0xB3" },
    { "Start Mail Key",                         "0xB4" },
    { "Select Media Key",                       "0xB5" },
    { "Start Application 1 Key",                "0xB6" },
    { "Start Application 2 Key",                "0xB7" },
    { "For US Keyboards, The ';:' Key",         "0xBA" },
    { "For Any Country/region, The '+' Key",    "0xBB" },
    { "For Any Country/region, The ',' Key",    "0xBC" },
    { "For Any Country/region, The '-' Key",    "0xBD" },
    { "For Any Country/region, The '.' Key",    "0xBE" },
    { "For US Keyboards, The '/?' Key",         "0xBF" },
    { "For US Keyboards, The '`~' Key",         "0xC0" },
    { "For US Keyboards, The '[{' Key",         "0xDB" },
    { "For US Keyboards, The '\\|' Key",        "0xDC" },
    { "For US Keyboards, The ']}' Key",         "0xDD" },
    { "For US Keyboards, The Quote (\"/') Key", "0xDE" },
    { "IME PROCESS Key",                        "0xE5" },
    { "Attn Key",                               "0xF6" },
    { "CrSel Key",                              "0xF7" },
    { "ExSel Key",                              "0xF8" },
    { "Erase EOF Key",                          "0xF9" },
    { "Play Key",                               "0xFA" },
    { "Zoom Key",                               "0xFB" },
    { "PA1 Key",                                "0xFD" },
    { "Clear Key",                              "0xFE" }
};

void VkidTableDialog::applySearchFilter() {
    const QString& search_term { ui->leditSearchFilter->text() };

    for(qint32 i { 0 }; i < ui->tablewVkidTable->rowCount(); ui->tablewVkidTable->setRowHidden(i++, false));

    if(search_term.size()) {
        for(qint32 i { 0 }; i < ui->tablewVkidTable->rowCount(); ++i) {
            const QString& row_description    { ui->tablewVkidTable->item(i, 0)->text() };
            const QString& row_vkid           { ui->tablewVkidTable->item(i, 1)->text() };

            if(!(row_description.contains(search_term, Qt::CaseInsensitive) || row_vkid.contains(search_term, Qt::CaseInsensitive))) {
                ui->tablewVkidTable->setRowHidden(i, true);
            }
        }
    }
}

void VkidTableDialog::copyItemVkid(QTableWidgetItem* item) {
    const qint32& item_row { ui->tablewVkidTable->row(item) };
    const QString& vkid { ui->tablewVkidTable->item(item_row, 1)->text() };
    QApplication::clipboard()->setText(vkid, QClipboard::Mode::Clipboard);
}

VkidTableDialog::VkidTableDialog(QWidget* parent)
    :
      QDialog    { parent                  },
      ui         { new Ui::VkidTableDialog }
{
    ui->setupUi(this);

    setWindowFlags(
                Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                );

    setAttribute(Qt::WA_DeleteOnClose);

    ui->tablewVkidTable->setRowCount(static_cast<quint32>(MsdnVkidTable.size()));
    ui->tablewVkidTable->horizontalHeader()->resizeSection(0, 250);
    ui->tablewVkidTable->horizontalHeader()->setStretchLastSection(true);
    ui->tablewVkidTable->verticalHeader()->setHidden(true);

    quint32 current_row { 0 };

    for(const QPair<QString, QString>& vkid_desc_pair : MsdnVkidTable) {
        const QString& vkid_description    { vkid_desc_pair.first  };
        const QString& vkid_string         { vkid_desc_pair.second };

        ui->tablewVkidTable->setItem(current_row, 0, new QTableWidgetItem { vkid_description });
        ui->tablewVkidTable->setItem(current_row, 1, new QTableWidgetItem { vkid_string      });

        ++current_row;
    }

    connect(ui->tablewVkidTable,      SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this,                     SLOT(copyItemVkid(QTableWidgetItem*)));

    connect(ui->leditSearchFilter,    SIGNAL(textEdited(const QString&)),
            this,                     SLOT(applySearchFilter()));

    // QSS Stylesheet
    // ----------------------------------------------------------------------------------------------------
    connect(ui->leditSearchFilter, &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->leditSearchFilter); });
}

VkidTableDialog::~VkidTableDialog() {
    delete ui;
}
