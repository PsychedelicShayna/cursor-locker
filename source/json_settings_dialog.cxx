#include "json_settings_dialog.hxx"
#include "ui_json_settings_dialog.h"

const QMap<qint32, QString> JsonSettingsDialog::ActivationMethodResolverITOS = {
    { 0, ""       },
    { 1, "hotkey" },
    { 2, "image"  },
    { 3, "title"  }
};

const QMap<QString, qint32> JsonSettingsDialog::ActivationMethodResolverSTOI = {
    { ""      , 0 },
    { "hotkey", 1 },
    { "image" , 2 },
    { "title" , 3 }
};

qsizetype JsonSettingsDialog::JsonSettings::LoadFromFile(const QString& path, QWidget* calling_widget) {
    QFile json_file { path };

    if(!json_file.open(QFile::ReadOnly | QFile::Text)) {
        if(calling_widget != nullptr) QMessageBox::critical(calling_widget, "JSON I/O Error!", "Failed to load values from JSON file! Are you sure the program has permission to read from its location? File path: \"" + path + "\"");
        return -1;
    }

    const QByteArray&       json_file_bytes    { json_file.readAll()                      };
    const QJsonDocument&    json_document      { QJsonDocument::fromJson(json_file_bytes) };
    const QJsonObject&      json_object        { json_document.object()                   };

    const static QString& json_typeerror_title       { "JSON Value Type Error!" };
    const static QString& json_typeerror_message     { "The type of the JSON value \"%1\" isn't a %2! Re-saving with new values should correct this issue."};

    const static QString& json_keyerror_title        { "JSON Key Error!" };
    const static QString& json_keyerror_message      { "The \"%1\" key isn't present in the JSON configuration file, re-saving with new values should correct this issue." };

    const static QString& json_unknownkey_title      { "Unknown JSON Key!" };
    const static QString& json_unknownkey_message    { "The JSON key \"%1\" is unknown and cannot be handled, re-saving should remove this key from the JSON file." };

    const static QString& json_valueerror_title      { "JSON Value Error!" };
    const static QString& json_valueerror_message    { "The value of the JSON key \"%1\" has an invalid value: \"%2\"; %3 Re-saving with new values should correct this issue." };

    typedef std::tuple<QString, QString, bool(QJsonValue::*)() const, std::function<void(const QJsonValue&)>> JsonKeyHandlerTuple_t;

    const auto& apply_handlers_to_object {
        [&] (const QList<JsonKeyHandlerTuple_t>& json_key_handlers, const QJsonObject& json_object) -> void {
            QList<const JsonKeyHandlerTuple_t*> used_key_handlers;

            // Iterate over all key strings in json_object
            for(const QString& object_key : json_object.keys()) {
                const QJsonValue& object_value { json_object[object_key] };    // Get the value of the current object_key, from the json_object.

                // Find a JSON key handler (tuple) that accepts object_key.
                const auto matching_handler {
                    std::find_if(json_key_handlers.begin(), json_key_handlers.end(), [&](const auto& handler) -> bool {
                        return object_key == std::get<0>(handler);    // Check if first value of the tuple matches object_key.
                    })
                };

                // A handler accepting object_key has been found.
                if(matching_handler != json_key_handlers.end()) {
                    used_key_handlers.emplace_back(&(*matching_handler));

                    // Get the type checker functor, and value handler functor from the matching_handler tuple.
                    const auto& mfptr_type_checker   { std::get<2>(*matching_handler) };
                    const auto& lambda_value_handler { std::get<3>(*matching_handler) };

                    // Get the expected type as a string, for use in error messages if the value doesn't pass the type check.
                    const QString& expected_type_name { std::get<1>(*matching_handler) };


                    // Check if object_value is of the required type for lambda_value_handler.
                    if(std::bind(mfptr_type_checker, object_value)()) {
                        lambda_value_handler(object_value);    // Let lambda_value_handler determine what to further do with the value.
                    }

                    // The call to mfptr_type_checker returned false, so object_value is of a type incompatible with lambda_value_handler.
                    else {
                        if(calling_widget != nullptr) QMessageBox::warning(calling_widget, json_typeerror_title, json_typeerror_message.arg(object_key, expected_type_name));
                    }
                }

                // No matching handler was found, the object_key is unknown.
                else {
                    if(calling_widget != nullptr) QMessageBox::warning(calling_widget, json_unknownkey_title, json_unknownkey_message.arg(object_key));
                }
            }

            // Iterate through json_key_handlers to ensure that all handlers were utilized, meaning all expected JSON keys were accounted for.
            for(const JsonKeyHandlerTuple_t& key_handler : json_key_handlers) {
                // Show a warning if a key wasn't handled and is therefore missing from the JSON file.
                if(!used_key_handlers.contains(&key_handler)) {
                    if(calling_widget != nullptr) QMessageBox::warning(calling_widget, json_keyerror_title, json_keyerror_message.arg(std::get<0>(key_handler)));
                }
            }
        }
    };

    const QList<JsonKeyHandlerTuple_t>& json_key_handler_tuples {
        {"image", "string", &QJsonValue::isString, [&](const QJsonValue& value) -> void {
                ProcessImageName = value.toString();
            }},

        {"title", "string", &QJsonValue::isString, [&](const QJsonValue& value) -> void {
                ForegroundWindowTitle = value.toString();
            }},

        {"method", "string", &QJsonValue::isString, [&](const QJsonValue& value) -> void {
                static const QStringList valid_values { "", "title", "image", "hotkey" };
                const QString& value_string { value.toString() };

                if(valid_values.contains(value_string)) {
                    ActivationMethod = value_string;
                } else {
                    if(calling_widget != nullptr) QMessageBox::warning(calling_widget, json_valueerror_title, json_valueerror_message.arg("method", value_string, "value must be one of: \"title\", \"image\", \"hotkey\". "));
                }
            }},

        {"stylesheet_path", "string", &QJsonValue::isString, [&](const QJsonValue& value) -> void {
                StylesheetPath = value.toString();
            }},

        {"mute", "boolean", &QJsonValue::isBool, [&](const QJsonValue& value) -> void {
                InitialMuteState = value.toBool();
            }},

        {"shortcut", "object", &QJsonValue::isObject, [&](const QJsonValue& value) -> void {
                const QList<JsonKeyHandlerTuple_t>& shortcut_key_handler_tuples {
                    {"vkid", "string", &QJsonValue::isString, [&](const QJsonValue& value) -> void {
                            const QString& vkid_string { value.toString() };

                            if(vkid_string.size()) {
                                bool success { false };
                                const quint32& vkid { vkid_string.toUInt(&success, 16) };

                                if(success) {
                                    HotkeyVkid = QString { "0x%1" }.arg(vkid, 2, 16, QChar { '0' });
                                } else {
                                    if(calling_widget != nullptr) QMessageBox::warning(calling_widget, json_valueerror_title, json_valueerror_message.arg("shortcut/vkid", vkid_string, "cannot interpret value as a hexadecimal string, are you sure it's valid hexadecimal?"));
                                }
                            }
                        }},

                    {"modifier_alt", "boolean", &QJsonValue::isBool, [&](const QJsonValue& value) -> void {
                            HotkeyModifierBitmask |= (WINMOD_ALT * value.toBool());
                        }},

                    {"modifier_control", "boolean", &QJsonValue::isBool, [&](const QJsonValue& value) -> void {
                            HotkeyModifierBitmask |= (WINMOD_CONTROL * value.toBool());
                        }},

                    {"modifier_shift", "boolean", &QJsonValue::isBool, [&](const QJsonValue& value) -> void {
                            HotkeyModifierBitmask |= (WINMOD_SHIFT * value.toBool());
                        }},

                    {"modifier_win", "boolean", &QJsonValue::isBool, [&](const QJsonValue& value) -> void {
                            HotkeyModifierBitmask |= (WINMOD_WIN * value.toBool());
                        }},
                };

                apply_handlers_to_object(shortcut_key_handler_tuples, value.toObject());
            }},
    };

    apply_handlers_to_object(json_key_handler_tuples, json_object);

    return json_file_bytes.size();
}

qsizetype JsonSettingsDialog::JsonSettings::SaveToFile(const QString& path, QWidget* calling_widget) const {
    QFile json_file { path };

    if(!json_file.open(QFile::WriteOnly | QFile::Text)) {
        if(calling_widget != nullptr) QMessageBox::critical(calling_widget, "JSON I/O Error!", "Failed to open JSON file for writing! Are you sure the program has permission to write to its location? File path: \"" + path + "\"");
        return -1;
    }

    QJsonObject json_object;

    json_object["image"]  = ProcessImageName;
    json_object["title"]  = ForegroundWindowTitle;
    json_object["mute"]   = InitialMuteState;
    json_object["method"] = ActivationMethod;

    json_object["stylesheet_path"] = StylesheetPath;

    QJsonObject shortcut_object;

    shortcut_object["vkid"]               = HotkeyVkid;
    shortcut_object["modifier_alt"]       = static_cast<bool>(HotkeyModifierBitmask & WINMOD_ALT);
    shortcut_object["modifier_control"]   = static_cast<bool>(HotkeyModifierBitmask & WINMOD_CONTROL);
    shortcut_object["modifier_shift"]     = static_cast<bool>(HotkeyModifierBitmask & WINMOD_SHIFT);
    shortcut_object["modifier_win"]       = static_cast<bool>(HotkeyModifierBitmask & WINMOD_WIN);

    json_object["shortcut"] = shortcut_object;

    QJsonDocument    json_document    { json_object                                               };
    QByteArray       json_bytes       { json_document.toJson(QJsonDocument::JsonFormat::Indented) };

    qint64 bytes_written { json_file.write(json_bytes) };
    json_file.close();

    if(bytes_written != json_bytes.size()) {
        if(calling_widget != nullptr) QMessageBox::critical(calling_widget, "JSON I/O Error!", "The amount of bytes written to the file doesn't correspond to the size of the JSON data. Are you sure the program has permission to write to its location? File path: \"" + path + "\"");
        return -2;
    }

    return bytes_written;
}

JsonSettingsDialog::JsonSettings::JsonSettings()
    :
      HotkeyModifierBitmask    { NULL  },
      InitialMuteState         { false }
{

}

void JsonSettingsDialog::loadUiSettingsFromJsonFile() {
    JsonSettings json_settings;

    const qsizetype& bytes_read { json_settings.LoadFromFile(jsonConfigFilePath, this) };

    if(bytes_read > 0) {
        ui->leditForegroundWindowTitle->setText(json_settings.ForegroundWindowTitle);
        ui->leditProcessImageName->setText(json_settings.ProcessImageName);
        ui->leditKeyboardShortcut->setText(json_settings.HotkeyVkid);
        ui->cbMuted->setChecked(json_settings.InitialMuteState);
        ui->leditStylesheetPath->setText(json_settings.StylesheetPath);

        hotkeyModifierList->SetModifierCheckStateFromBitmask(json_settings.HotkeyModifierBitmask);

        if(ActivationMethodResolverSTOI.contains(json_settings.ActivationMethod)) {
            ui->cbxActivationMethod->setCurrentIndex(ActivationMethodResolverSTOI[json_settings.ActivationMethod]);
        }
    }
}

void JsonSettingsDialog::saveUiSettingsToJsonFile() {
    JsonSettings json_settings;
    json_settings.ProcessImageName = ui->leditProcessImageName->text();
    json_settings.ForegroundWindowTitle = ui->leditForegroundWindowTitle->text();
    json_settings.InitialMuteState = ui->cbMuted->isChecked();

    const qint32& activation_method_index { ui->cbxActivationMethod->currentIndex() };

    if(ActivationMethodResolverITOS.contains(activation_method_index)) {
        json_settings.ActivationMethod = ActivationMethodResolverITOS[activation_method_index];
    } else {
        QMessageBox::critical(this, "Value Resolution Error!", "Activation method index could not be resolved to a string! This shouldn't be possible, please report this error to the developer!");
        return;
    }

    json_settings.StylesheetPath = ui->leditStylesheetPath->text();

    QJsonObject shortcut_object;

    const QString& vkid_string { ui->leditKeyboardShortcut->text() };

    if(vkid_string.size()) {
        bool conversion_success { false };
        vkid_string.toUInt(&conversion_success, 16);

        if(conversion_success) {
            json_settings.HotkeyVkid = vkid_string;
        } else {
            QMessageBox::critical(this, "JSON Value Error!", "Cannot convert the VKID field from a hexadecimal string to an integer. Cannot continue saving. Are you sure it's valid hexadecimal?");
            return;
        }
    }

    json_settings.HotkeyModifierBitmask = hotkeyModifierList->GetModifierCheckStateAsBitmask();

    const qsizetype& bytes_written { json_settings.SaveToFile(jsonConfigFilePath, this) };

    if(bytes_written > 0) {
        close();
    }
}

JsonSettingsDialog::JsonSettingsDialog(const QString& json_config_file_path, QWidget* parent)
    :
      QDialog               { parent                       },
      ui                    { new Ui::JsonSettingsDialog   },
      jsonConfigFilePath    { json_config_file_path        },
      hotkeyModifierList    { new QKbModifierList { this } }
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowFlags(
            Qt::Dialog
            | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            );

    HWND desktop_window_handle { GetDesktopWindow() };
    RECT desktop_window_rect;

    GetWindowRect(desktop_window_handle, &desktop_window_rect);
    CloseHandle(desktop_window_handle);

    hotkeyModifierList->addItem("Modifiers");
    hotkeyModifierList->AddItemsFromBitmask(WINMOD_CONTROL | WINMOD_ALT | WINMOD_SHIFT | WINMOD_WIN);
    hotkeyModifierList->setMinimumWidth(90);
    hotkeyModifierList->setMinimumHeight(25);
    ui->hlayoutKeyboardShortcut->insertWidget(0, hotkeyModifierList);

    connect(ui->btnCancel,          &QPushButton::clicked,
            this,                   &JsonSettingsDialog::close);

    connect(ui->btnSaveDefaults,    &QPushButton::clicked,
            this,                   &JsonSettingsDialog::saveUiSettingsToJsonFile);

    // QSS Stylesheet
    // ----------------------------------------------------------------------------------------------------
    connect(ui->leditForegroundWindowTitle,    &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->leditForegroundWindowTitle); });
    connect(ui->leditKeyboardShortcut,         &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->leditKeyboardShortcut);      });
    connect(ui->leditProcessImageName,         &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->leditProcessImageName);      });
    connect(ui->leditStylesheetPath,           &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->leditStylesheetPath);        });

    QFileInfo file_info { json_config_file_path };
    jsonConfigFilePath = file_info.absoluteFilePath();

    if(file_info.exists() && file_info.isFile()) {
        loadUiSettingsFromJsonFile();
    } else {
        QMessageBox::warning(this, "JSON I/O Error", "An existing JSON file could not be found, pressing save will attempt to generate a new one with the provided values. File path: \"" + jsonConfigFilePath + "\"");
    }

    setMaximumHeight(minimumHeight());
}

JsonSettingsDialog::~JsonSettingsDialog() {
    delete ui;
}
