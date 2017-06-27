/*
 * =====================================================================================
 *
 *       Filename:  MIDIcontrol_factory.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/20/2017 11:54:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef MIDICONTROLS_h
#define MIDICONTROLS_h

#include <stdint.h>
#include <stdarg.h>
#include <Arduino.h>
#include <MIDI.h>

typedef int status_t;
typedef int type_t;
typedef int channel_t;
typedef int control_t;
typedef int value_t;
typedef uint8_t pin_t;

#define RET_ERR -99
#define RET_OK  0
#define RET_NOTCHANGED -98

#define ADC_RANGE 1<<10
#define MIDI_RANGE 1<<8

#define MAX_INSTANCES 16

#define TYPE_ANALOG 0

class MIDIcontrol
{
public:
	value_t checkValue();
	virtual status_t setup();
	inline channel_t getChannel(){ return channel_; }; 
	inline control_t getControl(){ return control_; }; 
	inline void controlChange(midi::MidiInterface<HardwareSerial>* MIDI_) {
		if (MIDI_ != NULL ){
			MIDI_->sendControlChange(control_, lastValue_, channel_);
		}
	}

protected:
	type_t type_;
	channel_t channel_;
	control_t control_;
	value_t lastValue_;
	virtual value_t getValue_();
	inline void setChCtl(channel_t channel, control_t control){ 
		channel_ = channel;
		control_ = control;
	}

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
private:
	value_t getValue_();
};

class MIDIcontrolButton : MIDIcontrol
{
private:
	value_t getValue_();
};

class MIDIcontrols
{
public:
	//status_t addPot(pin_t pin, type_t type, channel_t channel, control_t control);
	//status_t addButton(pin_t pin, type_t type, channel_t channel, control_t control);
	//status_t addI2C(pin_t i2caddress, type_t type, channel_t channel, control_t control);
	status_t add(MIDIcontrol* control);
	status_t setLog(Stream* stream);
	inline void midi (midi::MidiInterface<HardwareSerial> * MIDI){
		MIDI_ = MIDI;
	};
	status_t checkAll();
	status_t setupAll();
private:
	midi::MidiInterface<HardwareSerial> * MIDI_;
	int numInstances_;
	MIDIcontrol* instances_[MAX_INSTANCES];
	Stream* serial_;

	void log(char *fmt, ... );

};
#endif
