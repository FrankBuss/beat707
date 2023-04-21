#include "plugin.hpp"


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
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(PATH2146_INPUT, "");
		configOutput(CIRCLE2150_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			midiOutput.sendMessage(msg);
		}
	}
};


struct Beat707SimulationWidget : ModuleWidget {
	Beat707SimulationWidget(Beat707Simulation* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Beat707Simulation.svg")));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(174.605, 3.903)), module, Beat707Simulation::PATH2146_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(186.142, 3.903)), module, Beat707Simulation::CIRCLE2150_OUTPUT));
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