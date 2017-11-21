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
#define PIN_BUTTON_AXIS 8
#define PIN_BUTTON_A 5
#define PIN_BUTTON_B 3

// Value of the button, if pressed
#define VALUE_BUTTON_AXIS 127

#define PRG_NEXT 82
#define PRG_PREV 81

// Decomment this for debugging
//#define DEBUG

// Instance the containers depending on the MIDI packet
MIDIcontrols<MIDIcontrol> axis;
MIDIcontrols<MIDIcontrol> accgyro;
MIDIcontrols<MIDIprogram> buttons;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
	MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
	Serial.begin(115200);
	delay(1000);

#ifdef DEBUG
	// Debug output into serial
	axis.setLog(&Serial);
	buttons.setLog(&Serial);
	accgyro.setLog(&Serial);
#else
	axis.midi(&MIDI);
	buttons.midi(&MIDI);
	accgyro.midi(&MIDI);
#endif

	// Objects can be defined and then added
	MIDIcontrol * yaxis = new MIDIcontrolPot(0x01, 0x01, PIN_Y_AXIS);
	MIDIcontrol * xaxis = new MIDIcontrolPot(0x01, 0x02, PIN_X_AXIS);
	MIDIcontrol * baxis = new MIDIcontrolButton(0x01, 0x03, PIN_BUTTON_AXIS, VALUE_BUTTON_AXIS);
	axis.add( yaxis );
	axis.add( xaxis );
	axis.add( baxis );

	// ...or directly inserted into the container
	buttons.add( new MIDIprogramButton(0x01, PRG_NEXT, PIN_BUTTON_A) );
	buttons.add( new MIDIprogramButton(0x01, PRG_PREV, PIN_BUTTON_B) );
	accgyro.add( new MIDIcontrolCurieAcc (0x01, 0x04) );
	accgyro.add( new MIDIcontrolCurieGyro(0x01, 0x05) );

	// Setup all the sensors at once
	axis.setupAll();
	buttons.setupAll();
	accgyro.setupAll();
}

void loop()
{
	// Check the values of sensors and send MIDI packets
	axis.checkAll();
	buttons.checkAll();
	accgyro.checkAll();

	delay(50);
}

