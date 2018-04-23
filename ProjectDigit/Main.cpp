
#pragma warning(disable:4244)
#pragma warning(disable:4018)

#define NOMINMAX

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "Main.hpp"
#include "LogSystem.hpp"
#include "App.hpp"

using namespace std;

void threadRendering(SFGUI& sfgui) {
	win.setActive(true);
	while (isReady) {
		if (!win.isOpen()) {
			sleep(milliseconds(5));
			continue;
		}

		renderLock.lock();

		win.setActive(true);
		win.clear();

		logicDataLock.lock();
		app->onRender(win);
		sfgui.Display(win);
		logicDataLock.unlock();

		View v = win.getView();
		win.setView(View(FloatRect(0, 0, win.getSize().x, win.getSize().y)));
		win.draw(text);
		win.setView(v);

		win.display();

		renderLock.unlock();

		frameCounter++;
		if (frameCounterClock.getElapsedTime() >= seconds(1.0f)) {
			frameCounterClock.restart();
			framePerSecond = frameCounter;
			frameCounter = 0;
			win.setTitle(StringParser::toStringFormatted("%s | Async | TPS: %d, EPS: %d, FPS: %d", projectCode.c_str(), logicTickPerSecond, eventTickPerSecond, framePerSecond));
			text.setString(StringParser::toStringFormatted("Async | TPS: %d, EPS: %d, FPS: %d", logicTickPerSecond, eventTickPerSecond, framePerSecond));
		}
	}
}

void threadLogicUpdate(int ticksPerSecond) {
	Time tickTime = seconds(1.0f / ticksPerSecond);
	Clock logicCycleClock;
	desktopUpdate.restart();
	while (isReady && win.isOpen()) {
		logicDataLock.lock();

		app->updateLogic(win);

		logicDataLock.unlock();

		Time t;
		logicTickCounter++;
		if (logicTickCounterClock.getElapsedTime() > seconds(1.0f)) {
			logicTickCounterClock.restart();
			logicTickPerSecond = logicTickCounter;
			logicTickCounter = 0;
		}
		if ((t = logicCycleClock.getElapsedTime()) < tickTime)
			sleep(tickTime - t);
		logicCycleClock.restart();
	}
}

void initRenderWindow(Uint32 style = sf::Style::Default, bool useVSync = true, int framePerSecond = 120) {
	sf::ContextSettings settings;
	VideoMode desk = VideoMode::getDesktopMode();
	settings.antialiasingLevel = 0;
	win.create(style == sf::Style::Fullscreen ? desk : VideoMode(desk.width * 5 / 6, desk.height * 5 / 6), projectCode, style, settings);
	win.clear();
	win.display();
	if (useVSync)
		win.setVerticalSyncEnabled(true);
	else
		win.setFramerateLimit(framePerSecond);
	win.setVisible(true);
}


#ifdef SFML_SYSTEM_WINDOWS
#include <Windows.h>
//Platform-Depedent: Windows
BOOL systemExitEventHandler(DWORD dwCtrlType) {
	if (dwCtrlType == CTRL_C_EVENT)
		mlog << Log::Error << "[Main/EVENT] Control-C Console Exit" << dlog << Log::Info;
	else if (dwCtrlType == CTRL_BREAK_EVENT)
		mlog << Log::Error << "[Main/EVENT] Control-Break Console Exit" << dlog << Log::Info;
	else if (dwCtrlType == CTRL_CLOSE_EVENT)
		mlog << Log::Error << "[Main/EVENT] Control-Close Console Exit" << dlog << Log::Info;
	else if (dwCtrlType == CTRL_LOGOFF_EVENT)
		mlog << Log::Error << "[Main/EVENT] System-Logoff Exit" << dlog << Log::Info;
	else if (dwCtrlType == CTRL_SHUTDOWN_EVENT)
		mlog << Log::Error << "[Main/EVENT] System-Shutdown Exit" << dlog << Log::Info;
	else
		return false;
	logicDataLock.lock();
	isReady = false;
	logicDataLock.unlock();
	waitUntilEqual(isProgramRunning, false);
	return true;
}
#endif // SFML_SYSTEM_WINDOWS

int main(int argc, char* argv[]) {

	isProgramRunning = true;

	cout << projectCode << ' ' << projectSuffix << "\n  Stage " << releaseStage << endl;
	cout << "Version " << majorVersion << "." << minorVersion << "." << releaseVersion << " " << releaseStage << ", Complie Time: " << compileTime << endl << endl;

	//system("PAUSE>nul");

	cout << "Pre-Initalaizing..." << flush;

#ifdef OUTPUT_LOG_TO_STDOUT
	dlog.addOutputStream(clog);
#endif
#ifdef OUTPUT_LOG_TO_FILE
	dlog.addOutputStream(logout);
#endif
	dlog.ignore(LOG_IGNORE_LEVEL);

	cout << "Done." << endl;

	mlog << Log::Info << "Initalaizing..." << dlog;

	// Initalaize function allocators
	functionAllocatorManager.addAllocator(new ArithmeticFunctionAllocator());
	functionAllocatorManager.addAllocator(new AdvMathsFunctionAllocator());

	srand(time(NULL));
	bool isFullscreen = false;

	SFGUI sfgui;
	//sfgui.AddCharacterSet(0x4E00, 0x9FA6);
	//sf::Font fon;
	//fon.loadFromFile("C:\\Windows\\Fonts\\msyh.ttc");
	//Context::Get().GetEngine().GetResourceManager().SetDefaultFont(make_shared<sf::Font>(fon));
	text.setFont(*Context::Get().GetEngine().GetResourceManager().GetFont(""));
	text.setCharacterSize(14);

	desktop = new Desktop();

	app = new App();
	app->initalaize(desktop);

	desktop->Update(0.5f);

#ifdef SFML_SYSTEM_WINDOWS
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)systemExitEventHandler, true);
#endif // SFML_SYSTEM_WINDOWS

	mlog << "Done." << dlog;

	initRenderWindow();

	isReady = true;

	app->initalaizePostWindow(win, desktop);

#ifdef USE_ASYNC_RENDERING
	mlog << Log::Warning << "Async Rendering/Logic Update Enabled. Unstable. Aware." << dlog << Log::Info;
	win.setActive(false);
	thread render(threadRendering, ref(sfgui));
	thread logic(threadLogicUpdate, 60);
#endif

	Time eventTickTime = seconds(1.0f / 60);
	Clock eventCycleClock;
	while (win.isOpen() && isReady) {

		Event event;
		while (win.pollEvent(event)) {

			logicDataLock.lock();
			desktop->HandleEvent(event);
			app->handleEvent(win, event);
			desktop->Update(desktopUpdate.restart().asSeconds());
			logicDataLock.unlock();

			if (event.type == Event::Closed) {
				logicDataLock.lock();
				isReady = false;
				win.close();
				logicDataLock.unlock();
				break;
			}
			else if (event.type == Event::Resized) {
				win.setView(View(FloatRect(0, 0, event.size.width, event.size.height)));
			}
			else if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::F11)
					if (!isFullscreen) {
#ifdef USE_ASYNC_RENDERING
						renderLock.lock();
						logicDataLock.lock();
#endif
						initRenderWindow(sf::Style::Fullscreen);
						isFullscreen = true;
						app->onViewportChange(win);
#ifdef USE_ASYNC_RENDERING
						win.setActive(false);
						logicDataLock.unlock();
						renderLock.unlock();
#endif
					}
					else {
#ifdef USE_ASYNC_RENDERING
						renderLock.lock();
						logicDataLock.lock();
#endif
						initRenderWindow();
						isFullscreen = false;
						app->onViewportChange(win);
#ifdef USE_ASYNC_RENDERING
						win.setActive(false);
						logicDataLock.unlock();
						renderLock.unlock();
#endif
					}
			}
		}
#ifndef USE_ASYNC_RENDERING
		app->updateLogic(win);
		desktop->Update(desktopUpdate.restart().asSeconds());
		win.clear();
		app->onRender(win);
		sfgui.Display(win);
		View v = win.getView();
		win.setView(View(FloatRect(0, 0, win.getSize().x, win.getSize().y)));
		win.draw(text);
		win.setView(v);
		win.display();
#endif

		eventTickCounter++;
		if (eventTickCounterClock.getElapsedTime() >= seconds(1.0f)) {
			eventTickCounterClock.restart();
			eventTickPerSecond = eventTickCounter;
			eventTickCounter = 0;
#ifndef USE_ASYNC_RENDERING
			win.setTitle(StringParser::toStringFormatted("%s | Mono-Thread | FPS: %d", projectCode.c_str(), eventTickPerSecond));
			text.setString(StringParser::toStringFormatted("Mono-Thread | FPS: %d", eventTickPerSecond));
#endif
		}

		Time t;
		if ((t = eventCycleClock.getElapsedTime()) < eventTickTime)
			sleep(eventTickTime - t);
		eventCycleClock.restart();

	}
	win.close();
	mlog << "Shutdown In Progress..." << dlog;
#ifdef USE_ASYNC_RENDERING
	mlog << "[*] Joining Render Thread..." << dlog;
	render.join();
	mlog << "[*] Joining Logic Update Thread..." << dlog;
	logic.join();
	mlog << "Complete." << dlog;
#endif
	mlog << "Byebye!" << dlog;

	sleep(milliseconds(500));
	isProgramRunning = false;

	return EXIT_SUCCESS;
}
