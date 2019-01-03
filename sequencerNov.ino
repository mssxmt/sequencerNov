#include <MozziGuts.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <Ead.h>
#include <mozzi_midi.h>
#include <tables/sin2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <LowPassFilter.h>
#include <Line.h>
#include <EventDelay.h>
#include <Portamento.h>

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

#define CONTROL_RATE 128

EventDelay kDelay;

Ead kEnvelope(CONTROL_RATE);

// Digital
#define  SYNC_OUT          13
#define  STEP_1            12
#define  STEP_2            11
#define  STEP_3            10
#define  STEP_4            8
#define  STEP_5            7
#define  STEP_6            6
#define  STEP_7            5
#define  STEP_8            4
#define  SWITCH1           3
#define  SYNC_IN           2

uint8_t scaleMap[6][7] = {
  {60, 62, 64, 65, 67, 69, 71}, //major
  {60, 62, 63, 65, 67, 69, 70}, //dorian
  {60, 62, 64, 67, 69, 72, 74}, //penta
  {60, 63, 65, 67, 70, 72, 75}, //minor penta 
  {60, 62, 63, 65, 67, 68, 70}, //minor
  {60, 61, 64, 65, 67, 68, 71} //gypsy
};

int tmp_read;
int tmp_bpm = 0;
int stp_tmp = 0;
int stp_num = 0;
int tmp_scale = 0;
unsigned int attack = 0;
unsigned int decay = 0;
int stp_cnt;
//envelope gaim
int gain;
//pages
int const nob[4] = {A0,A1,A2,A3};
int realNob[3][4];
int valNob[3][4] = {
  {30,200,10,3000},
  {50,50,0,26},
  {500,8,0,10}
};
int pageState[3] = {0,0,0};
byte Flag[3][4] = {
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
uint8_t cutoff_freq = 0;
//portamento
unsigned int PortT = 0;
//LFO
int Lfo_rate = 0;
int Lfo_s;
int Lfo_form = 0;
unsigned int Lfo;
//syncin
unsigned long syncIn;
unsigned long time1;
unsigned long time2;

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250);
  Serial.begin(9600);
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
int SWITCH2 = mozziAnalogRead(A5);
tmp_read = map(mozziAnalogRead(A4),0, 1023, 0, 6);
if((digitalRead(SWITCH1) == LOW) && (SWITCH2 < 800)){
  pageState[0] = 1;
}else if((digitalRead(SWITCH1) == HIGH) && (SWITCH2 < 800)){
  pageState[1] = 1;
}else if(((digitalRead(SWITCH1) == LOW) && (SWITCH2 > 800))||((digitalRead(SWITCH1) == HIGH) && (SWITCH2 > 800))){
  pageState[2] = 1;
}

for(int i=0; i<4; i++){
  realNob[0][i] = mozziAnalogRead(nob[i]);
  realNob[1][i] = mozziAnalogRead(nob[i]);
  realNob[2][i] = mozziAnalogRead(nob[i]);
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
  }
}

//BPM//_bpm = 1/tmp_bpm*60000
tmp_bpm = 30000/map(valNob[2][0],0,1023,1,600);
stp_tmp = map(valNob[2][1],0,1023,1,12);
tmp_scale = map(valNob[2][2],0,1023,0,5);
PortT = map(valNob[2][3],0, 1023, 0, 200);
if(stp_tmp >= 8){
  stp_num = 8;
}else{
  stp_num = stp_tmp;
}

input[0] = map(valNob[1][0],0,1023,0,1023);//VCO1
input[1] = map(valNob[1][1],0,1023,0,1023);//VCO2
Lfo_form = map(valNob[1][2],0,1023,0,4);
Lfo_rate = map(valNob[1][3],0, 1023, 25, 55);

Reso = map(valNob[0][1],0, 1023, 0, 255);
cutoff_freq = map(valNob[0][0],0, 1023, 30, 255);
attack = map(valNob[0][2],0,1023,10,100);
decay = map(valNob[0][3],0,1023,0,5000);

//VCO
curve[0][0] = map(valNob[1][0], 0, 341, 100, 0);
curve[0][1] = map(valNob[1][0], 0, 341, 0, 100);
curve[0][2] = map(valNob[1][0], 342, 682, 100, 0);
curve[0][3] = map(valNob[1][0], 342, 682, 0, 100);
curve[0][4] = map(valNob[1][0], 683, 1023, 100, 0);
curve[0][5] = map(valNob[1][0], 683, 1023, 0, 100);

if((0<=input[0])&&(input[0]<342)){
  sinGain[0] = curve[0][0];
}else{
  sinGain[0] = 0;
}

if((0<=input[0])&&(input[0]<342)){
  triGain[0] = curve[0][1];
}

if((341<input[0])&&(input[0]<683)){
  triGain[0] = curve[0][2];
}else if(input[0]>683){
  triGain[0] = 0;
}

if((341<input[0])&&(input[0]<683)){
  sawGain[0] = curve[0][3];
}else if(input[0]<341){
  sawGain[0] = 0;
}

if((682<input[0])&&(input[0]<=1023)){
  sawGain[0] = curve[0][4];
}

if((682<input[0])&&(input[0]<=1023)){
  squGain[0] = curve[0][5];
}else if(input[0]<682){
  squGain[0] = 0;
}

curve[1][0] = map(valNob[1][1], 0, 341, 100, 0);
curve[1][1] = map(valNob[1][1], 0, 341, 0, 100);
curve[1][2] = map(valNob[1][1], 342, 682, 100, 0);
curve[1][3] = map(valNob[1][1], 342, 682, 0, 100);
curve[1][4] = map(valNob[1][1], 683, 1023, 100, 0);
curve[1][5] = map(valNob[1][1], 683, 1023, 0, 100);

if((0<=input[1])&&(input[1]<342)){
  sinGain[1] = curve[1][0];
}else{
  sinGain[1] = 0;
}

if((0<=input[1])&&(input[1]<342)){
  triGain[1] = curve[1][1];
}

if((341<input[1])&&(input[1]<683)){
  triGain[1] = curve[1][2];
}else if(input[1]>683){
  triGain[1] = 0;
}

if((341<input[1])&&(input[1]<683)){
  sawGain[1] = curve[1][3];
}else if(input[1]<341){
  sawGain[1] = 0;
}

if((682<input[1])&&(input[1]<=1023)){
  sawGain[1] = curve[1][4];
}

if((682<input[1])&&(input[1]<=1023)){
  squGain[1] = curve[1][5];
}else if(input[1]<682){
  squGain[1] = 0;
}

aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aTri.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSaw.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSqu.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

aSin2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aTri2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSaw2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSqu2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

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
}

//LFO
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

aSin3.setFreq(Lfo_rate);
aTri3.setFreq(Lfo_rate);
aSaw3.setFreq(Lfo_rate);
aSqu3.setFreq(Lfo_rate);

Lfo = (128u + Lfo_s)<<8;
aLfo.set(Lfo, AUDIO_RATE / CONTROL_RATE);

//AD(noSR)
gain = (int) kEnvelope.next();

//Filter
lpf.setResonance(Reso);
lpf.setCutoffFreq(cutoff_freq);

digitalWrite(SYNC_OUT, LOW);

if(kDelay.ready()){
  if(stp_cnt < stp_num){
    switch(stp_cnt){
    case  0:
      digitalWrite(STEP_1, HIGH);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  1:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, HIGH);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  2:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, HIGH);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  3:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, HIGH);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
//      kEnvelope.start(attack,decay);
    break;
    case  4:
      digitalWrite(STEP_1, LOW);
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
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, HIGH);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  6:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, HIGH);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  7:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, HIGH);
      digitalWrite(SYNC_OUT, HIGH);
    break;
    }
  if(tmp_bpm < 400){
    kDelay.start(tmp_bpm);
    kEnvelope.start(attack,decay);
    stp_cnt++;
  }
  if(tmp_bpm > 400){
    time1  = pulseIn(SYNC_IN, HIGH);
    time2  = pulseIn(SYNC_IN, LOW);
    syncIn = time2/1000 + time1/100;
    kDelay.start(syncIn);
    stp_cnt++;
    kEnvelope.start(attack,decay);
    }
  }else{
    stp_cnt = 0;
  }
  }
//Serial.print("bpm: ");
//  Serial.println(pageState[0]);
//    Serial.println(map(valNob[0][1],0,1023,0,16));
//  Serial.println(PortT);
//  Serial.println(mozziAnalogRead(A3));
//  Serial.println(valNob[2][2]);
}

int updateAudio(){
int asig = gain*(lpf.next(((aSin.next()*sinGain[0]+aTri.next()*triGain[0]+aSaw.next()*sawGain[0]+aSqu.next()*squGain[0])>>2)>>8))>>6;
int asig2 = gain*(lpf.next(((aSin2.next()*sinGain[1]+aTri2.next()*triGain[1]+aSaw2.next()*sawGain[1]+aSqu2.next()*squGain[1])>>2)>>8))>>6;
int master = (asig + asig2)>>2;
if (Lfo_rate > 30){
return (int)((long)((long) master * aLfo.next()) >> 16);
}else if(Lfo_rate < 30){
return (int) master;
}
}

void loop(){
  audioHook();
}