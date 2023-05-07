#include "plugin.hpp"
#include "hardware.h"

#include "rack.hpp"
using namespace rack;
using namespace rack::math;
#include "gui.h"

struct SegmentDisplay2 : TransparentWidget {
    volatile bool segments[7] = {false, false, false, false, false, false, false};
    NVGcolor color = nvgRGB(0xFF, 0x00, 0x00);
    NVGcolor color_off = nvgRGB(0x20, 0x00, 0x00);
	volatile bool dot = false;

    void drawSegment(NVGcontext *vg, float x1, float y1, float x2, float y2, float width, bool on) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, x1, y1);
        nvgLineTo(vg, x2, y2);
        nvgStrokeColor(vg, on ? color : color_off);
        nvgStrokeWidth(vg, width);
        nvgStroke(vg);
    }

    void draw(const DrawArgs &args) override {
        NVGcontext *vg = args.vg;

        // Draw segments as lines
		drawSegment(vg, 5, 5, 20, 5, 2, segments[0]);
		drawSegment(vg, 22.5, 7.5, 22.5, 22.5, 2, segments[1]);
		drawSegment(vg, 22.5, 27.5, 22.5, 42.5, 2, segments[2]);
		drawSegment(vg, 5, 45, 20, 45, 2, segments[3]);
		drawSegment(vg, 2.5, 27.5, 2.5, 42.5, 2, segments[4]);
		drawSegment(vg, 2.5, 7.5, 2.5, 22.5, 2, segments[5]);
		drawSegment(vg, 5, 25, 20, 25, 2, segments[6]);
		nvgBeginPath(vg);
		nvgCircle(vg, 25, 42.5, 2);
		nvgFillColor(vg, dot ? color : color_off);
		nvgFill(vg);
	}
};

struct LED : TransparentWidget {
    volatile bool on;
    NVGcolor color = nvgRGB(0xFF, 0x00, 0x00);
    NVGcolor color_off = nvgRGB(0x20, 0x00, 0x00);

    void draw(const DrawArgs &args) override {
        NVGcontext *vg = args.vg;

		nvgBeginPath(vg);
		nvgCircle(vg, 0, 0, 2);
		nvgFillColor(vg, on ? color : color_off);
		nvgFill(vg);
	}
};

struct MyButton : rack::app::ParamWidget {
    bool pressed = false;
    NVGcolor color = nvgRGB(0x50, 0x50, 0x50);
    NVGcolor color_off = nvgRGB(0x00, 0x00, 0x00);

    void onButton(const rack::event::Button &e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (e.action == GLFW_PRESS) {
                pressed = true;
                e.consume(this);
            } else if (e.action == GLFW_RELEASE) {
                pressed = false;
                e.consume(this);
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS) {
            pressed = !pressed;
            e.consume(this);
        }
    }

    void draw(const DrawArgs &args) override {
        NVGcontext *vg = args.vg;

        nvgBeginPath(vg);
        nvgCircle(vg, box.size.x / 2, box.size.y / 2, box.size.x / 2);
        nvgFillColor(vg, pressed ? color : color_off);
        nvgFill(vg);
    }
};


struct Beat707Simulation;
static struct Beat707Simulation *theSimulation;

struct Beat707Simulation : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		PATH2146_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CIRCLE2150_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	midi::InputQueue midiInput;
	midi::Output midiOutput;

	Beat707Simulation() {
		theSimulation = this;
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(PATH2146_INPUT, "");
		configOutput(CIRCLE2150_OUTPUT, "");
		logic_init();
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			for (size_t i = 0; i < msg.bytes.size(); i++) {
				midiInQueue.push(msg.bytes[i]);
			}
		}
		midi::Message out;
		out.bytes = std::vector<uint8_t>();
		while (!midiOutQueue.empty()) {
			out.bytes.push_back(midiOutQueue.pop());
			if (out.bytes.size() == 3) {
				break;
			}
		}
		midiOutput.sendMessage(out);
	}
};

struct Beat707SimulationWidget;
static struct Beat707SimulationWidget *theWidget;

struct Beat707SimulationWidget : ModuleWidget {
	SegmentDisplay2 *display[24];
	LED *leds[24];
	MyButton *buttons[32];

	void add4x7segment(Vec pos, int start) {
		for (int i = 0; i < 4; i++) {
			SegmentDisplay2 *d = new SegmentDisplay2();
			d->box.pos = pos;
			d->box.pos.x += i * 30;
			d->box.size = Vec(30, 70);
			addChild(d);
			display[start + i] = d;
		}
	}

	void addLED(Vec pos, int number) {
		LED *l = new LED();
		l->box.pos = pos;
		l->box.size = Vec(20, 20);
		addChild(l);
		leds[number] = l;
	}

	void addButton(Vec pos, int number) {
		MyButton *b = new MyButton();
		b->box.pos = pos;
		b->box.pos.x -= 10;
		b->box.pos.y -= 10;
		b->box.size = Vec(20, 20);
		addChild(b);
		buttons[number] = b;
	}

	Beat707SimulationWidget(Beat707Simulation* module) {
		theWidget = this;
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Beat707Simulation.svg")));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(174.605, 3.903)), module, Beat707Simulation::PATH2146_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(186.142, 3.903)), module, Beat707Simulation::CIRCLE2150_OUTPUT));

		add4x7segment(lcd1.pos, 0);
		add4x7segment(lcd2.pos, 4);
		add4x7segment(lcd3.pos, 8);
		add4x7segment(lcd4.pos, 12);
		add4x7segment(lcd5.pos, 16);
		add4x7segment(lcd6.pos, 20);

		memset(leds, 0, sizeof(leds));
		addLED(led4, 4);
		addLED(led5, 5);
		addLED(led6, 6);
		addLED(led7, 7);
		addLED(led8, 8);
		addLED(led9, 9);
		addLED(led10, 10);
		addLED(led11, 11);
		addLED(led12, 12);
		addLED(led13, 13);
		addLED(led14, 14);
		addLED(led15, 15);
		addLED(led16, 16);
		addLED(led17, 17);
		addLED(led18, 18);
		addLED(led19, 19);
		addLED(led20, 20);
		addLED(led21, 21);
		addLED(led22, 22);
		addLED(led23, 23);

		memset(buttons, 0, sizeof(buttons));
		addButton(button_play, 0);
		addButton(button_stop, 1);
		addButton(button_del, 2);
		addButton(button_add, 3);
		addButton(button_mode, 4);
		addButton(button_patt_section, 5);
		addButton(button_load_patt, 6);
		addButton(button_track, 7);
		addButton(button1, 8);
		addButton(button2, 9);
		addButton(button3, 10);
		addButton(button4, 11);
		addButton(button5, 12);
		addButton(button6, 13);
		addButton(button7, 14);
		addButton(button8, 15);
		addButton(button9, 16);
		addButton(button10, 17);
		addButton(button11, 18);
		addButton(button12, 19);
		addButton(button13, 20);
		addButton(button14, 21);
		addButton(button15, 22);
		addButton(button16, 23);
		//addButton(button_patt_section, 24);
	}

    void step() override {
		uint32_t nb = 0;
		for (int i = 0; i < 32; i++) {
			if (buttons[i]) {
				if (buttons[i]->pressed) nb |= 1 << i;
			}
		}
		::buttons[0] = nb & 0xff;
		::buttons[1] = (nb >> 8) & 0xff;
		::buttons[2] = (nb >> 16) & 0xff;
		::buttons[3] = (nb >> 24) & 0xff;

        ModuleWidget::step();
    }	

	void appendContextMenu(Menu* menu) override {
		Beat707Simulation* module = dynamic_cast<Beat707Simulation*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("MIDI Input"));
		appendMidiMenu(menu, &module->midiInput);
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("MIDI Output"));
		appendMidiMenu(menu, &module->midiOutput);
	}
};

Model* modelBeat707Simulation = createModel<Beat707Simulation, Beat707SimulationWidget>("Beat707Simulation");

void sendScreen() {
	if (theSimulation && theWidget) {
		for (int i = 0; i < 24; i++) {
			int group = i / 8;
			int digit = i & 7;
			int s = ::segments[group][digit];
			theWidget->display[i]->segments[0] = s & 1;
			theWidget->display[i]->segments[1] = s & 2;
			theWidget->display[i]->segments[2] = s & 4;
			theWidget->display[i]->segments[3] = s & 8;
			theWidget->display[i]->segments[4] = s & 16;
			theWidget->display[i]->segments[5] = s & 32;
			theWidget->display[i]->segments[6] = s & 64;
			theWidget->display[i]->dot = s & 128;
		}
		for (int i = 0; i < 24; i++) {
			if (theWidget->leds[i]) {
				theWidget->leds[i]->on = ::leds[i / 8] & (1 << (i & 7));
			}
		}
	}
}
