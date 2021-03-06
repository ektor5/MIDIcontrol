/*
 * =====================================================================================
 *
 *       Filename:  MIDIcontrol.h
 *
 *    Description:  Controls more midicontrols
 *
 *        Version:  1.0.0
 *        Created:  06/20/2017 11:54:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ettore Chimenti <ek5.chimenti@gmail.com>
 *   Organization:  Aidilab Srl.
 *
 * =====================================================================================
 */

#ifndef MIDICONTROLS_h
#define MIDICONTROLS_h

#include <stdint.h>
#include <stdarg.h>
#include <Arduino.h>
#include <MIDI.h>

#if defined (__arc__)
#include <CurieIMU.h>
#endif

class MIDIcontrol;
class MIDIprogram;

typedef int status_t;
typedef int type_t;
typedef int channel_t;
typedef int control_t;
typedef int program_t;
typedef int value_t;
typedef uint8_t i2caddr_t;
typedef uint8_t pin_t;
typedef void (* controlcb) (MIDIcontrol *);
typedef void (* programcb) (MIDIprogram *);

#define RET_ERR -256
#define RET_OK  0
#define RET_NOTCHANGED -257

#define ADC_RANGE 1<<10
#define MIDI_RANGE 1<<7

#define MAX_INSTANCES 16

#define TYPE_ANALOG 0
#define TYPE_DIGITAL 1

#define SENSOR_WINDOW 3
#define DEBOUNCE_TIME 250

#define IR_SAMPLES 25
#define IR_MIN 3
#define IR_MAX 31

template <class T>
class MIDIcontrols
{
public:
	status_t add(T* control);
	inline void setLog(Stream* stream){
		serial_ = stream;
	};
	inline void midi (midi::MidiInterface<HardwareSerial> * MIDI){
		MIDI_ = MIDI;
	};
	status_t checkAll();
	status_t setupAll();
	void log(char *fmt, ... );


protected:
	midi::MidiInterface<HardwareSerial> * MIDI_;
	Stream* serial_;
	int numInstances_;
	T* instances_[MAX_INSTANCES];

};

class MIDIcontrol
{
public:
	value_t check();
	virtual status_t setup();
	inline channel_t getChannel(){ return channel_; };
	inline control_t getControl(){ return control_; };
	inline void send(midi::MidiInterface<HardwareSerial>* MIDI_) {
		parent_->log("sendMIDI_CtlCh ch %d ctl %d val %d \n", channel_,
			     control_, lastValue_);
		if (callback_ != NULL){
			callback_(this);
		}

		if (MIDI_ != NULL){
			MIDI_->sendControlChange(control_, lastValue_, channel_);
			delay(20);
		}
	};
	inline void setCallback (controlcb cb){
		callback_ = cb;
	};
	inline void setOutRange(value_t outRange){
		if ( (outRange > MIDI_RANGE) && (outRange < 0) )
			return;
		outRange_ = outRange;
	}
	inline void setNumVal(value_t numVal){
		numVal_ = numVal;
	}

	MIDIcontrols<MIDIcontrol> * parent_;

protected:
	type_t type_;
	value_t lastValue_ = 0;
	value_t numVal_ = SENSOR_WINDOW;
	channel_t channel_;
	control_t control_;
	value_t outRange_ = MIDI_RANGE;
	virtual value_t getValue_();
	inline void setChCtl(channel_t channel, control_t control){
		channel_ = channel;
		control_ = control;
	};

	//callback
	controlcb callback_ = NULL;

};

class MIDIcontrolPot : public MIDIcontrol
{
	value_t getValue_();
	pin_t pin_;
public:
	status_t setup();
	MIDIcontrolPot(channel_t channel, control_t control, pin_t pin);
};

class MIDIcontrolI2C : MIDIcontrol
{
	i2caddr_t addr;
	virtual value_t getValue_();
public:
	virtual status_t setup();
	MIDIcontrolI2C(channel_t channel, control_t control, pin_t pin);
};

#if defined (__arc__)
class MIDIcontrolCurieAcc : public MIDIcontrol
{
	value_t getValue_();
public:
	status_t setup();
	MIDIcontrolCurieAcc(channel_t channel, control_t control);
};

class MIDIcontrolCurieGyro : public MIDIcontrol
{
	value_t getValue_();
public:
	status_t setup();
	MIDIcontrolCurieGyro(channel_t channel, control_t control);
};
#endif


class MIDIcontrolButton : public MIDIcontrol
{
	pin_t pin_;
	value_t val_;
public:
	status_t setup();
	MIDIcontrolButton(channel_t channel, control_t control, pin_t pin, value_t val);
private:
	value_t getValue_();
};

class MIDIcontrolSwitchButton : public MIDIcontrol
{
public:
	volatile int controlSwitchChanged;
	static void interruptHandler_0();
	static void interruptHandler_1();
	static void interruptHandler_2();
	static void interruptHandler_3();
	status_t setup();
	MIDIcontrolSwitchButton(channel_t channel, control_t control, pin_t pin);
	MIDIcontrolSwitchButton(channel_t channel, control_t control, pin_t pin, value_t val);
protected:
	pin_t pin_;
	value_t val_;
	value_t getValue_();
	unsigned int lastPressTime_;
	static int instances_;
	static MIDIcontrolSwitchButton *btn[4];
};

class MIDIcontrolIRSharp: public MIDIcontrol
{
public:
	status_t setup();
	MIDIcontrolIRSharp(channel_t channel, control_t control, pin_t pin);
protected:
	pin_t pin_;
	int ir_val[IR_SAMPLES];
	value_t getValue_();
	void sort(int a[], int size);
	value_t distance();
};

class MIDIprogram
{
public:
	virtual status_t setup();
	virtual value_t check();
	inline channel_t getChannel(){ return channel_; };
	inline control_t getProgram(){ return program_; };
	inline void send(midi::MidiInterface<HardwareSerial> * MIDI_) {
		if (parent_ != NULL){
		parent_->log("sendMIDI_PrgCh ch %d prg %d \n", channel_,
			     program_);
		}
		if (callback_ != NULL){
			callback_(this);
		}
		if (MIDI_ != NULL ){
			MIDI_->sendProgramChange(program_, channel_);
		}
	}
	inline void setCallback( programcb callback ){
		callback_ = callback;
	}
	inline void setChannel(channel_t channel){
		channel_ = channel;
	}
	inline void setProgram(program_t program){
		program_ = program;
	}
	MIDIcontrols<MIDIprogram> * parent_;
protected:
	channel_t channel_;
	program_t program_;
	programcb callback_ = NULL;
};

class MIDIprogramButton : public MIDIprogram
{
public:
	volatile int programChanged;
	static void interruptHandler_0();
	static void interruptHandler_1();
	static void interruptHandler_2();
	static void interruptHandler_3();
	status_t check();
	status_t setup();
	MIDIprogramButton(channel_t channel, program_t program, pin_t pin);
protected:
	pin_t pin_;
	unsigned int lastPressTime_;
	static int instances_;
	static MIDIprogramButton *btn[4];
};

#endif
