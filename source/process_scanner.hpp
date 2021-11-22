#ifndef PROCESSSCANNER_HPP
#define PROCESSSCANNER_HPP

#include <QObject>
#include <QTreeWidgetItem>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

class ProcessScanner : public QObject {
Q_OBJECT

signals:
    void ScanStarted();
    void RootItemReady(QTreeWidgetItem*);
    void ScanFinished();

private:
    QString getHwndHash(HWND);

public slots:
    void PerformScan();
};

#endif // PROCESSSCANNER_HPP
