#include <MIDIcontrol.h>

/*
 * MIDIcontrol Library example
 *
 * UDOO Guitar - an extended guitar powered by UDOO
 *
 * Uses the embedded Arduino 101 to control sensors and sends MIDI packets
 * to the Audio Effects (like Rakarrak or Guitarix) through ttymidi to change
 * presets or effect parameters.
 *
 * Author: Ettore Chimenti <ek5.chimenti@gmail.com>
 *
 */

#define PIN_Y_AXIS A0
#define PIN_X_AXIS A1
#define PIN_IR A5
#define PIN_BUTTON_AXIS 8
#define PIN_BUTTON_A 2
#define PIN_BUTTON_B 3
#define PIN_BUTTON_C 4
#define PIN_BUTTON_D 5
#define PIN_BUTTON_E 6
#define PIN_BUTTON_F 7

// Value of the button, if pressed
#define VALUE_BUTTON_AXIS 127

#define PRG_NEXT 82
#define PRG_PREV 81

#define PRG_MAX 6

// Decomment this for debugging
//#define DEBUG

// Instance the containers depending on the MIDI packet
MIDIcontrols<MIDIcontrol> axis;
MIDIcontrols<MIDIcontrol> accgyro;
MIDIcontrols<MIDIcontrol> looper;
MIDIcontrols<MIDIprogram> buttons;

MIDI_CREATE_DEFAULT_INSTANCE();

int prg = 0;

void prgUp(MIDIprogram * obj)
{
	if (obj == NULL)
		return;

	if (prg == PRG_MAX)
		return;
	
	obj->setProgram( ++prg );
}

void prgDown(MIDIprogram * obj)
{
	if (obj == NULL)
		return;

	if (prg == 0)
		return;
	
	obj->setProgram( --prg );
}

void setup()
{
	MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
	Serial.begin(115200);
	delay(500);

#ifdef DEBUG
	// Debug output into serial
	axis.setLog(&Serial);
	buttons.setLog(&Serial);
	looper.setLog(&Serial);
	accgyro.setLog(&Serial);
#else
	axis.midi(&MIDI);
	buttons.midi(&MIDI);
	looper.midi(&MIDI);
	accgyro.midi(&MIDI);
#endif

	// Objects can be defined and then added
	MIDIcontrol * yaxis = new MIDIcontrolPot(0x01, 0x0c, PIN_Y_AXIS);
	MIDIcontrol * xaxis = new MIDIcontrolPot(0x01, 0x16, PIN_X_AXIS);
	MIDIcontrol * baxis = new MIDIcontrolButton(0x01, 0x12, PIN_BUTTON_AXIS, VALUE_BUTTON_AXIS);
	axis.add( yaxis );
	axis.add( xaxis );
	axis.add( baxis );

	MIDIcontrol * abtn = new MIDIcontrolSwitchButton(0x01, 0x19, PIN_BUTTON_A, VALUE_BUTTON_AXIS);
	MIDIcontrol * bbtn = new MIDIcontrolSwitchButton(0x01, 0x1a, PIN_BUTTON_B, VALUE_BUTTON_AXIS);
	MIDIcontrol * cbtn = new MIDIcontrolSwitchButton(0x01, 0x1b, PIN_BUTTON_C, VALUE_BUTTON_AXIS);
	MIDIcontrol * dbtn = new MIDIcontrolSwitchButton(0x01, 0x21, PIN_BUTTON_D, VALUE_BUTTON_AXIS);
	looper.add( abtn );
	looper.add( bbtn );
	looper.add( cbtn );
	looper.add( dbtn );

	MIDIprogram * ebtn = new MIDIprogramButton(0x01, PRG_NEXT, PIN_BUTTON_E);
	MIDIprogram * fbtn = new MIDIprogramButton(0x01, PRG_PREV, PIN_BUTTON_F);
	buttons.add( ebtn );
	buttons.add( fbtn );

	// add callbacks to modify parameters at runtime
	ebtn->setCallback( prgUp );
	fbtn->setCallback( prgDown );

	// ...or directly inserted into the container
	accgyro.add( new MIDIcontrolCurieAcc (0x01, 0x28) );
	accgyro.add( new MIDIcontrolCurieGyro(0x01, 0x29) );

	MIDIcontrol * ir = new MIDIcontrolIRSharp(0x01, 0x1c, PIN_IR);
	ir->setNumVal(10); // use more values to smooth effect
	accgyro.add( ir );

	// Setup all the sensors at once
	axis.setupAll();
	buttons.setupAll();
	looper.setupAll();
	accgyro.setupAll();
}

void loop()
{
	// Check the values of sensors and send MIDI packets
	axis.checkAll();
	buttons.checkAll();
	looper.checkAll();
	accgyro.checkAll();

	delay(50);
}
