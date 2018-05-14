#pragma once

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <mutex>
#include <atomic>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#ifdef SFML_SYSTEM_WINDOWS
#include <Windows.h>
#endif

#include "ImGui\imgui-setup.h"

#include "LogSystem.hpp"

#define USING_NAMESPACE \
using namespace std;\
using namespace sf

USING_NAMESPACE;

#define USE_DEFAULT_WINMAIN_ENTRYPOINT
//#define USE_DISCRETE_GPU

#define OUTPUT_LOG_TO_STDOUT
#define OUTPUT_LOG_TO_FILE
#define LOG_FILENAME "latest.log"
#define LOG_IGNORE_LEVEL Log::Debug

//Marcos & Typedefs
#define var auto
#define AUTOLOCK(a) lock_guard<mutex> lock(a)
typedef Vector2<double> Vector2d;
typedef sf::Rect<Uint32> UintRect;

//Constants
const int majorVersion = 0, minorVersion = 3, releaseVersion = 0;
const string projectCode = "Project Digit", projectSuffix = "Internal Release", releaseStage = "Beta";
const string compileTime = string(__DATE__) + " " + string(__TIME__);

const double PI = 3.1415926535897932385;
const double eps = 1e-6;

//Resources
RenderWindow win;
#ifdef OUTPUT_LOG_TO_FILE
ofstream logout(LOG_FILENAME);
#endif // OUTPUT_LOG_TO_FILE

//Locks & Mutexs
recursive_mutex renderLock, logicDataLock;

//Global Variables
atomic_bool isProgramRunning;
int logicTickPerSecond, logicTickCounter, framePerSecond, frameCounter, eventTickPerSecond, eventTickCounter;
Clock logicTickCounterClock, frameCounterClock, eventTickCounterClock;
Clock programRunTimeClock;  //Nerer resets; started as time (for this process) begins
atomic_bool isReady;

Clock desktopUpdate;

//Utilities
const double sqr(double x) { return x*x; }

const double getDisSquared(double posX1, double posY1, double posX2, double posY2) {
	return abs(posX1 - posX2)*abs(posX1 - posX2) + abs(posY1 - posY2)*abs(posY1 - posY2);
}

const double getDisSquared(Vector2d posX, Vector2d posY) {
	return getDisSquared(posX.x, posX.y, posY.x, posY.y);
}

const double getDis(double posX1, double posY1, double posX2, double posY2) {
	return sqrt(getDisSquared(posX1, posY1, posX2, posY2));
}

const double getDis(Vector2d posX, Vector2d posY) {
	return getDis(posX.x, posX.y, posY.x, posY.y);
}

const bool isSame(double x, double y) {
	if (abs(x - y) < 1e-4)
		return true;
	else
		return false;
}

template<class Type>
void resetVector2D(vector<vector<Type>>& vec, int Xsize, int Ysize, Type value) {
	vec.resize(Xsize);
	for (vector<Type>& i : vec) {
		i.resize(Ysize, value);
	}
}

template<typename Type1, typename Type2>
void waitUntilEqual(const Type1& val, const Type2 equal) {
	while (val != equal) sleep(milliseconds(2));
}


// Platform-Depedent: Windows
#ifdef SFML_SYSTEM_WINDOWS

#define USE_ASYNC_RENDERING

/*
const string getEnvironmentVariable(string name) {
	char buffer[2048];
	int size;
	size = GetEnvironmentVariable(name.c_str(), buffer, 2048);
	return size == 0 ? ""s : string(buffer);
}

void startProcess(string command, string workingDir = "") {
	LogMessage() << "Calling external runtime: " << command << dlog;

	if (workingDir == "") {
		size_t pos = command.find_last_of('\\');
		if (pos == string::npos)
			workingDir = "";
		else
			workingDir = command.substr(0, pos + 1);
	}

	mlog << "Runtime working directory: " << workingDir << dlog;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,                  // No module name (use command line)
		const_cast<LPTSTR>(command.c_str()), // Command line
		NULL,                                // Process handle not inheritable
		NULL,                                // Thread handle not inheritable
		FALSE,                               // Set handle inheritance to FALSE
		DETACHED_PROCESS,                    // Detach console if possible
		NULL,                                // Use parent's environment block
		workingDir == "" ? NULL : workingDir.c_str(),
		&si,                                 // Pointer to STARTUPINFO structure
		&pi)                                 // Pointer to PROCESS_INFORMATION structure
		) {
		mlog << Log::Error << "CreateProcess failed (" << (int)GetLastError() << ")." << dlog;
		return;
	}

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

const float getHighDpiScaleFactor() {
	UINT dpi = 96;
	DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext());
	switch (dpiAwareness) {
		// Scale the window to the system DPI
	case DPI_AWARENESS_SYSTEM_AWARE:
		dpi = GetDpiForSystem();
		break;

		// Scale the window to the monitor DPI
	case DPI_AWARENESS_PER_MONITOR_AWARE:
		dpi = GetDpiForWindow(win.getSystemHandle());
		break;
	}

	return dpi / 96.0f;
}
*/

////////////////////////////////////////////////////////////
// Inform the Nvidia/AMD driver that this SFML application
// could benefit from using the more powerful discrete GPU
////////////////////////////////////////////////////////////
#ifdef USE_DISCRETE_GPU
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // USE_DISCRETE_GPU

extern int main(int argc, char* argv[]);

////////////////////////////////////////////////////////////
// Windows specific: we define the WinMain entry point,
// so that developers can use the standard main function
// even in a Win32 Application project, and thus keep a
// portable code
////////////////////////////////////////////////////////////
#ifdef USE_DEFAULT_WINMAIN_ENTRYPOINT
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {
	return main(__argc, __argv);
}
#endif // USE_DEFAULT_WINMAIN_ENTRYPOINT

#endif // SFML_SYSTEM_WINDOWS


#include "FunctionAllocator.hpp"

#include "Variable.hpp"
#include "ArithmeticFunctions.hpp"
#include "AdvancedMathsFunctions.hpp"
#include "Script.hpp"
#include "App.hpp"


App* app;
