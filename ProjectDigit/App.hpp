#pragma once

#include "Main.hpp"
#include "LogSystem.hpp"

#include "CoordSystem.hpp"
#include "FunctionRenderer.hpp"

USING_NAMESPACE;

class App {

public:

	void initalaize(Desktop* d);

	void initalaizePostWindow(RenderWindow& win, Desktop* d);

	void onRender(sf::RenderWindow& win);

	void updateLogic(sf::RenderWindow& win);

	void handleEvent(sf::RenderWindow& win, sf::Event& e);

private:

	void loadScriptFile(string filename);

	shared_ptr<Widget> constructFunctionGUI(string name);

	Desktop* desktop;

	CoordSystem coord;
	Vector2f offset;

	Script script;
	vector<FunctionRenderer> renderer;

	Vector2i prevMouse;

	//////////////////////////////

	Window::Ptr mainWin;
	Box::Ptr mainBox;

	Frame::Ptr fileFrame, displayFrame, functionFrame;

	Box::Ptr fileBox;
	Entry::Ptr filenameEntry;
	Button::Ptr fileLoadButton;

	Button::Ptr settingsButton;

	Box::Ptr displayBox;
	Box::Ptr unitLengthBox;
	Scrollbar::Ptr unitLengthScr;
	Label::Ptr unitLengthLabel;

	//////////////////////////////

	Window::Ptr settingsWin;
	Notebook::Ptr settingNote;

	Table::Ptr pageGraphics;
	Table::Ptr pageAdvGraphics;

	//////////////////////////////

	Window::Ptr immediateWindow;
	Entry::Ptr immediateEntry;
	Button::Ptr immediateParseButton;

	//////////////////////////////

	//Window::Ptr functionWin;
	ComboBox::Ptr funcCombo;
	Box::Ptr funcBox;

	//////////////////////////////

	function<void(void)> parseFile, parseImmediate, updateFunctionUI;
};


void App::initalaize(Desktop* d) {
	mlog << "App Initalaization" << dlog;

	desktop = d;

	coord.unitLength = 50.0;
	coord.coordAxisColor = Color::White;
	coord.coordLineColor = Color(64, 64, 64);

	offset = Vector2f(0.0f, 0.0f);
	
	////////////////////////////// GUI INITIALIZATION //////////////////////////////

	mainWin = Window::Create(Window::BACKGROUND | Window::TITLEBAR | Window::SHADOW);
	mainBox = Box::Create(Box::Orientation::VERTICAL, 5.0f);
	mainWin->SetRequisition(Vector2f(350.0f, 0.0f));
	mainWin->SetTitle(L"Controls");
	mainWin->SetPosition(Vector2f(20.0f, 20.0f));

	fileFrame = Frame::Create(L"File");
	fileBox = Box::Create(Box::Orientation::HORIZONTAL, 5.0f);
	filenameEntry = Entry::Create();
	fileLoadButton = Button::Create(L"Load");
	fileLoadButton->GetSignal(Button::OnLeftClick).Connect(parseFile = [&]() {
		thread([&]() {
			loadScriptFile(filenameEntry->GetText().toAnsiString());
		}).detach();
	});
	fileBox->Pack(Label::Create(L"Script filename"), false, true);
	fileBox->Pack(filenameEntry, true, true);
	fileBox->Pack(fileLoadButton, false, true);
	fileFrame->Add(fileBox);
	mainBox->Pack(fileFrame, false);

	settingsButton = Button::Create(L"Settings...");
	settingsButton->GetSignal(Button::OnLeftClick).Connect([&]() {
		FloatRect rect = settingsWin->GetAllocation();
		Vector2f winsize = win.getView().getSize();
		settingsWin->SetPosition(Vector2f((winsize.x - rect.width) / 2.0f, (winsize.y - rect.height) / 2.0f*0.8f));
		settingsWin->Show(true);
		desktop->BringToFront(settingsWin);
	});
	mainBox->Pack(settingsButton, false);

	displayFrame = Frame::Create(L"Display Options");
	displayBox = Box::Create(Box::Orientation::VERTICAL, 5.0f);
	unitLengthBox = Box::Create(Box::Orientation::HORIZONTAL, 5.0f);
	unitLengthScr = Scrollbar::Create(Range::Orientation::HORIZONTAL);
	unitLengthScr->GetAdjustment()->Configure(2500.0f, 900.0f, 10010.0f, 100.0f, 800.0f, 10.0f);
	unitLengthScr->GetAdjustment()->SetValue(2500.0f);
	unitLengthScr->GetAdjustment()->GetSignal(Adjustment::OnChange).Connect([&]() {
		double val = sqrt(unitLengthScr->GetAdjustment()->GetValue());
		coord.unitLength = val;
		unitLengthLabel->SetText(StringParser::toStringFormatted("%.2lfpx", val));
	});
	unitLengthLabel = Label::Create(L"50.00px");
	unitLengthBox->Pack(Label::Create(L"Unit Length"), false, false);
	unitLengthBox->Pack(unitLengthScr);
	unitLengthBox->Pack(unitLengthLabel, false);
	displayBox->Pack(unitLengthBox);

	displayFrame->Add(displayBox);
	mainBox->Pack(displayFrame, false);

	//functionFrame = Frame::Create(L"Functions");
	//functionFrame->Add(initFunctionList());
	//mainBox->Pack(functionFrame, false);

	//////////////////////////////

	settingsWin = Window::Create(Window::BACKGROUND | Window::SHADOW | Window::TITLEBAR | Window::CLOSE);
	settingsWin->SetRequisition(Vector2f(600.0f, 0.0f));
	settingsWin->Show(false);
	settingsWin->SetTitle(L"Settings");
	settingsWin->GetSignal(Window::OnCloseButton).Connect([&]() {settingsWin->Show(false); });

	settingNote = Notebook::Create();

	pageGraphics = Table::Create();
	pageAdvGraphics = Table::Create();

	settingNote->AppendPage(pageGraphics, Label::Create(L"Graphics"));
	settingNote->AppendPage(pageAdvGraphics, Label::Create(L"Advanced Graphics"));

	settingsWin->Add(settingNote);
	desktop->Add(settingsWin);

	//////////////////////////////

	immediateWindow = Window::Create(Window::BACKGROUND);
	immediateEntry = Entry::Create();
	immediateParseButton = Button::Create(L"Parse");
	auto box = Box::Create(Box::Orientation::HORIZONTAL, 5.0f);
	box->Pack(Label::Create(L"Immediate Input"), false);
	box->Pack(immediateEntry, true);
	box->Pack(immediateParseButton, false);

	parseImmediate = bind([&](Button::Ptr button, Entry::Ptr entry) {
		logicDataLock.lock();
		ScriptParser::parseLine(script, entry->GetText().toAnsiString());
		renderer.clear();
		renderer.resize(script.displays.size());
		for (int i = 0; i < script.displays.size(); i++) {
			renderer[i].create(script.displays[i], script.displays[i].name);
		}

		funcCombo->Clear();
		for (FunctionRenderer& i : renderer)
			funcCombo->AppendItem(i.getName());
		funcBox->RemoveAll();
		if (funcCombo->GetSelectedItem() != ComboBox::NONE)
			funcBox->Pack(constructFunctionGUI(funcCombo->GetSelectedText()));
		FloatRect allocation = mainWin->GetAllocation();
		Vector2f requisition = mainWin->GetRequisition();
		mainWin->SetAllocation(FloatRect(allocation.left, allocation.top, requisition.x, requisition.y));

		//functionFrame->RemoveAll();
		//functionFrame->Add(initFunctionList());
		//mainWin->SetAllocation(FloatRect(mainWin->GetAllocation().left, mainWin->GetAllocation().top,
		//	mainWin->GetRequisition().x, mainWin->GetRequisition().y));
		logicDataLock.unlock();
		entry->SetText(L"");
	}, immediateParseButton, immediateEntry);

	immediateParseButton->GetSignal(Button::OnLeftClick).Connect(parseImmediate);

	immediateWindow->Add(box);
	desktop->Add(immediateWindow);

	//////////////////////////////

	//functionWin = Window::Create(Window::BACKGROUND | Window::SHADOW | Window::TITLEBAR | Window::RESIZE);
	//functionWin->SetTitle(L"Function Options");
	//functionWin->SetRequisition(Vector2f(350.0f, 0.0f));

	funcCombo = ComboBox::Create();
	funcBox = Box::Create(Box::Orientation::VERTICAL);
	funcCombo->GetSignal(ComboBox::OnSelect).Connect([this]() {
		funcBox->RemoveAll();
		if (funcCombo->GetSelectedItem() != ComboBox::NONE)
			funcBox->Pack(constructFunctionGUI(funcCombo->GetSelectedText()));
		FloatRect allocation = mainWin->GetAllocation();
		Vector2f requisition = mainWin->GetRequisition();
		mainWin->SetAllocation(FloatRect(allocation.left, allocation.top, requisition.x, requisition.y));
	});

	box = Box::Create(Box::Orientation::VERTICAL, 3.0f);
	box->Pack(funcCombo);
	box->Pack(funcBox);
	//functionWin->Add(box);
	auto frame = Frame::Create(L"Function Controls");
	frame->Add(box);
	mainBox->Pack(frame);

	//desktop->Add(functionWin);

	mainWin->Add(mainBox);
	desktop->Add(mainWin);
}

shared_ptr<Widget> App::constructFunctionGUI(string name) {
	mlog << "[GUI] Constructing Function GUI: " << name << dlog;

	auto box = Box::Create(Box::Orientation::VERTICAL, 2.0f);
	auto valFrame = Frame::Create(L"Variables");
	auto valTable = Table::Create();
	valTable->SetColumnSpacings(2.0f);
	valTable->SetRowSpacings(2.0f);

	shared_ptr<Function> func = script.functions[name];
	shared_ptr<Script::DisplayFunction> dp;
	FunctionRenderer* fr = nullptr;
	for (Script::DisplayFunction&i : script.displays)
		if (i.name == name) {
			dp = make_shared<Script::DisplayFunction>(i);
			break;
		}
	for (FunctionRenderer& i : renderer)
		if (i.getName() == name) {
			fr = &i;
			break;
		}

	int j = 0;
	for (pair<const string, shared_ptr<Variable>>&i : dp->changeVal) {
		mlogd << "      SETVAL Name: " << i.first << dlog;
		auto spin = SpinButton::Create(-1000.0f, 1000.0f, 0.5f);
		spin->SetValue(fr->getParam(i.first));
		spin->SetDigits(2);
		spin->GetSignal(SpinButton::OnValueChanged).Connect(bind([fr, spin](string valName) {
			mlogd << "[GUI/Function] Variable spinner value changed: " << valName << " of function " << fr->getName() << dlog;
			fr->setParam(valName, spin->GetValue());
			fr->forceUpdate();
		}, i.first));
		spin->SetRequisition(Vector2f(0.0f, 21.0f));

		valTable->Attach(Label::Create(i.first), UintRect(0, j, 1, 1), Table::FILL, Table::FILL | Table::EXPAND);
		valTable->Attach(spin, UintRect(1, j, 1, 1), Table::FILL | Table::EXPAND, Table::FILL | Table::EXPAND);

		j++;
	}
	valFrame->Add(valTable);

	box->Pack(valFrame);

	return box;
}

void App::initalaizePostWindow(RenderWindow& win, Desktop* d) {
	float height = immediateWindow->GetRequisition().y;
	immediateWindow->SetAllocation(FloatRect(0.0f, win.getSize().y - height, win.getSize().x, height));

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
		i.update(win, coord);
	}

}

void App::handleEvent(RenderWindow& win, Event& e) {
	if (e.type == Event::KeyReleased) {
		if (e.key.code == Keyboard::Return) {
			if (filenameEntry->HasFocus()) {
				parseFile();
			}
			if (immediateEntry->HasFocus()) {
				parseImmediate();
			}
		}
	}
	else if (e.type == Event::Resized) {
		float height = immediateWindow->GetRequisition().y;
		immediateWindow->SetAllocation(FloatRect(0.0f, win.getSize().y - height, win.getSize().x, height));

		//mainWin->SetAllocation(FloatRect(0.0f, 0.0f, mainWin->GetRequisition().x, win.getSize().y - height));
	}
}


void App::loadScriptFile(string filename) {
	//L"\u2588"
	Script s;

	ScriptParser::parseFromFile(s, filename);

	logicDataLock.lock();

	script = s;
	renderer.resize(script.displays.size());
	for (int i = 0; i < script.displays.size(); i++) {
		renderer[i].create(script.displays[i], script.displays[i].name);
	}
	//functionFrame->RemoveAll();
	//functionFrame->Add(initFunctionList());
	//mainWin->SetAllocation(FloatRect(mainWin->GetAllocation().left, mainWin->GetAllocation().top,
	//	mainWin->GetRequisition().x, mainWin->GetRequisition().y));

	funcCombo->Clear();
	for (FunctionRenderer& i : renderer)
		funcCombo->AppendItem(i.getName());
	funcBox->RemoveAll();
	FloatRect allocation = mainWin->GetAllocation();
	Vector2f requisition = mainWin->GetRequisition();
	mainWin->SetAllocation(FloatRect(allocation.left, allocation.top, requisition.x, requisition.y));

	logicDataLock.unlock();
}
