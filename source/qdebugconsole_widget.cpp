#include "qdebugconsole_widget.hpp"

void QDebugConsole::log(const QString& message, LOG_LEVEL ll) {
    const QPair<QPair<LOG_LEVEL, QString>, QString>& mpair { { ll, ConsoleContext }, message + '\n' };
    LogMessages.append(mpair);
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

void QDebugConsole::InsertIntoTextEdit(const QPair<QPair<LOG_LEVEL, QString>, QString> mpair) {
    const LOG_LEVEL& log_level { mpair.first.first };
    const QString& context { mpair.first.second };
    const QString& message { mpair.second };

    if(MinimumLogLevel > log_level) return;
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

    for(auto& message : LogMessages) {
        InsertIntoTextEdit(message);
    }
}

void QDebugConsole::ClearConsole() {
    LogMessages.clear();
    clear();
}

void QDebugConsole::HandleContextMenuActionTriggered(QAction* action) {
    if(action->parentWidget() == cmSubmenuLogLevels) {
        bool success { false };

        const LOG_LEVEL& new_ll  {
            static_cast<LOG_LEVEL>(action->data().toInt(&success))
        };

        if(success) {
            MinimumLogLevel = new_ll;
            RefreshAllLogMessages();

            for(auto iter_action : cmSubmenuLogLevels->actions()) {
                iter_action->setChecked(false);
            }

            action->setCheckable(true);
            action->setChecked(true);
        }
    }
}

void QDebugConsole::HandleContextMenuRequested(const QPoint& point) {
    const QPoint& global_point { mapToGlobal(point) };
    contextMenu->popup(global_point);
}

QDebugConsole::QDebugConsole(QWidget* parent)
    :
      QTextEdit { parent },

      // Console context menu initialization.
      contextMenu             { new QMenu { this } },
      cmActionClearConsole    { nullptr            },
      cmSubmenuLogLevels      { nullptr            }
{
    setReadOnly(true);

    cmActionClearConsole = contextMenu->addAction("Clear Console");

    cmSubmenuLogLevels = contextMenu->addMenu("Log Levels");
    cmSubmenuLogLevels->addAction(">= INFO")->setData(LL_INFO);
    cmSubmenuLogLevels->addAction(">= WARNING")->setData(LL_WARNING);
    cmSubmenuLogLevels->addAction(">= ERROR")->setData(LL_ERROR);
    cmSubmenuLogLevels->addAction(">= EXCEPTION")->setData(LL_EXCEPTION);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this,                    SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                    SLOT(HandleContextMenuRequested(const QPoint&)));

    connect(contextMenu,             SIGNAL(triggered(QAction*)),
            this,                    SLOT(HandleContextMenuActionTriggered(QAction*)));

    connect(cmActionClearConsole,    SIGNAL(triggered()),
            this,                    SLOT(ClearConsole()));

    connect(this,                    SIGNAL(MessageWasLogged(const QPair<QPair<LOG_LEVEL, QString>, QString>)),
            this,                    SLOT(InsertIntoTextEdit(const QPair<QPair<LOG_LEVEL, QString>, QString>)));
}

QDebugConsoleContext::QDebugConsoleContext(QDebugConsole* console, const QString& console_context)
    :
      consoleContext { console_context },
      ConsoleContext { consoleContext  },
      Console        { console         }
{
    Console->ConsoleContext = console_context;
}

QDebugConsoleContext::~QDebugConsoleContext() {
    Console->ConsoleContext.clear();
}
