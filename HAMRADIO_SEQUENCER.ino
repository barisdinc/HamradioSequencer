/*
 * Preamplifier and PowerSAmplifier Sequencer Circuit
 * Please find the schematics in http://github.com/barisdinc in repository Sequencer
 * Date : Feb 2018
 * Baris DINC (TA7W)
 */

//Input/Output PIN declerations
const byte preamp_ctrl_Pin  = 5;  // Preamplifier Control pin to switch preamp ON/OFF
const byte ptt_in_Pin       = 6;  // PTT input pin
const byte pre_in_Pin       = 7;  // pre-aplifier input pin
const byte pre_on_Pin       = 9;  // pre-aplifier on pin
const byte pa_on_Pin        = 10; // poweraplifier on pin
const byte rig_on_Pin       = 11; // PTT rig on pin
const byte pa_in_Pin        = 12; // power aplifier input pin

// Variable Declarations 
boolean       mode_TX          = false;  // Keep track of what we are doing, true || false
boolean       watch_Pre         = false;              // Keep track of preamp, true || false
unsigned int  pre_off_dly   = 200;    // delay for preamp to go to safe state, max 65535
unsigned int  pa_on_dly     = 200;      // delay for PA to get ready for tx, max 65535
unsigned int  pa_off_dly    = 200;     // delay to protect preamp and rx from pa, max 65535
unsigned int  ptt_off_dly   = 200;    // delay to protect preamp and rx from rig, max 65535
byte          debounceTime  = 50;            // time for ptt to settle, 0-255
boolean       debug         = true;              // Set to true for debug messages om serial port

long time;                         // long time – no se. 🙂 reference time for debounce

// Control State of Pins
// INPUT Pins
byte ptt_in_ON = HIGH;                // Active state for PTT in, 0 = active low, 1 = active high  
byte ptt_in_OFF = LOW;              // Active state for PTT in, 0 = active low, 1 = active high
byte pa_in_ON = LOW;                // Active state for PA in, 0 = active low, 1 = active high
byte pa_in_OFF = HIGH;                // Active state for PA in, 0 = active low, 1 = active high
byte pre_in_ON = HIGH;               // Active state for PRE in, 0 = active low, 1 = active high
byte pre_in_OFF = LOW;               // Active state for PRE in, 0 = active low, 1 = active high
byte pre_ctrl_ON = HIGH;               // Active state for PRE in, 0 = active low, 1 = active high
byte pre_ctrl_OFF = LOW;               // Active state for PRE in, 0 = active low, 1 = active high


// OUTPUT Pins
byte ptt_out_ON = LOW;              // Active state for PTT out, 0 = active low, 1 = active high
byte ptt_out_OFF = HIGH;              // Active state for PTT out, 0 = active low, 1 = active high
byte pa_out_ON = LOW;               // Active state for PA out, 0 = active low, 1 = active high
byte pa_out_OFF = HIGH;               // Active state for PA out, 0 = active low, 1 = active high
byte pre_out_ON = LOW;              // Active state for PRE out, 0 = active low, 1 = active high
byte pre_out_OFF = HIGH;              // Active state for PRE out, 0 = active low, 1 = active high

void setup() {

  if (debug) Serial.begin(9600);
  if (debug) Serial.println("Setup starting…");
  
  pinMode(ptt_in_Pin, INPUT);
  pinMode(pre_in_Pin, INPUT);
  pinMode(pa_in_Pin,  INPUT);
  pinMode(preamp_ctrl_Pin, INPUT);
  
  pinMode(pre_on_Pin, OUTPUT);
  pinMode(pa_on_Pin,  OUTPUT);
  pinMode(rig_on_Pin, OUTPUT);
//  pinMode(ready_Pin,  OUTPUT);
  
  // Ensure we don’t start TX here
  digitalWrite (pa_on_Pin, pa_out_OFF);    // pa RX
  digitalWrite (rig_on_Pin, ptt_out_OFF);   // rig RX
//  digitalWrite (ready_Pin, LOW);   // We're notready yet....
  
  if (debug) Serial.println("Waiting for ! PTT…");
  
  // read the state of the PTT input
  // Refuse to start sequencer with PTT on
  while (digitalRead(ptt_in_Pin) == ptt_in_ON)   delay (200); // Maybe some kind of error here? LED? LCD? Beep?
  
  // Start preamp if the user want it
  //if (digitalRead (pre_in_Pin) == pre_in_ON)  digitalWrite (pre_on_Pin, pre_out_ON);
  if (digitalRead (preamp_ctrl_Pin) == pre_ctrl_ON) digitalWrite (pre_on_Pin, pre_out_ON);
   else   digitalWrite (pre_on_Pin, pre_out_OFF);
  
  if (debug) Serial.println("Setup ended…");

}

void loop() {

  if (debug) {
    Serial.println("loop starting…");
    Serial.print("mode_TX: ");    if (digitalRead(ptt_in_Pin) == 0) Serial.println("RX...........");;

    Serial.print(mode_TX);
    Serial.print(" ptt_in_Pin: ");
    Serial.println(digitalRead(ptt_in_Pin));
  }
  
  if (digitalRead(ptt_in_Pin) == ptt_in_ON) {   // PTT is pressed
      time = millis();                            // Reset time
      while (millis() < time + debounceTime);     // wait a moment
      
      // Only call TX if we are in RX
      if (digitalRead(ptt_in_Pin) == ptt_in_ON && !mode_TX) TX();
    }
    
    // Only call RX if we are in TX
    if (digitalRead(ptt_in_Pin) == ptt_in_OFF && mode_TX) RX();
    
  // Preamp control off
  if (digitalRead(preamp_ctrl_Pin) == pre_ctrl_ON && !mode_TX) {
    PreCtrl(true);
  }

  
  // Preamp control on
  if (digitalRead(preamp_ctrl_Pin) == pre_ctrl_OFF) {
    PreCtrl(false);
  }
  
  if (debug) {
    Serial.println("loop ended…");
    delay(500);
  }

}

void TX() {
  if (debug) Serial.println("TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…TX…");
  mode_TX = true;

  
  // We’re running PA
  if (digitalRead(pa_in_Pin) == pa_in_ON) {
    Serial.println("PA ONNNNNNNNNNNNNNNNNN");
    if (digitalRead(pre_in_Pin) == pre_in_ON) preOff();      
    paOn();
    pttOn();
  }
  // We’re running barefoot
  else {
    if (digitalRead(pre_in_Pin) == pre_in_OFF) preOff();
    pttOn();
  }

}

void RX() {

  if (debug) Serial.println("RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…RX…");
  mode_TX = false;

  // We’re running PA
  if (digitalRead(pa_in_Pin) == pa_in_OFF) {
    Serial.println("PA OFFFFFFFFFFFFFF");
    pttOff();
    paOff();
    if (digitalRead(pre_in_Pin) == pre_in_ON) preOn();
  }
  // We’re running barefoot
  else {
    pttOff();
    if (digitalRead(pre_in_Pin) == pre_in_ON) preOn();

  }
 
}

void PreCtrl(boolean use) {
  if (use) {
    watch_Pre = true;
    preOn();
  }
  else {
    watch_Pre = false;
    preOff();
  }
}

void paOff() {
  digitalWrite (pa_on_Pin, pa_out_OFF);
}

void paOn() {
  digitalWrite (pre_on_Pin, pre_out_OFF); // To be extra extra shure. aka damage control
  digitalWrite (pa_on_Pin, pa_out_ON);
  delay (pa_on_dly);
}

void preOn() {
  if (digitalRead (preamp_ctrl_Pin) == pre_ctrl_ON)
  {
    delay (pa_off_dly);  // maybe this delay should be placed in ”paOff()” ????
    digitalWrite (pre_on_Pin,pre_out_ON);
  }
}

void preOff() {
  digitalWrite (pre_on_Pin,pre_out_OFF);
  delay (pre_off_dly);
}

void pttOn() {
  digitalWrite (pre_on_Pin, pre_out_OFF); // To be extra extra shure. aka damage control
  digitalWrite (rig_on_Pin, ptt_out_ON);
}

void pttOff() {
  digitalWrite (rig_on_Pin,ptt_out_OFF);
}



