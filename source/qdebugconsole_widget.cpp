#include "qdebugconsole_widget.hpp"

void QDebugConsole::InsertIntoTextEdit(const QPair<QPair<LOG_LEVEL, QString>, QString> mpair) {
    const LOG_LEVEL& log_level { mpair.first.first };
    const QString& context { mpair.first.second };
    const QString& message { mpair.second };

    if(MINIMUM_LL > log_level) return;
    moveCursor(QTextCursor::End);

    static const QMap<LOG_LEVEL, QString>&     ll_resolver        {
        { LL_INFO,  "[INFO] "  },  { LL_WARNING,   "[WARNING] "   },
        { LL_ERROR, "[ERROR] " },  { LL_EXCEPTION, "[EXCEPTION] " }
    };

    const QString& ll_string {
        ll_resolver.contains(log_level) ?
                    ll_resolver[log_level] : "UNKNOWN" };

    insertPlainText((context.size() ? ll_string + "(" + context + ") " : ll_string) + message);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void QDebugConsole::RefreshAllLogMessages() {
    clear();

    for(auto& message : logMessages) {
        InsertIntoTextEdit(message);
    }
}

void QDebugConsole::SetContext(const QString& context) {
    consoleContext = context;
}

void QDebugConsole::ClearContext() {
    consoleContext.clear();
}

void QDebugConsole::log(const QString& message, LOG_LEVEL ll) {
    const QPair<QPair<LOG_LEVEL, QString>, QString>& mpair { { ll, consoleContext }, message + '\n' };
    logMessages.append(mpair);
    emit MessageWasLogged(mpair);
}

void QDebugConsole::log(const QList<QString>& message_list, LOG_LEVEL ll) {
    QString message_concat;

    for(const auto& message : message_list) {
        message_concat += message;
    }

    log(message_concat, ll);
}

void QDebugConsole::log(const char* message, LOG_LEVEL ll) {
    log(QString { message }, ll);
}

QDebugConsole::QDebugConsole(QWidget* parent)
    :
      QTextEdit { parent }
{
    setReadOnly(true);

    connect(this,   SIGNAL(MessageWasLogged(const QPair<QPair<LOG_LEVEL, QString>, QString>)),
            this,   SLOT(InsertIntoTextEdit(const QPair<QPair<LOG_LEVEL, QString>, QString>)));
}

QDebugConsoleContext::QDebugConsoleContext(QDebugConsole* console, const QString& console_context)
    :
      consoleContext { console_context },
      ConsoleContext { consoleContext  },
      Console        { console         }
{
    Console->SetContext(console_context);
}

QDebugConsoleContext::~QDebugConsoleContext() {
    Console->ClearContext();
}
