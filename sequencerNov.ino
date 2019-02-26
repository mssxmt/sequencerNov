#include <MozziGuts.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <Ead.h>
#include <mozzi_midi.h>
#include <tables/sin2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/pinknoise8192_int8.h>
#include <LowPassFilter.h>
#include <Line.h>
//#include <EventDelay.h>
#include <Portamento.h>
#include <Smooth.h>

Oscil <PINKNOISE8192_NUM_CELLS, AUDIO_RATE> aNoise(PINKNOISE8192_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_NO_ALIAS_2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri2(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu2(SQUARE_NO_ALIAS_2048_DATA);

Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> aSin3(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, CONTROL_RATE> aTri3(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, CONTROL_RATE> aSaw3(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, CONTROL_RATE> aSqu3(SQUARE_NO_ALIAS_2048_DATA);

Portamento <CONTROL_RATE>aPortamento;

Line <unsigned int> aLfo;

LowPassFilter lpf;

#define CONTROL_RATE 64

EventDelay kDelay;

float smoothness = 0.9900f;
Smooth <long> aSmoothGain(smoothness);

Ead kEnvelope(CONTROL_RATE);

// Digital
#define  STEP_1            13
#define  STEP_2            12
#define  STEP_3            11
#define  STEP_4            10
#define  STEP_5            8
#define  STEP_6            7
#define  STEP_7            6
#define  STEP_8            5
#define  SWITCH1           4
#define  SYNC_OUT          3
#define  SYNC_IN           2

uint8_t scaleMap[9][22] = {
  {0, 48, 51, 53, 55, 59, 60, 63, 60, 63, 65, 67, 70, 72, 75, 72, 75, 77, 79, 82, 84, 87}, //minor penta
  {0, 48, 50, 52, 55, 57, 60, 62, 60, 62, 64, 67, 69, 72, 74, 72, 74, 76, 79, 81, 84, 86}, //penta
  {0, 48, 50, 51, 53, 55, 56, 59, 60, 62, 63, 65, 67, 68, 70, 72, 74, 75, 77, 79, 80, 82}, //minor
  {0, 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83}, //major
  {0, 48, 50, 51, 53, 55, 57, 59, 60, 62, 63, 65, 67, 69, 70, 72, 74, 75, 77, 79, 81, 82}, //dorian
  {0, 48, 49, 52, 53, 55, 56, 59, 60, 61, 64, 65, 67, 68, 71, 72, 73, 76, 77, 79, 80, 83},//gypsy
  {0, 48, 50, 52, 53, 54, 56, 59, 60, 62, 64, 65, 66, 68, 70, 72, 74, 76, 77, 78, 80, 82},//arabic
  {0, 50, 51, 53, 54, 55, 56, 59, 60, 62, 63, 65, 66, 67, 68, 71, 72, 74, 75, 77, 78, 79},//algerian
  {0, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71, 72, 73, 74}
};
int tmp_read;
int tmp_bpm = 0;
int stp_tmp = 0;
int stp_num = 0;
int tmp_scale = 0;
unsigned int attack_ms = 10;
unsigned int decay_ms = 10;
int stp_cnt = 0;
int syn_cnt = 0;
int keySft = 0;
int Octv = 0;
int Dtn = 0;
//envelope gaim
int Evgain;
//pages
int SWITCH2;
int nob0 = 0;
int nob1 = 1;
int nob2 = 2;
int nob3 = 3;
int realNob[4][4];
int valNob[4][4];
int pageState[4] = {0,0,0,0};
byte Flag[4][4] = {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};
//vco
int curve[2][6];
int input[2] = {0,0};
int sinGain[2];
int triGain[2];
int sawGain[2];
int squGain[2];
//LPF
uint8_t Reso = 0;
uint8_t cutoff_freq = 50;
//portamento
unsigned int PortT = 0;
//LFO
int Lfo_rate = 0;
int Lfo_s;
int Lfo_form = 0;
unsigned int Lfo;
int noiseGain = 0;
//syncin
unsigned long syncIn;
unsigned long time1;
unsigned long time2;
//gain
//byte gain = 255;

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250);
  Serial.begin(115200);
  pinMode(SYNC_OUT,OUTPUT);
  pinMode(STEP_1,OUTPUT);
  pinMode(STEP_2,OUTPUT);
  pinMode(STEP_3,OUTPUT);
  pinMode(STEP_4,OUTPUT);
  pinMode(STEP_5,OUTPUT);
  pinMode(STEP_6,OUTPUT);
  pinMode(STEP_7,OUTPUT);
  pinMode(STEP_8,OUTPUT);
  pinMode(SWITCH1,INPUT);
//  pinMode(SWITCH2,INPUT);
  pinMode(SYNC_IN,INPUT);  
}

void updateControl() {  

//pageSwitcher
SWITCH2 = mozziAnalogRead(A5);
//tmp_read = map(mozziAnalogRead(A4),0, 1023, 0, 21);
tmp_read = map(mozziAnalogRead(A4),0, 1023, 0, 15);
if((digitalRead(SWITCH1) == LOW) && (SWITCH2 > 10)){
  pageState[0] = 1;
}else if((digitalRead(SWITCH1) == HIGH) && (SWITCH2 > 10)){
  pageState[1] = 1;
}else if((digitalRead(SWITCH1) == LOW) && (SWITCH2 < 10)){
  pageState[2] = 1;
}else if((digitalRead(SWITCH1) == HIGH) && (SWITCH2 < 10)){
  pageState[3] = 1;
}

for(int i=0; i<4; i++){
   realNob[i][0] = map(mozziAnalogRead(nob0),0,1023,0,127);
   realNob[i][1] = map(mozziAnalogRead(nob1),0,1023,0,127);
   realNob[i][2] = map(mozziAnalogRead(nob2),0,1023,0,127);
   realNob[i][3] = map(mozziAnalogRead(nob3),0,1023,0,127);
}

//page1
if(pageState[0] == 1){
  for(int i=0; i<4; i++){
    if(Flag[0][i] == 0){
      if(valNob[0][i] == realNob[0][i]){
      Flag[0][i] = 1;
        }
      }else{
        valNob[0][i] = realNob[0][i];
      }
    }
  pageState[0] = 0;
  for(int i=0; i<4; i++){
    Flag[1][i] = 0;
    Flag[2][i] = 0;
    Flag[3][i] = 0;
  }
}

//page2
if(pageState[1] == 1){
  for(int i=0; i<4; i++){
    if(Flag[1][i] == 0){
      if(valNob[1][i] == realNob[1][i]){
      Flag[1][i] = 1;
        }
      }else{
        valNob[1][i] = realNob[1][i];
      }
    }
  pageState[1] = 0;
  for(int i=0; i<4; i++){
    Flag[0][i] = 0;
    Flag[2][i] = 0;
    Flag[3][i] = 0;
  }
}

//page3
if(pageState[2] == 1){
  for(int i=0; i<4; i++){
    if(Flag[2][i] == 0){
      if(valNob[2][i] == realNob[2][i]){
      Flag[2][i] = 1;
        }
      }else{
        valNob[2][i] = realNob[2][i];
      }
    }
  pageState[2] = 0;
  for(int i=0; i<4; i++){
    Flag[0][i] = 0;
    Flag[1][i] = 0;
    Flag[3][i] = 0;
  }
}

//page3
if(pageState[3] == 1){
  for(int i=0; i<4; i++){
    if(Flag[3][i] == 0){
      if(valNob[3][i] == realNob[3][i]){
      Flag[3][i] = 1;
        }
      }else{
        valNob[3][i] = realNob[3][i];
      }
    }
  pageState[3] = 0;
  for(int i=0; i<4; i++){
    Flag[0][i] = 0;
    Flag[1][i] = 0;
    Flag[2][i] = 0;
  }
}


//page1
//BPM//_bpm = 1/tmp_bpm*60000
tmp_bpm = 30000/map(valNob[0][0],0,127,10,600);
stp_tmp = map(valNob[0][1],0,127,0,12);
attack_ms = map(valNob[0][2],0,127,10,1000);
decay_ms = map(valNob[0][3],0,127,1,2000);
if(stp_tmp >= 8){
  stp_num = 8;
}else{
  stp_num = stp_tmp;
}

//page2
input[0] = valNob[1][0];//VCO1
input[1] = valNob[1][1];//VCO2
Reso = map(valNob[1][2],0, 127, 0, 200);
cutoff_freq = map(valNob[1][3],0, 127, 50, 255);


//page3

tmp_scale = map(valNob[2][0],0,127,0,8);
keySft = map(valNob[2][1],0,127,0,11);
Octv = map(valNob[2][2],0,127,0,2);
PortT = map(valNob[2][3],0, 127, 0, 500);
Octv *= 12;

//page4
Lfo_form = map(valNob[3][0],0,127,0,3);
Lfo_rate = map(valNob[3][1],0, 127, 25, 55);
Dtn = map(valNob[3][2],0,127,0,50);
if (valNob[3][2] == 0){
  Dtn = 0;
}
noiseGain = map(valNob[3][3],0,127,0,100);

//VCO
curve[0][0] = map(valNob[1][0], 0, 42, 100, 0);
curve[0][1] = map(valNob[1][0], 0, 42, 0, 100);
curve[0][2] = map(valNob[1][0], 43, 84, 100, 0);
curve[0][3] = map(valNob[1][0], 43, 84, 0, 100);
curve[0][4] = map(valNob[1][0], 85, 127, 100, 0);
curve[0][5] = map(valNob[1][0], 85, 127, 0, 100);

if((0<=input[0])&&(input[0]<42)){
  sinGain[0] = curve[0][0];
}else{
  sinGain[0] = 0;
}

if((0<=input[0])&&(input[0]<42)){
  triGain[0] = curve[0][1];
}

if((43<input[0])&&(input[0]<84)){
  triGain[0] = curve[0][2];
}else if(input[0]>85){
  triGain[0] = 0;
}

if((43<input[0])&&(input[0]<84)){
  sawGain[0] = curve[0][3];
}else if(input[0]<42){
  sawGain[0] = 0;
}

if((85<input[0])&&(input[0]<=127)){
  sawGain[0] = curve[0][4];
}

if((85<input[0])&&(input[0]<=127)){
  squGain[0] = curve[0][5];
}else if(input[0]<84){
  squGain[0] = 0;
}

curve[1][0] = map(valNob[1][1], 0, 42, 100, 0);
curve[1][1] = map(valNob[1][1], 0, 42, 0, 100);
curve[1][2] = map(valNob[1][1], 43, 84, 100, 0);
curve[1][3] = map(valNob[1][1], 43, 84, 0, 100);
curve[1][4] = map(valNob[1][1], 85, 127, 100, 0);
curve[1][5] = map(valNob[1][1], 85, 127, 0, 100);

if((0<=input[1])&&(input[1]<42)){
  sinGain[1] = curve[1][0];
}else{
  sinGain[1] = 0;
}

if((0<=input[1])&&(input[1]<42)){
  triGain[1] = curve[1][1];
}

if((43<input[1])&&(input[1]<84)){
  triGain[1] = curve[1][2];
}else if(input[1]>85){
  triGain[1] = 0;
}

if((43<input[1])&&(input[1]<84)){
  sawGain[1] = curve[1][3];
}else if(input[1]<42){
  sawGain[1] = 0;
}

if((85<input[1])&&(input[1]<=127)){
  sawGain[1] = curve[1][4];
}

if((85<input[1])&&(input[1]<=127)){
  squGain[1] = curve[1][5];
}else if(input[1]<84){
  squGain[1] = 0;
}

aNoise.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

//aSin.setFreq(mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft);
//aTri.setFreq(mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft);
//aSaw.setFreq(mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft);
//aSqu.setFreq(mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft);
aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
aTri.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
aSaw.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
aSqu.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);

//aSin2.setFreq((mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft)+Dtn);
//aTri2.setFreq((mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft)+Dtn);
//aSaw2.setFreq((mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft)+Dtn);
//aSqu2.setFreq((mtof(pgm_read_byte_near((scaleMap[tmp_scale][tmp_read]))) + Octv + keySft)+Dtn);
aSin2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
aTri2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
aSaw2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
aSqu2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
if (PortT > 10){
  aPortamento.setTime(PortT);
  aPortamento.start(scaleMap[tmp_scale][tmp_read]);
  aSin.setFreq_Q16n16(aPortamento.next());
  aTri.setFreq_Q16n16(aPortamento.next());
  aSaw.setFreq_Q16n16(aPortamento.next());
  aSqu.setFreq_Q16n16(aPortamento.next());

  aSin2.setFreq_Q16n16(aPortamento.next());
  aTri2.setFreq_Q16n16(aPortamento.next());
  aSaw2.setFreq_Q16n16(aPortamento.next());
  aSqu2.setFreq_Q16n16(aPortamento.next());
}else{
  PortT = 0;
}

//LFO
aSin3.setFreq(Lfo_rate);
aTri3.setFreq(Lfo_rate);
aSaw3.setFreq(Lfo_rate);
aSqu3.setFreq(Lfo_rate);

switch(Lfo_form){
  case 0:
  Lfo_s = aSin3.next();
  break;
  case 1:
  Lfo_s = aTri3.next();
  break;
  case 2:
  Lfo_s = aSaw3.next();
  break;
  case 3:
  Lfo_s = aSqu3.next();
  break;
}

Lfo = (128u + Lfo_s)<<8;
aLfo.set(Lfo, AUDIO_RATE / CONTROL_RATE);

//AD(noSR)
//Evgain = (int) kEnvelope.next();

//Filter
lpf.setResonance(Reso);
lpf.setCutoffFreq(cutoff_freq);

//digitalWrite(SYNC_OUT, LOW);
if(kDelay.ready()){
  if(stp_cnt < stp_num){
    switch(stp_cnt){
    case  0:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, HIGH);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_5, HIGH);
      }else{
        digitalWrite(STEP_5, LOW);
      }
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  1:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, HIGH);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_6, HIGH);
      }else{
        digitalWrite(STEP_6, LOW);
      }
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, LOW);
    break;
    case  2:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, HIGH);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_7, HIGH);
      }else{
        digitalWrite(STEP_7, LOW);
      }
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  3:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, HIGH);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_8, HIGH);
      }else{
        digitalWrite(STEP_8, LOW);
      }
      digitalWrite(SYNC_OUT, LOW);
    break;
    case  4:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      if(stp_tmp > 10){
      digitalWrite(STEP_1, HIGH);
      }else{
        digitalWrite(STEP_1, LOW);
      }
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, HIGH);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  5:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_2, HIGH);
      }else{
        digitalWrite(STEP_2, LOW);
      }
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, HIGH);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, LOW);
    break;
    case  6:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_3, HIGH);
      }else{
        digitalWrite(STEP_3, LOW);
      }
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, HIGH);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  7:
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      if(stp_tmp > 10){
      digitalWrite(STEP_4, HIGH);
      }else{
        digitalWrite(STEP_4, LOW);
      }
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, HIGH);
      digitalWrite(SYNC_OUT, LOW);
    break;
    }
  if(tmp_bpm < 500){
//    kEnvelope.set(attack_ms,decay_ms);
//
//    kEnvelope.start();
    kDelay.start(tmp_bpm);
    stp_cnt++;
  }
  if(tmp_bpm > 500){
    time1  = pulseIn(SYNC_IN, HIGH);
    time2  = pulseIn(SYNC_IN, LOW);
    syncIn = time2/1000 + time1/100;
//    kEnvelope.start(attack_ms,decay_ms);
    kDelay.start(syncIn);
    stp_cnt++;
    }
  }else{
    stp_cnt = 0;
  }
  }

Evgain = (int) kEnvelope.next();

//Serial.print("1:");
 Serial.println(cutoff_freq);
// Serial.print("2:");
// Serial.println(mozziAnalogRead(1));
// Serial.print("3:");
// Serial.println(mozziAnalogRead(2));
// Serial.print("4:");
// Serial.println(mozziAnalogRead(3));
// Serial.print("5:");
// Serial.println(mozziAnalogRead(4));
// Serial.print("6:");
// Serial.println(mozziAnalogRead(5));

}

int updateAudio(){
int asig = (aSin.next()*sinGain[0]+aTri.next()*triGain[0]+aSaw.next()*sawGain[0]+aSqu.next()*squGain[0])>>8; 
int asig2 = (aSin2.next()*sinGain[1]+aTri2.next()*triGain[1]+aSaw2.next()*sawGain[1]+aSqu2.next()*squGain[1])>>8;
int noise = aNoise.next()*noiseGain >>8;
int premaster = (asig + asig2 + noise)>>4;
int pm = Evgain*premaster >>8;
int master = lpf.next(pm);
//int lpfilterd = lpf.next(premaster);
//int master = aSmoothGain.next(Evgain)*lpfilterd >>14;
if (Lfo_rate > 29){
return (int)((long)((long) master * aLfo.next()) >> 16);
}else if(Lfo_rate < 29){
  Lfo_rate = 0;
return (int) master;
}
}

void loop(){
  audioHook();
}