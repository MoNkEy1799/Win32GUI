#include "src/Dinogui.h"

void testClickFunc(DINOGUI::Widget* wid)
{
	std::cout << "Button clicked" << std::endl;
	std::cout << ((DINOGUI::Textedit*)wid)->getText() << std::endl;
}

int main2()
{
	DINOGUI::Core* core = new DINOGUI::Core("Test Window", 800, 800);
	DINOGUI::Button* button = new DINOGUI::Button(core, "Test");
	DINOGUI::Button* button2 = new DINOGUI::Button(core, "Test2");
	button->place(0, 0);
	button->setSize(100, 100);
	button2->place(50, 100);
	button2->setSize(100, 100);
	core->run();
	return 0;
}

int main()
{
	DINOGUI::Core* core = new DINOGUI::Core("Test Window", 800, 1000);
	DINOGUI::Button* button = new DINOGUI::Button(core, "Button");
	DINOGUI::Button* button2 = new DINOGUI::Button(core, "Button2");
	DINOGUI::Button* button3 = new DINOGUI::Button(core, "Button3");
	DINOGUI::Label* label = new DINOGUI::Label(core, "Label");
	DINOGUI::Checkbox* checkbox = new DINOGUI::Checkbox(core, "Checkbox");
	DINOGUI::Textedit* textedit = new DINOGUI::Textedit(core);
	DINOGUI::Image* image = new DINOGUI::Image(core, "Dino.png");
	DINOGUI::Canvas* canvas = new DINOGUI::Canvas(core, 300, 300);
	DINOGUI::Table* table = new DINOGUI::Table(core);
	DINOGUI::Combobox* combo = new DINOGUI::Combobox(core, "combo");

	button->place(50, 50);
	button2->place(110, 70);
	button3->place(50, 70);
	label->place(120, 50);
	checkbox->place(190, 50);
	textedit->place(300, 50);
	textedit->setPlaceholderText("Edit now");
	image->place(50, 100);
	canvas->place(180, 100);
	table->place(60, 500);
	table->setSize(400, 200);
	table->setCell("One", 0, 0, 1, 2);
	table->setCell("Two", 0, 2, 1, 2);
	table->setCell("Three", 0, 4, 1, 2);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			table->setCell(std::to_string(j), i + 1, j);
		}
	}
	
	canvas->fill({ 0, 255, 255 });
	canvas->drawRectangle({ 1, 1 }, { 40, 40 }, { 0, 255, 0 });
	canvas->drawEllipse({ -20, 100 }, 90, 40, { 0, 0, 255 });
	canvas->drawCircle({ 240, 240 }, 24, { 255, 0, 0 });
	canvas->drawLine({ 30, 250 }, { 110, 100 }, { 0, 0, 0 });
	canvas->drawTriangle({ 250, 60 }, { 280, 200 }, { 200, 70 }, { 255, 0, 255 });

	combo->place(300, 20);
	combo->addItem("One");
	combo->addItem("Two");
	combo->addItem("Three");
	combo->addItem("Four");

	button->connect([textedit] { testClickFunc(textedit); });

	core->run();
	return 0;
}