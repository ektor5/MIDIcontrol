/*
 * =====================================================================================
 *
 *       Filename:  MIDIcontrol.h
 *
 *    Description:  Controls more midicontrols
 *
 *        Version:  1.0
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

typedef int status_t;
typedef int type_t;
typedef int channel_t;
typedef int control_t;
typedef int program_t;
typedef int value_t;
typedef uint8_t i2caddr_t;
typedef uint8_t pin_t;
typedef void (* controlcb) (channel_t, control_t, value_t);
typedef void (* programcb) (channel_t, program_t, value_t);

#define RET_ERR -256
#define RET_OK  0
#define RET_NOTCHANGED -257

#define ADC_RANGE 1<<10
#define MIDI_RANGE 1<<7

#define MAX_INSTANCES 16

#define TYPE_ANALOG 0
#define TYPE_DIGITAL 1

#define SENSOR_WINDOW 5

class MIDIcontrol;

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

		if (MIDI_ != NULL ){
			MIDI_->sendControlChange(control_, lastValue_, channel_);
		}
	};
	inline void setCallback (controlcb cb){
		callback = cb;
	};

	//callback
	controlcb callback;

	MIDIcontrols<MIDIcontrol> * parent_;

protected:
	value_t valueWindow_[SENSOR_WINDOW];
	int windowIndex = 0;
	type_t type_;
	value_t lastValue_;
	channel_t channel_;
	control_t control_;
	virtual value_t getValue_();
	inline void setChCtl(channel_t channel, control_t control){
		channel_ = channel;
		control_ = control;
	};

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

class MIDIprogram
{
public:
	virtual status_t setup();
	virtual value_t check();
	inline void send(midi::MidiInterface<HardwareSerial> * MIDI_) {
		parent_->log("sendMIDI_PrgCh ch %d prg %d \n", channel_,
			     program_);
		if (MIDI_ != NULL ){
			MIDI_->sendProgramChange(program_, channel_);
		}
	}
	MIDIcontrols<MIDIprogram> * parent_;
protected:
	channel_t channel_;
	program_t program_;
	inline void setChPrg(channel_t channel, program_t program){
		channel_ = channel;
		program_ = program;
	}
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
	static int instances_;
	static MIDIprogramButton *btn[4];
};

#endif
