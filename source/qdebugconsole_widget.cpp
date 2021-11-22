#include "qdebugconsole_widget.hpp"

void QDebugConsole::InsertIntoTextEdit(const QPair<LOG_LEVEL, QString> mpair) {
    if(MINIMUM_LL > mpair.first) return;
    moveCursor(QTextCursor::End);

    static const QMap<LOG_LEVEL, QString>&     ll_resolver        {
        { LL_INFO,  "[INFO] "  },  { LL_WARNING,   "[WARNING] "   },
        { LL_ERROR, "[ERROR] " },  { LL_EXCEPTION, "[EXCEPTION] " }
    };

    const QString& ll_string {
        ll_resolver.contains(mpair.first) ?
                    ll_resolver[mpair.first] : "UNKNOWN" };

    insertPlainText(ll_string + mpair.second);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void QDebugConsole::RefreshAllLogMessages() {
    clear();

    for(auto& message : logMessages) {
        InsertIntoTextEdit(message);
    }
}

void QDebugConsole::log(const QString& message, LOG_LEVEL ll) {
    const QPair<LOG_LEVEL, QString>& mpair { ll, message + '\n' };
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

    connect(this,   SIGNAL(MessageWasLogged(const QPair<LOG_LEVEL, QString>)),
            this,   SLOT(InsertIntoTextEdit(const QPair<LOG_LEVEL, QString>)));
}
