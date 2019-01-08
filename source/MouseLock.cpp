#include "MouseLock.h"

void MouseLock::cursorlocker_worker_() const {
	while (!m_stop_all_thread_loops) {
		while (m_looptoggle_cursorlocker && !m_stop_all_thread_loops) {
			SetCursorPos(m_resolutionWidth_center, m_resolutionHeight_center);
			Sleep(m_loopdelay_cursorlocker);
		}
		Sleep(500);
	}
}

void MouseLock::inputchecker_worker_() {
	while (!m_stop_all_thread_loops) {
		while (m_looptoggle_conditioner && !m_stop_all_thread_loops) {
			if ((GetAsyncKeyState(m_virtual_key_id_input) & 0x8000) != 0) {
				if (m_sound_frequency)  Beep(m_sound_frequency, m_sound_duration);
				m_looptoggle_cursorlocker = !m_looptoggle_cursorlocker;
			}
			Sleep(m_loopdelay_conditioner);
		}
		Sleep(500);
	}
}

void MouseLock::taskchecker_worker_() {
	std::string command = "tasklist | find \"";
	command.append(m_process_image_name);
	command.append("\" /i");

	while (!m_stop_all_thread_loops) {
		while (m_looptoggle_conditioner && !m_stop_all_thread_loops) {
			std::string command_output;
			execute_command_(command, &command_output);
			if (command_output.size() > 1) {
				if (m_sound_frequency && !m_looptoggle_cursorlocker) {
					Beep(m_sound_frequency, m_sound_duration);
				}

				m_looptoggle_cursorlocker = true;
			} else {
				if (m_sound_frequency && m_looptoggle_cursorlocker) {
					Beep(m_sound_frequency, m_sound_duration);
				}

				m_looptoggle_cursorlocker = false;
			}
		}
		Sleep(500);
	}
}

__int8 MouseLock::execute_command_(std::string command, std::string* output) const {
	std::string command_output;

	FILE* read_pipe = _popen(command.c_str(), "r");

	if (!read_pipe) {
		std::cout << "Failed to open pipe to system command: " << command << std::endl;
		return -1;
	}

	char buffer[1024];

	try {
		while (fgets(buffer, sizeof(buffer), read_pipe) != NULL)
			command_output.append(buffer);
	} catch (...) {
		std::cout << "Exception when reading from pipe." << std::endl;
		_pclose(read_pipe);
		return -2;
	}

	*output = command_output;
	return 1;
}

__int64 MouseLock::hexstr_to_int_(std::string hexadecimal) const {
	std::stringstream stream; __int64 decimal;
	stream << hexadecimal;
	stream >> std::hex >> decimal;
	return decimal;
}

std::string MouseLock::int_to_hexstr_(__int64 decimal) const {
	std::stringstream stream;
	stream << std::uppercase << std::hex << decimal;
	return stream.str();
}

__int64 MouseLock::resolve_numerical_(std::string input) const {
	bool hexadecimal = false; __int64 value = -1;
	for (std::size_t i = 0; i < input.size(); i++) {
		char& ichar = input.at(i);
		if (ichar < '0' || ichar > '9') {
			hexadecimal = true;
			break;
		}
	}

	if (hexadecimal) {
		try { value = hexstr_to_int_(input); }
		catch(...) { 
			std::cout << "Invalid hexadecimal." << std::endl;
			value = -1;
			return value;
		}
	} else {
		try { value = std::stoi(input); }
		catch (...) {
			std::cout << "Invalid decimal." << std::endl;
			value = -1;
			return value;
		}
	}
	
	return value;
}

std::pair<__int16, __int16> MouseLock::discover_resolution_() const {
	try {
		RECT desktopRect; HWND desktopHwnd;
		desktopHwnd = GetDesktopWindow();
		GetWindowRect(desktopHwnd, &desktopRect);
		return std::make_pair((__int16)desktopRect.right, (__int16)desktopRect.bottom);
	} catch (...) {
		std::cout << "Failed to discover resolution." << std::endl;
		return std::make_pair((__int16)-1, (__int16)-1);
	}	
}

std::vector<std::string> MouseLock::split_str_(std::string target, char splitter) const {
	std::vector<std::string> return_value;
	std::string buffer;

	for (std::size_t i = 0; i < target.size(); i++) {
		char& ichar = target.at(i);
		std::cout << buffer;
		if (ichar == splitter) {
			return_value.push_back(buffer);
			buffer.clear();
		} else {
			buffer.push_back(ichar);
		}
	}

	if (buffer.size() > 0) {
		return_value.push_back(buffer);
		buffer.clear();
	}

	return return_value;
}

void MouseLock::configurator_input() {
	bool input_loop = true;
	char input1;
	while (input_loop) {
		std::cout << "1.) Resolution: " << m_resolutionWidth << "x" << m_resolutionHeight << std::endl;
		std::cout << "2.) Sound: " << m_sound_frequency << "Hz / " << m_sound_duration << "ms" << std::endl;
		std::cout << "3.) Attach To Process: " << m_process_image_name << std::endl;
		std::cout << "4.) Virtual Key ID: 0x" << int_to_hexstr_(m_virtual_key_id_input) << std::endl;
		std::cout << "5.) Condition Check Delay: " << m_loopdelay_conditioner << "ms" << std::endl;
		std::cout << "6.) Cursor Locker Delay: " << m_loopdelay_cursorlocker << "ms" << std::endl;
		std::cout << std::endl << "0.) Launch MouseLock" << std::endl;

		input1 = _getch();

		switch (input1) {
		case '0': {
			m_resolutionWidth_center = (unsigned __int16)std::round(m_resolutionWidth / 2);
			m_resolutionHeight_center = (unsigned __int16)std::round(m_resolutionHeight / 2);

			m_thread_worker_binder = m_process_image_name == "Disabled" ?
				std::make_unique<std::_Binder<std::_Unforced, void(MouseLock::*)(), MouseLock*>>(std::bind(&MouseLock::inputchecker_worker_, this)) :
				std::make_unique<std::_Binder<std::_Unforced, void(MouseLock::*)(), MouseLock*>>(std::bind(&MouseLock::taskchecker_worker_, this));

			mouselock_start();

			std::cout << std::endl;
			while (input_loop) {
				std::cout << "Type \"quit\" to quit > ";
				std::string input; std::getline(std::cin, input);
				if (input == "quit") input_loop = false;
				else std::cout << std::endl;
			}

			break;
		}

		case '1': {
			std::pair<__int16, __int16> discovered_resolution = discover_resolution_();
			std::cout << std::endl << "Input Resolution (" << discovered_resolution.first << "x" << discovered_resolution.second << ") > ";
			std::string input; std::getline(std::cin, input);
			if (input != "") {
				try {
					std::vector<std::string> split = split_str_(input, 'x');

					m_resolutionWidth = (unsigned __int16)resolve_numerical_(split.at(0));
					m_resolutionHeight = (unsigned __int16)resolve_numerical_(split.at(1));
				} catch (...) {
					std::cout << std::endl << "Invalid resolution." << std::endl;
					std::cin.get();
				}
			}
			else {
				try {
					m_resolutionWidth = discovered_resolution.first;
					m_resolutionHeight = discovered_resolution.second;
				} catch (...) {
					std::cout << std::endl << "Failed to automatically assign resolution." << std::endl;
					std::cin.get();
				}
			}
			break;
		}

		case '2': {
			std::cout << std::endl << "Enter frequency and duration (1000/10) > ";
			std::string input; std::getline(std::cin, input);
			if (input != "") {
				try {
					std::vector<std::string> split = split_str_(input, '/');
					m_sound_frequency = (unsigned __int16)resolve_numerical_(split.at(0));
					m_sound_duration = (unsigned __int16)resolve_numerical_(split.at(1));
				} catch (...) {
					std::cout << std::endl << "Invalid frequency." << std::endl;
					std::cin.get();
				}
			} else {
				m_sound_frequency = 1000;
				m_sound_duration = 10;
			}		
			break;
		}

		case '3': {
			std::cout << std::endl << "Enter process image name (disabled) > ";
			std::string input; std::getline(std::cin, input);

			if (input != "") {
				m_process_image_name = input;
				m_loopdelay_conditioner = 1000;
			}
			else {
				m_process_image_name = "Disabled";
				m_loopdelay_conditioner = 100;
			}
			break;
		}
		
		case '4': {
			std::cout << std::endl << "Enter virtual key code (0x6A) > ";
			std::string input; std::getline(std::cin, input);

			if (input != "") {
				try {
					m_virtual_key_id_input = (unsigned __int8)resolve_numerical_(input);
				} catch (...) {
					std::cout << std::endl << "Unable to convert \"" << input << "\" into a numerical value." << std::endl;
					std::cin.get();

					m_virtual_key_id_input = 0x6A;
				} 
			} else {
				m_virtual_key_id_input = 0x6A;
			}

			break;
		}

		case '5': {
			std::cout << std::endl << "Input condition check delay (" << m_loopdelay_conditioner << ") > ";
			std::string input; std::getline(std::cin, input);

			if (input != "") {
				try { m_loopdelay_conditioner = (unsigned __int16)resolve_numerical_(input); }
				catch (...) {
					std::cout << std::endl << "Unable to convert \"" << input << "\" into a numerical value." << std::endl;
					std::cin.get();
					
				}
			}

			break;
		}

		case '6': {
			std::cout << std::endl << "Input mouse locking delay (" << m_loopdelay_cursorlocker << ") > ";
			std::string input; std::getline(std::cin, input);

			if (input != "") {
				try { m_loopdelay_cursorlocker = (unsigned __int16)resolve_numerical_(input); }
				catch (...) {
					std::cout << std::endl << "Unable to convert \"" << input << "\" into a numerical value." << std::endl;
				}
			}

			break;
		}
		};

		system("cls");
	}
}

void MouseLock::mouselock_start() {
	m_thread_cursorlocker = std::thread(std::bind(&MouseLock::cursorlocker_worker_, this));
	m_thread_conditioner = std::thread(*m_thread_worker_binder);
	m_looptoggle_conditioner = true;
}

void MouseLock::mouselock_stop() {
	m_looptoggle_conditioner = false;
	m_looptoggle_cursorlocker = false;
	m_stop_all_thread_loops = true;
	
	m_thread_cursorlocker.join();
	m_thread_conditioner.join();
}

MouseLock::MouseLock() {
	std::pair<__int16, __int16> discovered_resolution;
	discovered_resolution = discover_resolution_();
	
	m_resolutionWidth = discovered_resolution.first;
	m_resolutionHeight = discovered_resolution.second;

	m_sound_frequency = 1000;
	m_sound_duration = 10;
	
	m_virtual_key_id_input = 0x6A;

	m_process_image_name = "Disabled";

	m_stop_all_thread_loops = false;

	m_looptoggle_conditioner = false;
	m_looptoggle_cursorlocker = false;

	m_loopdelay_cursorlocker = 30;
	m_loopdelay_conditioner = 100;
}

MouseLock::~MouseLock() {
	std::cout << "Waiting to join threads." << std::endl;
	mouselock_stop();
}