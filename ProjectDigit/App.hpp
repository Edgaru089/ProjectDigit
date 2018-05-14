#pragma once

#include "Main.hpp"
#include "LogSystem.hpp"

#include "CoordSystem.hpp"
#include "FunctionRenderer.hpp"

USING_NAMESPACE;

class App {

public:

	void initalaize();

	void initalaizePostWindow(RenderWindow& win);

	void onRender(sf::RenderWindow& win);

	void runImGui();

	void updateLogic(sf::RenderWindow& win);

	void handleEvent(sf::RenderWindow& win, sf::Event& e);

	void onViewportChange(RenderWindow& win);

private:

	void ResetFunctionRenderers();

	void loadScriptFile(string filename);

	CoordSystem coord;
	Vector2f offset;

	Script script;
	vector<FunctionRenderer> renderer;

	Vector2i prevMouse;


	function<void(void)> parseFile, parseImmediate, updateFunctionUI;
};


void App::ResetFunctionRenderers()
{
	renderer.clear();
	renderer.resize(script.displays.size());
	for (int i = 0; i < script.displays.size(); i++) {
		renderer[i].create(script.displays[i], script.displays[i].name);
	}
}

void App::initalaize() {
	mlog << "App Initalaization" << dlog;

	coord.unitLength = 50.0;
	coord.coordAxisColor = Color::White;
	coord.coordLineColor = Color(64, 64, 64);

	offset = Vector2f(0.0f, 0.0f);
}

void App::initalaizePostWindow(RenderWindow& win) {

}

void App::onRender(RenderWindow& win) {
	win.setView(View(offset, Vector2f(win.getSize().x, win.getSize().y)));

	coord.render(win);

	for (FunctionRenderer& i : renderer) {
		i.render(win, coord);
	}

}

void App::updateLogic(RenderWindow& win) {

	if (Mouse::isButtonPressed(Mouse::Right)) {
		if (prevMouse == Vector2i(-100000, -10000))
			prevMouse = Mouse::getPosition(win);
		else {
			offset += Vector2f(prevMouse.x - Mouse::getPosition(win).x, prevMouse.y - Mouse::getPosition(win).y);
			prevMouse = Mouse::getPosition(win);
			win.setView(View(offset, Vector2f(win.getSize().x, win.getSize().y)));
		}
	}
	else
		prevMouse = Vector2i(-100000, -10000);

	for (FunctionRenderer& i : renderer) {
		win.setView(View(offset, Vector2f(win.getSize().x, win.getSize().y)));
		i.update(win, coord);
	}

}

void App::handleEvent(RenderWindow& win, Event& e) {
	if (e.type == Event::Resized) {
		onViewportChange(win);
	}
}

void App::onViewportChange(RenderWindow& win) {

}

void App::runImGui() {

	//////////////////// Controls Window ////////////////////
	ImGui::SetNextWindowSize(ImVec2(350, 600), ImGuiCond_FirstUseEver);
	imgui::Begin("Controls", NULL, ImGuiWindowFlags_MenuBar);

	//////// Menu Bar ////////
	static bool hasLog = true, hasMetrics = false, hasDemo = false, setInputFocus = false;
	static char imm[256];
	if (imgui::BeginMenuBar()) {
		if (imgui::BeginMenu("Main")) {
			imgui::MenuItem("Log Window       ", NULL, &hasLog);
			imgui::MenuItem("Metrics Window   ", NULL, &hasMetrics);
			imgui::MenuItem("Demo Window      ", NULL, &hasDemo);
			imgui::EndMenu();
		}
		if (imgui::BeginMenu("Immediate Input")) {
			if (setInputFocus) {
				imgui::SetKeyboardFocusHere();
				setInputFocus = false;
			}
			imgui::PushItemWidth(400.0f);
			if (imgui::InputText("Input", imm, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
				thread([&]() {
					logicDataLock.lock();
					ScriptParser::parseLine(script, imm);
					ResetFunctionRenderers();
					memset(imm, 0, sizeof(imm));
					logicDataLock.unlock();
					setInputFocus = true;
				}).detach();
			}
			imgui::EndMenu();
		}
		imgui::EndMenuBar();
	}

	/// Show Metrics Window ///
	if (hasMetrics)
		imgui::ShowMetricsWindow(&hasMetrics);

	/// Show Demo Window ///
	if (hasDemo)
		imgui::ShowDemoWindow(&hasDemo);

	//////// Load File Frame ////////
	static char filename[64];
	if (imgui::InputText("Script filename", filename, 64, ImGuiInputTextFlags_EnterReturnsTrue)) {
		thread([&]() {
			loadScriptFile(filename);
		}).detach();
	}

	//////// Unit Length Silder ////////
	static int unit = coord.unitLength;
	if (imgui::DragInt("Unit Length", &unit, 0.2f, 20, 700)) {
		coord.unitLength = unit;
	}

	imgui::Separator();

	//////// Function Area ////////
	imgui::BeginChild("Functions");
	imgui::Text("Function Settings");

	static vector<float> value;
	static vector<vector<float>> color;
	int l = 0, k = 0;
	color.resize(script.displays.size());
	for (Script::DisplayFunction& dp : script.displays) {
		imgui::SetNextTreeNodeOpen(true, ImGuiCond_Appearing);
		bool gui = imgui::TreeNode(dp.name.c_str());
		FunctionRenderer* fr = nullptr;
		for (auto& i : renderer) {
			if (i.getName() == dp.name) {
				fr = &i;
				break;
			}
		}

		color[l].resize(3, 1.0f);

		if (gui)
			imgui::ColorEdit3("Color", &color[l][0], ImGuiColorEditFlags_PickerHueWheel);
		if (fr->getColor() != Color(color[l][0] * 255, color[l][1] * 255, color[l][2] * 255))
			fr->setColor(Color(color[l][0] * 255, color[l][1] * 255, color[l][2] * 255));

		l++;

		for (pair<const string, shared_ptr<Variable>>& i : dp.changeVal) {
			if (value.size() < k + 1)
				value.resize(k + 1);
			if (gui&&imgui::DragFloat(("Variable " + i.first).c_str(), &value[k], 0.01f))
				fr->setParam(i.first, value[k]);
			k++;
		}

		if (gui)
			imgui::TreePop();
	}
	value.resize(k + 1, 0.0f);

	imgui::EndChild();

	imgui::End();

	//////////////////// Log Window ////////////////////
	if (hasLog) {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		imgui::Begin("Logs", NULL, ImGuiWindowFlags_MenuBar);

		//////// Menu Bar ////////
		static bool follow = true;
		if (imgui::BeginMenuBar()) {
			if (imgui::BeginMenu("Controls")) {
				if (imgui::MenuItem("Clear"))
					dlog.clearBuffer();
				imgui::Separator();
				imgui::MenuItem("Follow the end of log     ", NULL, &follow);
				imgui::EndMenu();
			}
			imgui::EndMenuBar();
		}

		//////// Text Area ////////
		imgui::BeginChild("DigitLogScroll", Vector2i(0, 0), true);
		static float size;
		for (const string& i : dlog.getBuffers())
			imgui::Text((i + '\n').c_str());
		if (size != imgui::GetScrollMaxY() && follow)
			imgui::SetScrollY(imgui::GetScrollMaxY());
		size = imgui::GetScrollMaxY();
		imgui::EndChild();

		imgui::End();
	}

}

void App::loadScriptFile(string filename) {
	//L"\u2588"
	Script s;

	ScriptParser::parseFromFile(s, filename);

	logicDataLock.lock();

	script = s;
	ResetFunctionRenderers();

	logicDataLock.unlock();
}
