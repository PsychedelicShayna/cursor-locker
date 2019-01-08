#pragma once

#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <thread>
#include <memory>
#include <vector>
#include <sstream>
#include <string>

class MouseLock {
private:
	/* Resolution used in center calculation */
	unsigned __int16 m_resolutionWidth;
	unsigned __int16 m_resolutionHeight;

	/* Calculated center coordinates */
	unsigned __int16 m_resolutionWidth_center;
	unsigned __int16 m_resolutionHeight_center;

	/* Notification sound settings */
	unsigned __int16 m_sound_frequency;
	unsigned __int16 m_sound_duration;

	/* Virtual key code to detect */
	unsigned __int8 m_virtual_key_id_input;

	/* Image name to detect */
	std::string m_process_image_name;

	/* Thread stop signal */
	bool m_stop_all_thread_loops;

	/* Thread loop condition toggles */
	bool m_looptoggle_cursorlocker;
	bool m_looptoggle_conditioner;

	/* Thread loop delays */
	unsigned __int16 m_loopdelay_cursorlocker;
	unsigned __int16 m_loopdelay_conditioner;

	/* Primary Threads */
	std::thread m_thread_cursorlocker;
	std::thread m_thread_conditioner;

	/* Conditioner thread worker binder */
	std::unique_ptr<std::_Binder<std::_Unforced, void(MouseLock::*)(), MouseLock*>> m_thread_worker_binder;

	/* Thread worker methods */
	void cursorlocker_worker_() const;
	void inputchecker_worker_();
	void taskchecker_worker_();

	/* System command executer */
	__int8 execute_command_(std::string command, std::string* output) const;

	/* Convert hex string to int */
	__int64 hexstr_to_int_(std::string) const;

	/* Convert int to hex string */
	std::string int_to_hexstr_(__int64) const;

	/* Resolves decimal and hexadecimal string to int */
	__int64 resolve_numerical_(std::string) const;
	
	/* Uses Windows.h geometry to discover resolution */
	std::pair<__int16, __int16> discover_resolution_() const;

	/* Splits a string by a specific character */
	std::vector<std::string> split_str_(std::string, char) const;

public:
	/* Input loop for getting user input */
	void configurator_input();

	/* Spawns the threads */
	void mouselock_start();

	/* Joins all threads, and issues loop terminator signals */
	void mouselock_stop();

	/* Constructor that assigns default values where applicable */
	MouseLock();

	/* Destructor, calls mouselock_stop() to end threads  */
	~MouseLock();
};