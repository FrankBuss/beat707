#include "plugin.hpp"
#include "hardware.h"

#include "rack.hpp"
#include "gui.h"
using namespace rack;

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
			midiOutput.sendMessage(msg);
		}
	}
};

struct Beat707SimulationWidget;
static struct Beat707SimulationWidget *theWidget;

struct Beat707SimulationWidget : ModuleWidget {
	SegmentDisplay2 *display[24];

	void add4x7segment(rack::math::Vec pos, int start) {
		for (int i = 0; i < 4; i++) {
			SegmentDisplay2 *d = new SegmentDisplay2();
			d->box.pos = pos;
			d->box.pos.x += i * 30;
			d->box.size = Vec(30, 70);
			addChild(d);
			display[start + i] = d;
		}
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
	}
}
