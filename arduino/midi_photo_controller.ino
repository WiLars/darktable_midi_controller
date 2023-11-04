#include <MIDIUSB.h>
#include <MIDIUSB_Defs.h>
//#include <pitchToNote.h>
//#include <frequencyToNote.h>
//#include <pitchToFrequency.h>


#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
//#include <Keyboard.h>



const byte LATCH = 0; // shift register pulse

//LED
const byte LEDPIN = 1;

byte LED_R[16] = {127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127};
byte LED_G[16] = {127,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0};
byte LED_B[16] = {127,127,0,0,0,0,0,127,0,0,0,0,0,0,0,0};
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, LEDPIN, NEO_GRB + NEO_KHZ800);

byte mode = 0;
byte midi_val = 127;
//-----THIS BLOCK HANDLES ENCODER INPUT-----
//Encoder 1
ClickEncoder *encoder0;      // Dreh-Enkoder mit Klickbutton
ClickEncoder *encoder1;
ClickEncoder *encoder2;      // Dreh-Enkoder mit Klickbutton
ClickEncoder *encoder3;

bool enchold[4];
static int16_t enclastval[4] = {0,0,0,0};

void timerIsr() {
  encoder0->service();
  encoder1->service();
  encoder2->service();
  encoder3->service();
}

void ReadEncoder(int encindex){
  ClickEncoder *encoder;
  if (encindex==0){
    encoder=encoder0;}
  else if (encindex==1){
    encoder=encoder1;}
     else if (encindex==2){
    encoder=encoder2;}
      else if (encindex==3){
    encoder=encoder3;}


int encValue = encoder->getValue();
if (encValue != 0) {
    enclastval[encindex] += encValue;
    byte midi_send =0;
    byte control_channel = mode*4+encindex;
    if (encValue < 0){
      midi_send = 1;}
    else if (encValue > 0){
      midi_send = 127;}
    Serial.print (control_channel);
    Serial.println (midi_send);
 for (uint16_t i=0; i<abs(encValue);i++){ // darktable scheint im relativen encoding nur +1 (=1) und -1 (=127) zu verstehen, deshalb muss für encoder accel das commando einfach mehrfach geschickt werden
    controlChange(0, control_channel, midi_send);
  }
    MidiUSB.flush();
    //Serial.print (encindex);
    //Serial.print (enclastval[encindex] );
    //Serial.println (encValue);
    }

ClickEncoder::Button b = encoder->getButton();     // den Button-Status abfragen
if (b != ClickEncoder::Open) {
    switch (b) {                                     // und entsprechend darauf reagieren
      case ClickEncoder::Clicked:                    // Button wurde einmal angeklickt
        Serial.println (encindex*2+8+mode*16);

        noteOn(0, encindex*2+8+mode*16, 127);
        MidiUSB.flush();
        noteOff(0, encindex*2+8+mode*16, 127);
        MidiUSB.flush();
        break;
      case ClickEncoder::DoubleClicked:              // Doppelklick auf den Button
        Serial.println (encindex*2+8+1+mode*16);
        noteOn(0, encindex*2+8+1+mode*16, 127);
        MidiUSB.flush();
        noteOff(0, encindex*2+8+1+mode*16, 127);
        MidiUSB.flush();
        break;
//      case ClickEncoder::Held:                       // Button gedrueckt halten
//        Serial.print (encindex);
//        Serial.println ("help");
//        break;
//      case ClickEncoder::Released:                   // Button losgelassen (nach gedrueckt halten)
//        Serial.print (encindex);
//        Serial.println ("released");
//        break;
  }
}}

//-------THIS BLOCK HNADLES BUTTON INPUT
void EvalButtons(byte top_bank, byte bottom_bank, byte bottom_bank_old){ 
  for(int i = 0; i < 8; i++, top_bank = top_bank >> 1){
        if (top_bank & 1) {// bit is on
          mode = 7-i; // top_bank is reversed
          Serial.println (mode);
  }}
  for(int i = 0; i < 8; i++, bottom_bank = bottom_bank >> 1, bottom_bank_old = bottom_bank_old >> 1){
    
    int j =0;
        if (bottom_bank & 1) {// bit is on
          if ((bottom_bank^bottom_bank_old) &1){ //only send signal if bit is newly switched off
            if(i > 3){ //reorder channels for bottom line
              j=i-4;}
            else{
              j=i+4;}
            Serial.println (j+mode*16);
            noteOn(0, j+mode*16, 127);
            MidiUSB.flush();
          }}
        else{// bit is off
          
          if ((bottom_bank^bottom_bank_old) &1){ //only send signal if bit is newly switched off
           if(i > 3){ //reorder channels for bottom line
            j=i-4;}
           else{
            j=i+4;}
          noteOff(0, j+mode*16, 127);
          MidiUSB.flush();
          }}
  }
}

// --- LEDS -----
void setLEDs(){
  byte basecol[3] = {255,255,220}; //RGB
  byte color_active[3] = {64,255,64};
  byte *color; //RGB
  byte color_red[3] = {255,0,0}; //RGB
  byte color_orange[3] = {255,160,0}; //RGB
  byte color_yellow[3] = {192,255,0}; //RGB
  byte color_green[3] = {0,255,0}; //RGB
  byte color_torquise[3] = {0,255,255}; //RGB
  byte color_blue[3] = {0,0,255}; //RGB
  byte color_purple[3] = {255,0,255}; //RGB
  byte color_magenta[3] = {255,0,160}; //RGB
  byte dimming = 192;
  byte dimming_passive = 96; 
  byte dimming_local;
  
  for (uint16_t i=0; i<16;i++){
    color =basecol;
    if (mode == 4){
      switch (i){
        case 8:
        color=color_red;
        break;
        case 9:
        color=color_orange;
        break;
        case 10:
        color=color_yellow;
        break;
        case 11:
        color=color_green;
        break;
        case 12:
        color=color_torquise;
        break;
        case 13:
        color=color_blue;
        break;
        case 14:
        color=color_purple;
        break;
        case 15:
        color=color_magenta;
        break;
        default:
        color =basecol;
        break;
    }}
    if (mode == 7){
      switch (i){
        case 8:
        color=color_red;
        break;
        case 9:
        color=color_orange;
        break;
        case 10:
        color=color_green;
        break;
        case 11:
        color=color_blue;
        break;
        case 12:
        color=color_magenta;
        break;
        case 13:
        color=basecol;
        break;
        case 14:
        color=basecol;
        break;
        case 15:
        color=basecol;
        break;
        default:
        color =basecol;
        break;
    }}
    if (i == 7-mode){
      dimming_local = dimming;
      color  = color_active;
    }
    else{
      dimming_local = dimming_passive;
      }
    
    strip.setPixelColor(i,strip.Color(color[0]&dimming_local, color[1]&dimming_local,color[2]&dimming_local));
    }
  strip.show();
  
}



// ------Arduino MIDI functions MIDIUSB Library
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);}

//------SETUP FUNCTION
void setup ()
{
strip.begin();
strip.show();

// shift reg
  SPI.begin ();
  Serial.begin (115200);
  Serial.println ("Begin switch test.");
  pinMode (LATCH, OUTPUT);
  digitalWrite (LATCH, HIGH);
// encoders
  encoder0 = new ClickEncoder(19, 18, 20, 4); // den Dreh-Enkoder initialisieren
  encoder1 = new ClickEncoder(3, 4, 21, 4);
  encoder2 = new ClickEncoder(10, 9, 6, 4); // den Dreh-Enkoder initialisieren
  encoder3 = new ClickEncoder(8, 7, 5, 4);
  //encoder0->setAccelerationEnabled(false);
  //encoder1->setAccelerationEnabled(false);
  //Timer
  Timer1.initialize(1000);                      // den Interrupt-Timer fuer den Dreh-Enkoder initialisieren
  Timer1.attachInterrupt(timerIsr);
//LEDs
  setLEDs();     
}  // end of setup


byte switchBank1;
byte switchBank2;
byte oldswitchBank1;
byte oldswitchBank2;


//----- LOOP function

void loop ()
{
  digitalWrite (LATCH, LOW);    // pulse the parallel load latch
  digitalWrite (LATCH, HIGH);
  switchBank1 = SPI.transfer (0);
  switchBank2 = SPI.transfer (0);
  if (switchBank1 != oldswitchBank1 or switchBank2 != oldswitchBank2){
    //Serial.print (switchBank1);
    //Serial.println (switchBank2);
    EvalButtons(switchBank2, switchBank1, oldswitchBank1);
    oldswitchBank1 = switchBank1;
    oldswitchBank2 = switchBank2;
    delay(10);
    setLEDs();
    }
//  delay (10);   // debounce
  
  ReadEncoder(0);
  ReadEncoder(1);
  ReadEncoder(2);
  ReadEncoder(3);

  
}  // end of loop
