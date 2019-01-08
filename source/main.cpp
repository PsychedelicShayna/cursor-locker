#include <windows.h>
#include <wtypes.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <map>

bool getKeyState_(unsigned VKID) {
	return ((GetAsyncKeyState(VKID) & 0x8000) != 0);
}

class MouseLocker {
public:
	short m_resolutionHeight;
	short m_resolutionWidth;
	short m_update_interval;

	unsigned __int8 m_virtualkey_code;
	
	bool m_thread_terminator;
	bool m_cursor_worker_active;
	bool m_debug_thread_active;
	
	bool m_makesound;

	short m_soundFrequency;
	short m_soundDuration;

	std::thread* m_cursor_thread;
	std::thread* m_debug_thread;

	std::pair<std::string, long> executeCommand_(std::string command) {
		std::string command_output;

		FILE* read_pipe = _popen(command.c_str(), "r");

		if (!read_pipe) {
			std::cout << "Failed to open pipe to system command: " << command << std::endl;
			return std::make_pair(command_output, 1);
		}

		char buffer[1024];

		try {
			while (fgets(buffer, sizeof(buffer), read_pipe) != NULL)
				command_output.append(buffer);
		} catch (...) {
			std::cout << "Exception when reading from pipe." << std::endl;
			_pclose(read_pipe);
			return std::make_pair(command_output, 2);
		}
		
		return std::make_pair(command_output, 0);
	}

	void cursor_worker_() {
		std::pair<short, short> coordinates;
		coordinates.first = std::round(m_resolutionWidth / 2);
		coordinates.second = std::round(m_resolutionHeight / 2);

		while (!m_thread_terminator) {
			while (m_cursor_worker_active) {
				SetCursorPos(coordinates.first, coordinates.second);
				Sleep(m_update_interval);
			}
			Sleep(500);
		}
	}

	void debug_info_worker_() {
		while (!m_thread_terminator && m_debug_thread_active) {
			system("cls");
			std::stringstream vkid_hex; vkid_hex << std::uppercase << std::hex << (int)m_virtualkey_code;
			std::cout << "Virtual Key Code: 0x" << vkid_hex.str() << std::endl;
			std::cout << "Virtual Key State: " << (int)((GetAsyncKeyState(m_virtualkey_code) & 0x8000) != 0) << " (SLOW) " << std::endl;
			std::cout << "Cursor Worker Active: " << m_cursor_worker_active << std::endl;
			std::cout << "Base Resolution: " << m_resolutionWidth << "x" << m_resolutionHeight << std::endl;
			std::cout << "Cursor Target Position: " << std::round(m_resolutionWidth / 2) << "x" << std::round(m_resolutionHeight / 2) << std::endl;
			Sleep(500);
		}
	}

	MouseLocker() {
		/* Resolution setup */
		RECT desktopGeometry;
		HWND desktopWindow = GetDesktopWindow();
		GetWindowRect(desktopWindow, &desktopGeometry);
		m_resolutionHeight = desktopGeometry.bottom;
		m_resolutionWidth = desktopGeometry.right;

		/* Cursor Thread Setup */
		m_virtualkey_code = 0x6A;
		m_update_interval = 30;

		/* Threading Terminator Setup */
		m_cursor_worker_active = false;
		m_thread_terminator = false;
		m_debug_thread_active = false;

		/* Sound Setup */
		m_makesound = true;
		m_soundFrequency = 1000;
		m_soundDuration = 10;
	}

	void spawn () {
		/* Thrading Setup */
		m_cursor_thread = new std::thread(std::bind(&MouseLocker::cursor_worker_, this));
		m_debug_thread = new std::thread(std::bind(&MouseLocker::debug_info_worker_, this));
	}

	~MouseLocker() {
		m_cursor_worker_active = false;
		m_thread_terminator = true;

		Sleep(1000);

		delete m_cursor_thread;
		delete m_debug_thread;
	}

	void start() {
		while (true) {
			bool VKID_State = (GetAsyncKeyState(m_virtualkey_code) & 0x8000) != 0;
			if (VKID_State) {
				m_cursor_worker_active = !m_cursor_worker_active;
				if(m_makesound) Beep(m_soundFrequency, m_soundDuration);
			}
			Sleep(100);
		}
	}
};

int main2(int argc, char* argv[]) {
	MouseLocker mouseLocker = MouseLocker();
	std::vector<std::string> arguments;

	std::map<std::string, unsigned long*> resolver{
		{ "vkid", (unsigned long*)&mouseLocker.m_virtualkey_code },
		{ "width",(unsigned long*)&mouseLocker.m_resolutionWidth },
		{ "height", (unsigned long*)&mouseLocker.m_resolutionHeight },
		{ "debug", (unsigned long*)&mouseLocker.m_debug_thread_active },
		{ "interval", (unsigned long*)&mouseLocker.m_update_interval },
		{"sound", (unsigned long*)&mouseLocker.m_makesound},
		{"sound_freq", (unsigned long*)&mouseLocker.m_soundFrequency},
		{"sound_dur", (unsigned long*)&mouseLocker.m_soundDuration}
	};

	for (int i = 1; i < argc; i++) arguments.push_back(std::string(argv[i]));
	for (int i = 0; i < arguments.size(); i++) {
		std::string& argument = arguments.at(i);
		std::vector<std::string> split;
		std::string buffer;

		for (int c = 0; c < argument.size(); c++) {
			char& cchar = argument.at(c);
			if (cchar == '=') {
				split.push_back(buffer);
				buffer.clear();
			} else {
				buffer.push_back(cchar);
			}
		}

		if (buffer.size() > 0) split.push_back(buffer);
		buffer.clear();
		
		std::string key = split.at(0);
		unsigned long value;

		bool hexadecimal = false;
		for (int c = 0; c < split.at(1).size(); c++) {
			char& cchar = split.at(1).at(c);
			if (cchar < 48 || cchar > 57) {
				hexadecimal = true;
				break;
			}
		}

		if (hexadecimal) {
			std::stringstream stream;
			stream << std::hex << split.at(1);
			stream >> value;
		} else { 
			value = std::stoi(split.at(1)); 
		}

		*resolver.at(key) = value;
	}

	mouseLocker.spawn();
	mouseLocker.start();

	std::cin.get();
	return EXIT_SUCCESS;
}