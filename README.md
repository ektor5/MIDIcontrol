# MIDIcontrol

An Arduino library that easily controls MIDI audio applications with sensors.

Basically uses the Arduino MIDI library (Link) to send the MIDI packets through
the MIDI object (could be either a USB or a Serial instance) when the sensors
change their value.

## Objects

The objects available are:

* **MIDIcontrol** (sends ControlChange)
	+ **MIDIcontrolPot**(controller, channel, pin) :: Potentiometer
	+ **MIDIcontrolButton**(controller, channel, value, pin) :: Button
	+ **MIDIcontrolCurieAcc**(controller, channel) :: Arduino 101 Accelerometer
	+ **MIDIcontrolCurieGyro**(controller, channel) :: Arduino 101 Gyroscope
	+ **MIDIcontrolIRSharp**(controller, channel, pin) :: IR Distance sensor (Sharp GP2Y0A41SK0F)
	+ **MIDIcontrolSwitchButton**(controller, channel, pin [, value]) :: Sends 0 or 1 (or value)
* **MIDIprogram** (sends ProgramChange)
	+ **MIDIprogramButton**(controller, program, pin) :: Button (interrupt)

* **MIDIcontrols**\<classtype\> :: control all instances at once

## Methods

Main callable metods:

* **MIDIcontrol/MIDIprogram**:
	* **setup**() :: setup the sensor
	* **check**() :: update last value
	* **send**(&MIDI) :: sends MIDI packet with last value
	* **setCallback**(*callback (*MIDIcontrol/MIDIprogram)) :: set callback
	* **setOutRange**(value) :: map the max output to this value (only control)
	* **setNumVal**(value) :: set number of averaged values (smoothness) (only control)

* **MIDIcontrols**\<classtype\>:
	* **add**(*instance) :: add instance to the collection
	* **setupAll**() :: setup the sensors
	* **checkAll**() :: update last values and send packets if changed
	* **midi**(&MIDI) :: set the MIDI object for sending MIDI packets
	* **setLog**(&serial) :: set the logging output (optional)

## Usage

Typical workflow:

* Include MIDIcontrol.h
```cpp
#include <MIDIcontrol.h>
```
* Declare the **MIDIcontrols** container
```cpp
// Instance the containers depending on the MIDI packet
MIDIcontrols<MIDIcontrol> axis;
MIDIcontrols<MIDIcontrol> accgyro;
```
* Initialize MIDI object and bind it to the container
```cpp
MIDI_CREATE_DEFAULT_INSTANCE(); // uses Serial

void setup()
{
	MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
	axis.midi(&MIDI);
	accgyro.midi(&MIDI);
	...
}
```
* Initialize **MIDIcontrol** objects and add them to the container
```cpp
	// Objects can be defined and then added
	MIDIcontrol * yaxis = new MIDIcontrolPot(0x01, 0x0c, PIN_Y_AXIS);
	axis.add( yaxis );
	// ...or directly inserted into the container
	accgyro.add( new MIDIcontrolCurieAcc (0x01, 0x28) );

```
* Setup all the objects
```cpp
	axis.setupAll();
	accgyro.setupAll();
```
* In the loop check all the objects
```cpp
void loop()
{
	// Check the values of sensors and send MIDI packets
	axis.checkAll();
	accgyro.checkAll();

	delay(50);
}
```
