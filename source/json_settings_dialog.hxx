#ifndef JSON_SETTINGS_DIALOG_HXX
#define JSON_SETTINGS_DIALOG_HXX

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>

#include <tuple>

#include "keyboard_modifier_list_widget.hpp"

namespace Ui {
    class JsonSettingsDialog;
}

class JsonSettingsDialog : public QDialog {
Q_OBJECT

public:
    struct JsonSettings {
        QString ForegroundWindowTitle;
        QString ProcessImageName;
        QString HotkeyVkid;
        QString ActivationMethod;
        QString StylesheetPath;
        quint32 HotkeyModifierBitmask;
        bool    InitialMuteState;

        qsizetype LoadFromFile(const QString& path, QWidget* calling_widget = nullptr);
        qsizetype SaveToFile(const QString& path, QWidget* calling_widget = nullptr) const;

        JsonSettings();
    };

    static const QMap<qint32, QString>    ActivationMethodResolverITOS;
    static const QMap<QString, qint32>    ActivationMethodResolverSTOI;

signals:
    void MuteStateChanged(const bool new_mute_state);

private:
    Ui::JsonSettingsDialog* ui;
    QString jsonConfigFilePath;

    QKbModifierList* hotkeyModifierList;

protected slots:
    void loadUiSettingsFromJsonFile();
    void saveUiSettingsToJsonFile();

public:
    explicit JsonSettingsDialog(const QString& json_config_file_path, QWidget* parent = nullptr);
    virtual ~JsonSettingsDialog() override;
};

#endif // JSON_SETTINGS_DIALOG_HXX
