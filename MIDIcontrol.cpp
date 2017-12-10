/*
 * =====================================================================================
 *
 *       Filename:  MIDIcontrol.cpp
 *
 *    Description:  Controls more midicontrols
 *
 *        Version:  1.0.0
 *        Created:  06/20/2017 11:52:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ettore Chimenti <ek5.chimenti@gmail.com>
 *   Organization:  Aidilab Srl.
 *
 * =====================================================================================
 */

#include "MIDIcontrol.h"

template <class T>
status_t MIDIcontrols<T>::checkAll(){
	T* instance;
	value_t value;

	for (int i=0; i<numInstances_ ; i++){

		instance = instances_[i];
		value = instance->check();

		if ( value == RET_ERR ){
			return RET_ERR;
		}

		if ( value != RET_NOTCHANGED ) {
			//send a control midi packet
			instance->send(MIDI_);
		}
	}
	return 0;
};

template <class T>
status_t MIDIcontrols<T>::setupAll(){
	T* instance;

	for (int i=0; i<numInstances_ ; i++){
		instance = instances_[i];
		instance->setup();
	}
	return 0;
};

template <class T>
void MIDIcontrols<T>::log(char *fmt, ... ){
	if (serial_ == NULL)
		return ;

	char buf[128];
	va_list args;
	va_start (args, fmt );
	vsnprintf(buf, 128, fmt, args);
	va_end (args);
	serial_->print(buf);
}

template <class T>
status_t MIDIcontrols<T>::add(T* control){

	if (control == NULL){
		log("Error: controller is NULL!\n");
		return RET_ERR;
	}
	if (numInstances_ == MAX_INSTANCES){
		log("Error: Too many instances!\n");
		return RET_ERR;
	}

	instances_[numInstances_++] = control;
	control->parent_ = this;

	return 0;
}

template class MIDIcontrols<MIDIcontrol>;
template class MIDIcontrols<MIDIprogram>;

value_t MIDIcontrol::check(){
	value_t value;
	value_t meanValue;

	value = getValue_();
	if ( value == RET_ERR )
		return RET_ERR;

	//on the fly avg
	if (numVal_ > 1)
		meanValue = ( lastValue_ * numVal_ + value ) / (numVal_ + 1);
	else
		meanValue = value;

	if ( meanValue != lastValue_ ){
		//update value
		lastValue_ = meanValue;
		return meanValue;
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
	return map(val, 0, ADC_RANGE, 0, outRange_);
}

MIDIcontrolButton::MIDIcontrolButton (channel_t channel, control_t control,
				      pin_t pin, value_t val): pin_(pin), val_(val) {
		setChCtl(channel, control);
		type_= TYPE_DIGITAL;
}

status_t MIDIcontrolButton::setup(){
	pinMode(pin_, INPUT);
	return 0;
}

value_t MIDIcontrolButton::getValue_(){
	value_t state = digitalRead(pin_);

	//returns val or 0
	return state ? 0 : val_ ;
}

#if defined (__arc__)
MIDIcontrolCurieAcc::MIDIcontrolCurieAcc(channel_t channel, control_t control){
	setChCtl(channel, control);
}

status_t MIDIcontrolCurieAcc::setup(){
	if (!CurieIMU.begin(ACCEL))
		return RET_ERR;

	CurieIMU.setAccelerometerRange(2);
	return 0;
}

value_t MIDIcontrolCurieAcc::getValue_(){
	float ax,ay,az;
	CurieIMU.readAccelerometerScaled(ax, ay, az);
	parent_->log("acc %f, %f, %f \n", ax, ay, az);
	return sqrt(ax*ax+ay*ay+az*az);
}

MIDIcontrolCurieGyro::MIDIcontrolCurieGyro(channel_t channel, control_t control){
	setChCtl(channel, control);
}

status_t MIDIcontrolCurieGyro::setup(){
	if (!CurieIMU.begin(GYRO))
		return RET_ERR;

	CurieIMU.setGyroRange(2);
	return 0;
}

value_t MIDIcontrolCurieGyro::getValue_(){
	float ax,ay,az;
	CurieIMU.readGyroScaled(ax, ay, az);
	parent_->log("gyro %f, %f, %f \n", ax, ay, az);
	return sqrt(ax*ax+ay*ay+az*az);
}
#endif

// part of this object is taken from https://github.com/guillaume-rico/SharpIR
MIDIcontrolIRSharp::MIDIcontrolIRSharp (channel_t channel, control_t control,
				      pin_t pin): pin_(pin) {
		setChCtl(channel, control);
		type_= TYPE_ANALOG;
}

status_t MIDIcontrolIRSharp::setup(){
	pinMode(pin_, INPUT);
	return 0;
}

value_t MIDIcontrolIRSharp::getValue_(){
	int val;
	val = distance();

	return map(val, IR_MIN, IR_MAX, 0, outRange_);
}

// Sort an array
void MIDIcontrolIRSharp::sort(int a[], int size) {
    for(int i=0; i<(size-1); i++) {
        bool flag = true;
        for(int o=0; o<(size-(i+1)); o++) {
            if(a[o] > a[o+1]) {
                int t = a[o];
                a[o] = a[o+1];
                a[o+1] = t;
                flag = false;
            }
        }
        if (flag) break;
    }
}

value_t MIDIcontrolIRSharp::distance() {
    int ir_val[IR_SAMPLES];
    int mapped;
    int distanceCM;

    for (int i=0; i<IR_SAMPLES; i++){
        // Read analog value
        ir_val[i] = analogRead(pin_);
    }
    
    // Sort
    sort(ir_val,IR_SAMPLES);

    // FIXME: Maybe for Arduino 101 5000 is too high (should be 3300?)
    mapped = map(ir_val[IR_SAMPLES / 2], 0, 1023, 0, 5000);

    //GP2YA41SK0F
    distanceCM = 12.08 * pow(mapped / 1000.0, -1.058);

    // TODO: Add other types

    if ( distanceCM < IR_MIN )
	    return IR_MIN;
    else if ( distanceCM > IR_MAX )
	    return IR_MAX;

    return distanceCM;
}

int MIDIcontrolSwitchButton::instances_ = 0;
MIDIcontrolSwitchButton *MIDIcontrolSwitchButton::btn[4];

void MIDIcontrolSwitchButton::interruptHandler_0(){
		btn[0]->controlSwitchChanged = 1;
}
void MIDIcontrolSwitchButton::interruptHandler_1(){
		btn[1]->controlSwitchChanged = 1;
}
void MIDIcontrolSwitchButton::interruptHandler_2(){
		btn[2]->controlSwitchChanged = 1;
}
void MIDIcontrolSwitchButton::interruptHandler_3(){
		btn[3]->controlSwitchChanged = 1;
}

status_t MIDIcontrolSwitchButton::setup(){
	pinMode(pin_, INPUT);

	switch (instances_){
	case 0:
		attachInterrupt(pin_, interruptHandler_0, FALLING);
		break;
	case 1:
		attachInterrupt(pin_, interruptHandler_1, FALLING);
		break;
	case 2:
		attachInterrupt(pin_, interruptHandler_2, FALLING);
		break;
	case 3:
		attachInterrupt(pin_, interruptHandler_3, FALLING);
		break;
	default:
		return RET_ERR;
	}

	btn[instances_++] = this;

	//no avg
	numVal_ = 1;

	return 0;
}

MIDIcontrolSwitchButton::MIDIcontrolSwitchButton (channel_t channel, control_t control,
				      pin_t pin): pin_(pin), val_(1) {
		setChCtl(channel, control);
}

MIDIcontrolSwitchButton::MIDIcontrolSwitchButton (channel_t channel, control_t control,
				      pin_t pin, value_t val): pin_(pin), val_(val) {
		setChCtl(channel, control);
}

value_t MIDIcontrolSwitchButton::getValue_(){
	unsigned int lifetime, tried;

	if (controlSwitchChanged){
		controlSwitchChanged = 0;

		tried = millis();
		lifetime = tried - lastPressTime_;

		if (lifetime > DEBOUNCE_TIME){
			lastPressTime_ = tried;

			// switch
			return lastValue_ ? 0 : val_;
		}
	}

	return lastValue_;
}

int MIDIprogramButton::instances_ = 0;
MIDIprogramButton *MIDIprogramButton::btn[4];

void MIDIprogramButton::interruptHandler_0(){
		btn[0]->programChanged = 1;
}
void MIDIprogramButton::interruptHandler_1(){
		btn[1]->programChanged = 1;
}
void MIDIprogramButton::interruptHandler_2(){
		btn[2]->programChanged = 1;
}
void MIDIprogramButton::interruptHandler_3(){
		btn[3]->programChanged = 1;
}

status_t MIDIprogramButton::setup(){
	pinMode(pin_, INPUT);

	switch (instances_){
	case 0:
		attachInterrupt(pin_, interruptHandler_0, FALLING);
		break;
	case 1:
		attachInterrupt(pin_, interruptHandler_1, FALLING);
		break;
	case 2:
		attachInterrupt(pin_, interruptHandler_2, FALLING);
		break;
	case 3:
		attachInterrupt(pin_, interruptHandler_3, FALLING);
		break;
	}

	btn[instances_++] = this;

	return 0;
}

MIDIprogramButton::MIDIprogramButton (channel_t channel, program_t program,
				      pin_t pin): pin_(pin) {
		setChPrg(channel, program);
}

value_t MIDIprogramButton::check(){
	unsigned int lifetime, tried;

	if (programChanged){
		programChanged = 0;

		tried = millis();
		lifetime = tried - lastPressTime_;

		if (lifetime > DEBOUNCE_TIME){
			lastPressTime_ = tried;
			return 1;
		}
	}

	return RET_NOTCHANGED;
}
