/*
   Neutron magpie dodeca:
   First 6 key values received get assigned sequentially to the first 6 trigger outs (odd side)
   This allows you to plug in, hit each drum pad or key in sequence to assign them.
   The even side outputs are gates, if the velocity of the hit exceeds some threshold (TBD)
   a gate is generated.  Useful to provide "accent" with fixed size triggers.  Gate stays on until
   the next time that key comes in below the threshold.

   License CC-By-SA 4.0, Pete Hartman 3/2017
 * */

#include <MIDI.h>

#define THRESHOLD 100
#define BOUNCETIME 20
#define TRIGLENGTH 20

#define PINWRITE(x,y) (out2pin[x] ?  analogWrite(out2pin[x], y) : analogWrite(A14, y))

uint8_t out2pin[] = {23, 22, 20, 21, 9, 10, 0, 25, 6, 5, 4, 3}; // output pins, "0" maps to A14
                                                                // remapped to be 1 3 5 7 9 11 2 4 6 8 10 12
uint8_t triggerTimer[6];
uint8_t debounceTimer[6];
bool    debounceReady[6];
//uint8_t trigLength = 20; //this is how long the drum triggers are (in ms?)
uint8_t keyList[128]; // fast way to assign keys to outputs, just index on the key number
                      // sloppy code: assumed (correctly?) that it would be initialized to 0s
uint8_t assignedCount = 0;

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation ere:
// http://arduinomidilib.sourceforge.net/class_m_i_d_i___class.html

// ignore channel
void HandleNoteOn(byte channel, byte key, byte velocity) {
  uint8_t pin, gate;
  if (velocity != 0) { // real note on
    // deal with identification or assignment
    if (((pin = keyList[key]) == 0) && (assignedCount < 6)) {
      keyList[key] = ++assignedCount;
      pin = assignedCount;
    }
    if ((pin != 0) && debounceReady[pin-1]) { // OK we have a valid output pin, armed to fire
      // not checking out2pin[pin] for 0, because pin will always be 1-6, which are NOT 0
      analogWrite(out2pin[pin-1], 127); // triggers always full value; using 7 bit resolution
      if (velocity > THRESHOLD) {
        gate = 127;
      }
      else gate = 0;
      PINWRITE(pin+5, gate);
      triggerTimer[pin-1] = 0;
      debounceTimer[pin-1] = 0;
      debounceReady[pin-1] = false;
    }
  }
  // Try to keep your callbacks short (no delays ect) as the contrary would slow down the loop()
  // and have a bad impact on real-time performance.
}
void ISR_TRIGEND(void);

IntervalTimer countdownTimer;

void setup() {
  for (int i = 0; i < 12; i ++) {
    if (out2pin[i]) {
      pinMode(out2pin[i], OUTPUT);
      analogWriteFrequency(out2pin[i], 375000);
    }
  }
  pinMode(A14, OUTPUT);
  analogWriteFrequency(A14, 375000);
  analogWriteResolution(7);

  // setup for debounce
  for (int i = 0; i < 6; i ++) {
    debounceTimer[i] = 0;
    debounceReady[i] = true;
  }

  for (int i = 0; i < 12; i ++) {
    for (int j = 0; j < 128; j ++) {
      PINWRITE(i, j);
      delay(2);
    }
    PINWRITE(i, 0);
  }
  
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  // Connect the HandleNoteOn function to the library, so it is called upon receipt of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
 
  countdownTimer.begin(ISR_TRIGEND, 1000);

  Serial.begin(9600);
}


void loop() {
  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

  // There is no need to check if there are messages incoming if they are bound to a Callback function.
}
