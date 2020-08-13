// Gear Stick / H-shifter firmware for Arduino Leonardo compatible devices
// Written by Oli Norwell (www.olinorwell.com) - 11 July 2020

#include <Joystick.h>                 // Download library from https://github.com/MHeironimus/ArduinoJoystickLibrary

//#define DEBUG                       // Remove the comment '//' to enable debug mode - which will
                                      // output data into the serial console and disable the joystick functionality

Joystick_ Joystick;                   // Our joystick object - (read more at https://github.com/MHeironimus/ArduinoJoystickLibrary )

char dir_pins[] = { 8,9,10,11 };      // Our directional switches are connected to Ground and also a pin on the Leonardo [LEFT,DOWN,RIGHT,UP]
                                      // Note: due to how the joystick works, the relevant switches to connect to are opposite the location
                                      // So for example connect LEFT/pin 8 to the switch that is on the right of your joystick
                                      // Use the debug mode to see if you have wired it correctly

int dir_states[4];                    // The state of the 4 switches during this frame   (1/HIGH = not pressed,  0/LOW = pressed)
int curGear = 0;                      // The current gear that we are in
int lastGear = 0;                     // The gear that we were in during the last frame (to detect a change)
long lastChange[4];                   // The time when we last changed the state of our 4 switches (needed for debouncing)

void setup()                          // Our setup function - ran once when the device starts up
{

#ifndef DEBUG                         // If the #define DEBUG line at the top is commented out, then this runs
Joystick.begin();                     // Begin the joystick library
#endif

#ifdef DEBUG                          // If the #define DEBUG line at the top is not commented out, then this runs
Serial.begin(9600);                   // This starts up the Arduino Serial communications, which we use to send info messages to the computer
#endif

for(int a = 0; a < 4; a++)            // We have 4 pins that we are using to read the 4 switches on our joystick/gear stick [0,1,2,3]
{
  pinMode(dir_pins[a], INPUT_PULLUP); // Set them to be INPUT_PULLUP - which means that by default they will be connected to HIGH/5V
  dir_states[a] = HIGH;               // As we know that by default the pins are HIGH - we set our current states to be HIGH
  lastChange[a] = 0;                  // Set to 0 the lastChange values for each pin
}

return;
}

void loop()                           // Our loop function - after the setup function, this runs then repeats forever
{

for(int a = 0; a < 4; a++)            // For each switch read the pin value to see if the direction is pressed
{                                     // Remember when pressed it gets pulled to Ground/LOW.

  int butState = digitalRead(dir_pins[a]);  // Read the pin value
  
  if(butState != dir_states[a] && millis() - lastChange[a] > 50) // If it is different that the current value we have for it
  {                                                              // *** and *** it is over 50ms since we last changed it
  
      #ifdef DEBUG                   // This debug message is commented out - uncomment it to see in the serial monitor when
  //   Serial.println("PRESS");      // each directional switch is being pressed - this is useful to see if your wiring is
  //   Serial.println(a);            // correct
      #endif

  
        lastChange[a] = millis();    // Set the moment that we last changed it to the current milisecond counter
        dir_states[a] = butState;    // Update the state of the directional switch to what we just read
  }
}

lastGear = curGear;                  // On each loop there is the possibility of our gear changing, to know if it has
                                     // changed we need to remember what is was previously

// So now depending on the combinations on directional switches we can choose the gear we are in
// 0 = left  1 = down   2 = right  3 = up
if(dir_states[0] == LOW && dir_states[3] == LOW) curGear = 1;         // If left and up are pressed, then that's first gear
else if(dir_states[0] == LOW && dir_states[1] == LOW) curGear = 2;    // Left and down means 2nd gear
else if(dir_states[0] == HIGH && dir_states[2] == HIGH && dir_states[3] == LOW) curGear = 3; // If just up is pressed, that's 3rd
else if(dir_states[0] == HIGH && dir_states[2] == HIGH && dir_states[1] == LOW) curGear = 4; // If just down is pressed, that's 4th
else if(dir_states[2] == LOW && dir_states[3] == LOW) curGear = 5; // If right and up are pressed, then that's 5th
else if(dir_states[2] == LOW && dir_states[1] == LOW) curGear = 6; // If right and down are pressed, then that's reverse/6th
else curGear = 0;

// If the current gear we are in is now different than the last frame, then we need to tell the computer
if(curGear != lastGear)
{

// If the DEBUG symbol isn't defined then that means that we aren't in our debug mode, so send joystick mesasges
#ifndef DEBUG
  // Depending on what gear we are currently in, we firstly ensure all the other buttons are released
  // we do this as it makes no sense that you could be in multiple gears at the same time, so we ensure this can't happen
  // We then send a message to 'press' one of our 7 virtual joystick button, which represent the gears including reverse, and neutral
  
  if(curGear == 1) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(0); }
  else if(curGear == 2) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(1); }
  else if(curGear == 3) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(2); }
  else if(curGear == 4) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(3); }
  else if(curGear == 5) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(4); }
  else if(curGear == 6) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(5); }
  else if(curGear == 0) { for(int a = 0; a < 7; a++) Joystick.releaseButton(a); Joystick.pressButton(6); } 

  // Note: Whether we should be pressing a button for neutral is an interesting topic - some games set the gear to neutral
  // when no other gears are set, others don't. Depending on the game you will either want to link button '6' to neutral or not
  
#endif

// If we have our debug mode set, then rather than send a joystick command we simply send a message via the serial interface
// If using the Arduino IDE then this can be read by going to Tools->Serial Monitor
#ifdef DEBUG
  if(curGear == 1) { Serial.println("1st gear"); }
  else if(curGear == 2) { Serial.println("2nd gear"); }
  else if(curGear == 3) { Serial.println("3rd gear"); }
  else if(curGear == 4) { Serial.println("4th gear"); }
  else if(curGear == 5) { Serial.println("5th gear"); }
  else if(curGear == 6) { Serial.println("Reverse"); }
  else if(curGear == 0) { Serial.println("Neutral"); }
#endif
 
}

// We then run this function again, endlessly, while our device is powered on

  return;
}
