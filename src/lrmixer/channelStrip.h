/* ------------------------------------------------------------
author: "Julius Smith"
license: "MIT Style STK-4.2"
name: "compressor"
version: "0.0"
Code generated with Faust 2.27.2 (https://faust.grame.fr)
Compilation options: -lang cpp -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __ChannelStrip_H__
#define  __ChannelStrip_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>
#include <sys/mman.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS ChannelStrip
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

typedef struct {
	FAUSTFLOAT fCheckbox0;
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fHslider3;
	float fRec5[2];
	float fRec4[2];
	FAUSTFLOAT fHslider4;
	float fRec3[2];
	float fRec2[2];
	float fRec1[2];
	float fRec0[2];
	FAUSTFLOAT fHbargraph0;
}cs_members_t;

/*
#define fCheckbox0 cs_members.fCheckbox0
#define fHslider0 cs_members.fHslider0
#define fSampleRate cs_members.fSampleRate
#define fConst0 cs_members.fConst0
#define fHslider1 cs_members.fHslider1
#define fHslider2 cs_members.fHslider2
#define fHslider3 cs_members.fHslider3
#define fRec5 cs_members.fRec5
#define fRec4 cs_members.fRec4
#define fHslider4 cs_members.fHslider4
#define fRec3 cs_members.fRec3
#define fRec2 cs_members.fRec2
#define fRec1 cs_members.fRec1 
#define fRec0 cs_members.fRec0
#define fHbargraph0 cs_members.fHbargraph0
*/

class ChannelStrip : public dsp {
	
 private:
	
	cs_members_t cs_members;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.1");
		m->declare("author", "Julius Smith");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.1");
		m->declare("compressors.lib/compression_gain_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compression_gain_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compression_gain_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/compressor_lad_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compressor_lad_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compressor_lad_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/compressor_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compressor_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compressor_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/name", "Faust Compressor Effect Library");
		m->declare("compressors.lib/version", "0.0");
		m->declare("description", "Compressor demo application, adapted from the Faust Library's dm.compressor_demo in demos.lib");
		m->declare("documentation", "https://faustlibraries.grame.fr/libs/compressors/#cocompressor_mono");
		m->declare("filename", "faustTest.dsp");
		m->declare("license", "MIT Style STK-4.2");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.3");
		m->declare("name", "compressor");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.1");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.0");
		m->declare("version", "0.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	virtual int getInputRate(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	virtual int getOutputRate(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}

	~ChannelStrip(){
		munlock(&cs_members, sizeof(cs_members_t));
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		cs_members.fSampleRate = sample_rate;
		cs_members.fConst0 = (1.0f / std::min<float>(192000.0f, std::max<float>(1.0f, float(cs_members.fSampleRate))));
	}
	
	virtual void instanceResetUserInterface() {
		cs_members.fCheckbox0 = FAUSTFLOAT(0.0f);
		cs_members.fHslider0 = FAUSTFLOAT(2.0f);
		cs_members.fHslider1 = FAUSTFLOAT(15.0f);
		cs_members.fHslider2 = FAUSTFLOAT(2.0f);
		cs_members.fHslider3 = FAUSTFLOAT(40.0f);
		cs_members.fHslider4 = FAUSTFLOAT(-24.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			cs_members.fRec5[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			cs_members.fRec4[l1] = 0.0f;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			cs_members.fRec3[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			cs_members.fRec2[l3] = 0.0f;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			cs_members.fRec1[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			cs_members.fRec0[l5] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	virtual void instanceInit(int sample_rate) {
		mlock(&cs_members, sizeof(cs_members_t));
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual ChannelStrip* clone() {
		return new ChannelStrip();
	}
	
	virtual int getSampleRate() {
		return cs_members.fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->declare(0, "tooltip", "References:                 https://faustlibraries.grame.fr/libs/compressors/                 http://en.wikipedia.org/wiki/Dynamic_range_compression");
		ui_interface->openVerticalBox("COMPRESSOR");
		ui_interface->declare(0, "0", "");
		ui_interface->openHorizontalBox("0x00");
		ui_interface->declare(&cs_members.fCheckbox0, "0", "");
		ui_interface->declare(&cs_members.fCheckbox0, "tooltip", "When this is checked, the compressor                 has no effect");
		ui_interface->addCheckButton("Bypass", &cs_members.fCheckbox0);
		ui_interface->declare(&cs_members.fHbargraph0, "1", "");
		ui_interface->declare(&cs_members.fHbargraph0, "tooltip", "Compressor gain in dB");
		ui_interface->declare(&cs_members.fHbargraph0, "unit", "dB");
		ui_interface->addHorizontalBargraph("Compressor Gain", &cs_members.fHbargraph0, -50.0f, 10.0f);
		ui_interface->closeBox();
		ui_interface->declare(0, "1", "");
		ui_interface->openHorizontalBox("0x00");
		ui_interface->declare(0, "3", "");
		ui_interface->openHorizontalBox("Compression Control");
		ui_interface->declare(&cs_members.fHslider2, "0", "");
		ui_interface->declare(&cs_members.fHslider2, "style", "knob");
		ui_interface->declare(&cs_members.fHslider2, "tooltip", "A compression Ratio of N means that for each N dB increase in input         signal level above Threshold, the output level goes up 1 dB");
		ui_interface->addHorizontalSlider("Ratio", &cs_members.fHslider2, 2.0f, 1.0f, 20.0f, 0.100000001f);
		ui_interface->declare(&cs_members.fHslider4, "1", "");
		ui_interface->declare(&cs_members.fHslider4, "style", "knob");
		ui_interface->declare(&cs_members.fHslider4, "tooltip", "When the signal level exceeds the Threshold (in dB), its level         is compressed according to the Ratio");
		ui_interface->declare(&cs_members.fHslider4, "unit", "dB");
		ui_interface->addHorizontalSlider("Threshold", &cs_members.fHslider4, -24.0f, -100.0f, 10.0f, 0.100000001f);
		ui_interface->closeBox();
		ui_interface->declare(0, "4", "");
		ui_interface->openHorizontalBox("Compression Response");
		ui_interface->declare(&cs_members.fHslider1, "1", "");
		ui_interface->declare(&cs_members.fHslider1, "scale", "log");
		ui_interface->declare(&cs_members.fHslider1, "style", "knob");
		ui_interface->declare(&cs_members.fHslider1, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain         to approach (exponentially) a new lower target level (the compression         `kicking in')");
		ui_interface->declare(&cs_members.fHslider1, "unit", "ms");
		ui_interface->addHorizontalSlider("Attack", &cs_members.fHslider1, 15.0f, 1.0f, 1000.0f, 0.100000001f);
		ui_interface->declare(&cs_members.fHslider3, "2", "");
		ui_interface->declare(&cs_members.fHslider3, "scale", "log");
		ui_interface->declare(&cs_members.fHslider3, "style", "knob");
		ui_interface->declare(&cs_members.fHslider3, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain         to approach (exponentially) a new higher target level (the compression         'releasing')");
		ui_interface->declare(&cs_members.fHslider3, "unit", "ms");
		ui_interface->addHorizontalSlider("Release", &cs_members.fHslider3, 40.0f, 1.0f, 1000.0f, 0.100000001f);
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->declare(&cs_members.fHslider0, "5", "");
		ui_interface->declare(&cs_members.fHslider0, "tooltip", "The compressed-signal output level is increased by this amount         (in dB) to make up for the level lost due to compression");
		ui_interface->declare(&cs_members.fHslider0, "unit", "dB");
		ui_interface->addHorizontalSlider("MakeUpGain", &cs_members.fHslider0, 2.0f, -96.0f, 96.0f, 0.100000001f);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		int iSlow0 = int(float(cs_members.fCheckbox0));
		float fSlow1 = std::pow(10.0f, (0.0500000007f * float(cs_members.fHslider0)));
		float fSlow2 = std::max<float>(cs_members.fConst0, (0.00100000005f * float(cs_members.fHslider1)));
		float fSlow3 = (0.5f * fSlow2);
		int iSlow4 = (std::fabs(fSlow3) < 1.1920929e-07f);
		float fSlow5 = (iSlow4 ? 0.0f : std::exp((0.0f - (cs_members.fConst0 / (iSlow4 ? 1.0f : fSlow3)))));
		float fSlow6 = ((1.0f / std::max<float>(1.00000001e-07f, float(cs_members.fHslider2))) + -1.0f);
		int iSlow7 = (std::fabs(fSlow2) < 1.1920929e-07f);
		float fSlow8 = (iSlow7 ? 0.0f : std::exp((0.0f - (cs_members.fConst0 / (iSlow7 ? 1.0f : fSlow2)))));
		float fSlow9 = std::max<float>(cs_members.fConst0, (0.00100000005f * float(cs_members.fHslider3)));
		int iSlow10 = (std::fabs(fSlow9) < 1.1920929e-07f);
		float fSlow11 = (iSlow10 ? 0.0f : std::exp((0.0f - (cs_members.fConst0 / (iSlow10 ? 1.0f : fSlow9)))));
		float fSlow12 = float(cs_members.fHslider4);
		float fSlow13 = (1.0f - fSlow5);
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			float fTemp1 = (iSlow0 ? 0.0f : fTemp0);
			float fTemp2 = std::fabs(fTemp1);
			float fTemp3 = ((cs_members.fRec4[1] > fTemp2) ? fSlow11 : fSlow8);
			cs_members.fRec5[0] = ((cs_members.fRec5[1] * fTemp3) + (fTemp2 * (1.0f - fTemp3)));
			cs_members.fRec4[0] = cs_members.fRec5[0];
			cs_members.fRec3[0] = ((cs_members.fRec3[1] * fSlow5) + (fSlow6 * (std::max<float>(((20.0f * std::log10(cs_members.fRec4[0])) - fSlow12), 0.0f) * fSlow13)));
			float fTemp4 = (fTemp1 * std::pow(10.0f, (0.0500000007f * cs_members.fRec3[0])));
			float fTemp5 = std::fabs(fTemp4);
			float fTemp6 = ((cs_members.fRec1[1] > fTemp5) ? fSlow11 : fSlow8);
			cs_members.fRec2[0] = ((cs_members.fRec2[1] * fTemp6) + (fTemp5 * (1.0f - fTemp6)));
			cs_members.fRec1[0] = cs_members.fRec2[0];
			cs_members.fRec0[0] = ((fSlow5 * cs_members.fRec0[1]) + (fSlow6 * (std::max<float>(((20.0f * std::log10(cs_members.fRec1[0])) - fSlow12), 0.0f) * fSlow13)));
			cs_members.fHbargraph0 = FAUSTFLOAT((20.0f * std::log10(std::pow(10.0f, (0.0500000007f * cs_members.fRec0[0])))));
			output0[i] = FAUSTFLOAT((iSlow0 ? fTemp0 : (fSlow1 * fTemp4)));
			cs_members.fRec5[1] = cs_members.fRec5[0];
			cs_members.fRec4[1] = cs_members.fRec4[0];
			cs_members.fRec3[1] = cs_members.fRec3[0];
			cs_members.fRec2[1] = cs_members.fRec2[0];
			cs_members.fRec1[1] = cs_members.fRec1[0];
			cs_members.fRec0[1] = cs_members.fRec0[0];
		}
	}

};

#endif
