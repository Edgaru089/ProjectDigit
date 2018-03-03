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

	shared_ptr<Widget> initFunctionList();

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

	function<void(void)> parseFile, parseImmediate, updateFunctionUI;
};


void App::initalaize(Desktop* d) {
	mlog << "App Initalaization" << dlog;

	desktop = d;

	coord.unitLength = 50.0;
	coord.coordAxisColor = Color::White;
	coord.coordLineColor = Color(64, 64, 64);

	offset = Vector2f(0.0f, 0.0f);

	////////////////////////////// GUI INITALAIZATION //////////////////////////////

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

	mainWin->Add(mainBox);
	desktop->Add(mainWin);

	//////////////////////////////

	settingsWin = Window::Create(Window::BACKGROUND | Window::SHADOW | Window::TITLEBAR | Window::CLOSE | Window::TOPLEVEL);
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
	logicDataLock.unlock();
}

shared_ptr<Widget> App::initFunctionList() {
	Box::Ptr box;
	box = Box::Create(Box::Orientation::VERTICAL, 5.0f);

	if (renderer.size() == 0)
		box->Pack(Label::Create(L"No functions displayed."));

	for (FunctionRenderer& i : renderer) {

		Frame::Ptr frame = Frame::Create(i.getName());

		Table::Ptr colorTable = Table::Create();
		Label::Ptr en1 = Label::Create(L"255"), en2 = Label::Create(L"255"), en3 = Label::Create(L"255");
		Scrollbar::Ptr scr1 = Scrollbar::Create(), scr2 = Scrollbar::Create(), scr3 = Scrollbar::Create();

		auto onColorChange = [=](FunctionRenderer& ren) {
			Uint16 R = scr1->GetAdjustment()->GetValue(),
				G = scr2->GetAdjustment()->GetValue(),
				B = scr3->GetAdjustment()->GetValue();
			//mlog << "ColorChange " << ren.getName() << " " << R << " " << G << " " << B << dlog;
			ren.setColor(Color(R, G, B));
			ren.forceUpdate();
			desktop->SetProperty("Label"s + ren.getName(), "Color", Color(R, G, B));

			en1->SetText(StringParser::toStringFormatted("%03d", R));
			en2->SetText(StringParser::toStringFormatted("%03d", G));
			en3->SetText(StringParser::toStringFormatted("%03d", B));
		};

		scr1->GetAdjustment()->Configure(255, 0, 265, 2, 16, 10);
		scr2->GetAdjustment()->Configure(255, 0, 265, 2, 16, 10);
		scr3->GetAdjustment()->Configure(255, 0, 265, 2, 16, 10);
		scr1->GetAdjustment()->SetValue(255);
		scr2->GetAdjustment()->SetValue(255);
		scr3->GetAdjustment()->SetValue(255);
		scr1->GetAdjustment()->GetSignal(Adjustment::OnChange).Connect(bind(onColorChange, ref(i)));
		scr2->GetAdjustment()->GetSignal(Adjustment::OnChange).Connect(bind(onColorChange, ref(i)));
		scr3->GetAdjustment()->GetSignal(Adjustment::OnChange).Connect(bind(onColorChange, ref(i)));

		colorTable->Attach(Label::Create(L"Red"), UintRect(0, 0, 1, 1), Table::FILL, Table::FILL);
		colorTable->Attach(scr1, UintRect(1, 0, 1, 1), Table::FILL | Table::EXPAND, Table::FILL);
		colorTable->Attach(en1, UintRect(2, 0, 1, 1), Table::FILL, Table::FILL);

		colorTable->Attach(Label::Create(L"Green"), UintRect(0, 1, 1, 1), Table::FILL, Table::FILL);
		colorTable->Attach(scr2, UintRect(1, 1, 1, 1), Table::FILL | Table::EXPAND, Table::FILL);
		colorTable->Attach(en2, UintRect(2, 1, 1, 1), Table::FILL, Table::FILL);

		colorTable->Attach(Label::Create(L"Blue"), UintRect(0, 2, 1, 1), Table::FILL, Table::FILL);
		colorTable->Attach(scr3, UintRect(1, 2, 1, 1), Table::FILL | Table::EXPAND, Table::FILL);
		colorTable->Attach(en3, UintRect(2, 2, 1, 1), Table::FILL, Table::FILL);


		frame->Add(colorTable);

		box->Pack(frame);
	}

	return box;
}
