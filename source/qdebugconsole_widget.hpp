#ifndef QDEBUGCONSOLE_HPP
#define QDEBUGCONSOLE_HPP

#include <QScrollBar>
#include <QTextEdit>
#include <QObject>
#include <QWidget>

class QDebugConsole : public QTextEdit {
Q_OBJECT

public:
    enum LOG_LEVEL {
        LL_INFO         = 0,
        LL_WARNING      = 1,
        LL_ERROR        = 2,
        LL_EXCEPTION    = 3
    } MINIMUM_LL = LL_INFO;

public slots:
    void InsertIntoTextEdit(const QPair<LOG_LEVEL, QString>);
    void RefreshAllLogMessages();

signals:
    void MessageWasLogged(const QPair<LOG_LEVEL, QString>);

private:
    QList<QPair<LOG_LEVEL, QString>> logMessages;

public:
    void log(const QString&,         LOG_LEVEL ll = LL_INFO);
    void log(const QList<QString>&,  LOG_LEVEL ll = LL_INFO);
    void log(const char*,            LOG_LEVEL ll = LL_INFO);

    QDebugConsole(QWidget* parent);
};

#endif // QDEBUGCONSOLE_HPP
