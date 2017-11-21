/*
 * =====================================================================================
 *
 *       Filename:  MIDIcontrol_factory.cpp
 *
 *    Description:  Controls more midicontrols
 *
 *        Version:  1.0
 *        Created:  06/20/2017 11:52:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ek5.chimenti@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "MIDIcontrol.h"
#include <MIDI.h>

status_t MIDIcontrols::checkAll()
{
	MIDIcontrol* instance;
	value_t value;
	channel_t channel;
	control_t control;

	for (int i=0; i<numInstances_ ; i++){

		instance = instances_[i];
		channel = instance->getChannel();
		control = instance->getControl();

		value = instance->checkValue();

		if ( value == RET_ERR ){
			log("Error: ch %d ctl %d\n", channel, control);
			return RET_ERR;
		}

		if ( value != RET_NOTCHANGED ) {
			//send a control midi packet 
			instance->controlChange(MIDI_);
			log("SendMIDI ch %d ctl %d val %d \n", channel, control, value);
		}
	}

	return 0;

};

status_t MIDIcontrols::setupAll()
{
	MIDIcontrol* instance;
	channel_t channel;
	control_t control;

	for (int i=0; i<numInstances_ ; i++){

		instance = instances_[i];
		channel = instance->getChannel();
		control = instance->getControl();

		instance->setup();

		log("Setup ch %d ctl %d \n", channel, control);
	}

	return 0;
};

void MIDIcontrols::log(char *fmt, ... ){
	if (serial_ == NULL) 
		return ;

	char buf[128]; 
	va_list args;
	va_start (args, fmt );
	vsnprintf(buf, 128, fmt, args);
	va_end (args);
	serial_->print(buf);
}

status_t MIDIcontrols::setLog(Stream * stream){
	if (stream == NULL) 
		return RET_ERR;

	serial_ = stream;

	return 0;
}

status_t MIDIcontrols::add(MIDIcontrol* control){

	if (control == NULL){
		log("Error: controller is NULL!\n");
		return RET_ERR;
	}
	if (numInstances_ == MAX_INSTANCES){
		log("Error: Too many instances!\n");
		return RET_ERR;
	}

	instances_[numInstances_++] = control;

	return 0;
}

value_t MIDIcontrol::checkValue(){
	value_t value_;

	value_ = getValue_();
	if ( value_ == RET_ERR )
		return RET_ERR;

	if ( value_ != lastValue_ ){
		//update value
		lastValue_ = value_;
		return value_;
	 } else {
		return RET_NOTCHANGED;
	 }
}

MIDIcontrolPot::MIDIcontrolPot (channel_t channel, control_t control, pin_t pin):
	pin_(pin) {
		setChCtl(channel, control);
		type_= TYPE_ANALOG;
}

status_t MIDIcontrolPot::setup(){
	pinMode(pin_, INPUT);
	return 0;
}

value_t MIDIcontrolPot::getValue_(){
	value_t val = analogRead(pin_);
	//map(value, fromLow, fromHigh, toLow, toHigh)
	return map(val, 0, ADC_RANGE, 0, MIDI_RANGE);
}
