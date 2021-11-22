#include "process_scanner.hpp"

QString ProcessScanner::getHwndHash(HWND hwnd) {
    uint8_t* hwnd_bytes { reinterpret_cast<uint8_t*>(&hwnd) };
    int32_t hwnd_size = { sizeof(hwnd) };

    uint8_t digest_buffer[4];
    ZeroMemory(digest_buffer, sizeof(digest_buffer));

    for(int32_t i { 0 }, c { hwnd_size - 1 }; i < hwnd_size && c >= 0; ++i, --c) {
        hwnd_bytes[c] ^= hwnd_bytes[i];
    }

    for(int32_t i { 0 }; i < static_cast<int32_t>(hwnd_size); ++i) {
        uint16_t& digest_short =  reinterpret_cast<uint16_t*>(digest_buffer)[(i % 2 == 0) ? 0 : 1];
        const uint8_t& hwnd_byte { hwnd_bytes[i] };

        digest_short ^= static_cast<uint16_t>(hwnd_byte) << 8;

        for(uint8_t c { 0 }; c < 8; ++c) {
            if(digest_short & 0x8000) {
                digest_short ^= (digest_short << 1) ^ 0x1021;
            } else {
                digest_short <<= 1;
            }
        }
    }

    for(int32_t i { 0 }, c { sizeof(digest_buffer) - 1 }; i < sizeof(digest_buffer) && c >= 0; ++i, --c) {
        digest_buffer[c] ^= digest_buffer[i];
    }

    QString hexdigest;

    for(size_t i { 0 }; i < sizeof(digest_buffer); ++i) {
        char hex_buffer[5];
        ZeroMemory(hex_buffer, sizeof(hex_buffer));
        sprintf(hex_buffer,"%02X", digest_buffer[i]);
        hexdigest += hex_buffer;
    }

    return hexdigest;
}

void ProcessScanner::PerformScan() {
    emit ScanStarted();

    HANDLE process_snapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(process_snapshot, &process_entry)) {
        do {
            QString process_image_name { QString::fromWCharArray(process_entry.szExeFile) };
            QString process_pid { QString::number(process_entry.th32ProcessID) };

            QTreeWidgetItem* top_level_item  {
                new QTreeWidgetItem { {process_image_name, process_pid} }
            };

            HWND process_window { nullptr };

            do {
                process_window = FindWindowEx(nullptr, process_window, nullptr, nullptr);

                DWORD window_pid { NULL };
                GetWindowThreadProcessId(process_window, &window_pid);

                if(window_pid == process_entry.th32ProcessID) {
                    bool window_visible = IsWindowVisible(process_window);

                    char window_title_buffer[512];
                    ZeroMemory(window_title_buffer, sizeof(window_title_buffer));

                    int32_t bytes_written { GetWindowTextA(process_window, window_title_buffer, sizeof(window_title_buffer)) };

                    if(bytes_written) {
                        QString window_title { window_title_buffer };
                        top_level_item->addChild(new QTreeWidgetItem { {window_title, getHwndHash(process_window), window_visible ? "Visible" : "Invisible"} });
                    }
                }
            } while(process_window != nullptr);

            if(top_level_item->childCount()) {
                emit RootItemReady(top_level_item);
            } else {
                delete top_level_item;
            }
        } while (Process32Next(process_snapshot, &process_entry));
    }

    CloseHandle(process_snapshot);

    emit ScanFinished();
}
