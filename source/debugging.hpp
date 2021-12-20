#ifndef DEBUGGING_HPP
#define DEBUGGING_HPP

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

#include <stdio.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>


#include <QtCore/QtDebug>
#include <QtCore/QString>
#include <QtCore/QDateTime>

namespace Debugging {
    extern bool CONSOLE_HAS_BEEN_ALLOCATED;
    extern bool LOG_FILE_HAS_BEEN_DELETED;

    errno_t SpawnDebugConsole();
    void DebugMessageHandler(QtMsgType message_type, const QMessageLogContext& message_context, const QString& message);
}


#endif // DEBUGGING_HPP
