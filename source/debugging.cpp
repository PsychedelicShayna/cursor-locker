#include "debugging.hpp"

bool Debugging::LOG_FILE_HAS_BEEN_DELETED     { false };
bool Debugging::CONSOLE_HAS_BEEN_ALLOCATED    { false };

errno_t Debugging::SpawnDebugConsole() {
    if(!CONSOLE_HAS_BEEN_ALLOCATED) {
        AllocConsole();

        FILE* console_stdout { nullptr };
        const errno_t& error_code { freopen_s(&console_stdout, "CONOUT$", "w", stdout) };

        if(error_code) return error_code;

        HANDLE console_handle { GetStdHandle(STD_OUTPUT_HANDLE) };
        COORD console_buffer_size { 300, 2000 };

        SetConsoleScreenBufferSize(console_handle, console_buffer_size);
        SetConsoleMode(console_handle, ENABLE_QUICK_EDIT_MODE);

        CONSOLE_HAS_BEEN_ALLOCATED = true;
    }

    return NULL;
}

void Debugging::DebugMessageHandler(QtMsgType message_type, const QMessageLogContext& message_context, const QString& message) {
    const QMap<QtMsgType, QString>& message_type_resolver {
        { QtMsgType::QtDebugMsg,       "DEBUG"    },
        { QtMsgType::QtWarningMsg,     "WARNING"  },
        { QtMsgType::QtCriticalMsg,    "CRITICAL" },
        { QtMsgType::QtFatalMsg,       "FATAL"    },
        { QtMsgType::QtInfoMsg,        "INFO"     },
    };

    const QString& message_type_string { message_type_resolver.contains(message_type) ? message_type_resolver[message_type] : "UNKNOWN" };

    const QString& formatted_message { QString { "(%1/%2) [%3] @ %4:%5 -- %6\n%7\n\n" }
        .arg(message_type_string)
        .arg(message_context.category)
        .arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs))
        .arg(message_context.file)
        .arg(message_context.line)
        .arg(message_context.function)
        .arg(message)
    };

    QFileInfo log_file_info { QCoreApplication::applicationFilePath() + ".log" };
    QFile log_file { log_file_info.absoluteFilePath() };

    QFile::OpenMode open_mode_flags { QFile::OpenModeFlag::Text };

    if(!LOG_FILE_HAS_BEEN_DELETED && log_file_info.exists() && log_file_info.isFile()) {
        log_file.remove();
        open_mode_flags |= QFile::OpenModeFlag::WriteOnly;
        LOG_FILE_HAS_BEEN_DELETED = true;
    } else {
        open_mode_flags |= QFile::OpenModeFlag::Append;
    }

    if(log_file.open(open_mode_flags)) {
        log_file.write(formatted_message.toUtf8());
        log_file.close();
    }

    if(CONSOLE_HAS_BEEN_ALLOCATED) {
        fprintf(stdout, "%s", formatted_message.toStdString().c_str());
    }
}
