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
#include <Portamento.h>
#include <IntMap.h>

#define CONTROL_RATE 64

Oscil <PINKNOISE8192_NUM_CELLS, AUDIO_RATE> aNoise(PINKNOISE8192_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);
//Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
//Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_NO_ALIAS_2048_DATA);

//Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
//Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri2(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu2(SQUARE_NO_ALIAS_2048_DATA);

Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> aSin3(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, CONTROL_RATE> aTri3(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, CONTROL_RATE> aSaw3(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, CONTROL_RATE> aSqu3(SQUARE_NO_ALIAS_2048_DATA);

Portamento <CONTROL_RATE>aPortamento;

Line <unsigned int> aLfo;

LowPassFilter lpf;

EventDelay kDelay;
EventDelay kDelay2;

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
int Bpm;
int tmp_read;
int tmp_bpm = 0;
int stp_tmp = 0;
int stp_num = 0;
int tmp_scale = 0;
unsigned int attack_ms = 10;
unsigned int decay_ms = 10;
int stp_cnt = 0;
//int syn_cnt = 0;
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
byte pageState[4] = {0,0,0,0};
byte Flag[4][4] = {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};
//vco
int curve[2][6];
int input[2] = {0,0};
byte sinGain[2];
byte triGain[2];
byte sawGain[2];
byte squGain[2];
//LPF
uint8_t Reso = 0;
uint8_t cutoff_freq = 50;
//portamento
unsigned int PortT = 0;
//LFO
int Lfo_rate = 0;
int Lfo_s;
byte Lfo_form = 0;
unsigned int Lfo;
byte noiseGain = 0;
//syncin
unsigned long syncIn;
volatile unsigned long time1;
volatile unsigned long time2;
//mapping
const IntMap readIntMap(0, 1023, 0, 15);
const IntMap nobIntMap(0,1023,0,127);
const IntMap bpmIntMap(0,127,10,600);
const IntMap stepIntMap(0,127,1,12);
const IntMap atkIntMap(0,127,10,1000);
const IntMap dcyIntMap(0,127,1,4000);
const IntMap resIntMap (0, 127, 0, 200);
const IntMap cutoffIntMap (0, 127, 50, 255);
//const IntMap sclIntMap(0,127,0,8);
//const IntMap keysIntMap(0,127,0,11);
//const IntMap octIntMap(0,127,0,3);
//const IntMap portIntMap(0,127,0,500);
//const IntMap lfoFIntMap(0,127,0,3);
//const IntMap lfoRIntMap(0,127,0,1400);
//const IntMap dtnIntMap(0,127,0,500);
//const IntMap noiseIntMap(0,127,0,255);
//const IntMap cerv0IntMap(0, 42, 254, 0);
//const IntMap cerv1IntMap(0, 42, 0, 254);
//const IntMap cerv2IntMap(43, 84, 254, 0);
//const IntMap cerv3IntMap(43, 84, 0, 254);
//const IntMap cerv4IntMap(85, 127, 254, 0);
//const IntMap cerv5IntMap(85, 127, 0, 254);
volatile boolean mEasure = false;
byte j = 1;

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250);
//  Serial.begin(115200);
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
  valNob[0][0] = 50;
//  valNob[0][1] = 8;
//  stp_tmp = 8;
//  stp_num = 8;
  tmp_bpm = 600;
//  valNob[1][2] = 190;
//  valNob[1][3] = 240;
//  Reso = 100;
//  cutoff_freq = 100;
////  valNob[0][3] = 1575;
//  decay_ms = 100;
attachInterrupt(0,synC,CHANGE);
}

void updateControl() {  

//pageSwitcher
SWITCH2 = mozziAnalogRead(A5);
tmp_read = readIntMap(mozziAnalogRead(A4));

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
//   realNob[i][0] = map(mozziAnalogRead(nob0),0,1023,0,127);
//   realNob[i][1] = map(mozziAnalogRead(nob1),0,1023,0,127);
//   realNob[i][2] = map(mozziAnalogRead(nob2),0,1023,0,127);
//   realNob[i][3] = map(mozziAnalogRead(nob3),0,1023,0,127);
   realNob[i][0] = nobIntMap(mozziAnalogRead(nob0));
   realNob[i][1] = nobIntMap(mozziAnalogRead(nob1));
   realNob[i][2] = nobIntMap(mozziAnalogRead(nob2));
   realNob[i][3] = nobIntMap(mozziAnalogRead(nob3));
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
tmp_bpm = 30000/bpmIntMap(valNob[0][0]);
stp_tmp = stepIntMap(valNob[0][1]);
attack_ms = atkIntMap(valNob[0][2]);
decay_ms = dcyIntMap(valNob[0][3]);
//tmp_bpm = 30000/map(valNob[0][0],0,127,10,600);
//stp_tmp = map(valNob[0][1],0,127,1,12);
//attack_ms = map(valNob[0][2],0,127,10,1000);
//decay_ms = map(valNob[0][3],0,127,1,4000);
if(stp_tmp >= 8){
  stp_num = 8;
}else{
  stp_num = stp_tmp;
}

//page2
input[0] = valNob[1][0];//VCO1
input[1] = valNob[1][1];//VCO2
Reso = resIntMap(valNob[1][2]);
cutoff_freq = cutoffIntMap(valNob[1][3]);
//Reso = map(valNob[1][2],0, 127, 0, 200);
//cutoff_freq = map(valNob[1][3],0, 127, 50, 255);

//page3
//tmp_scale = sclIntMap(valNob[2][0]);
//keySft = keysIntMap(valNob[2][1]);
//Octv = octIntMap(valNob[2][2]);
//PortT = portIntMap(valNob[2][3]);
switch (j){
  case 1:
  tmp_scale = map(valNob[2][0],0,127,0,8);
  break;
  case 2:
  keySft = map(valNob[2][1],0,127,0,11);
  break;
  case 3:
  Octv = map(valNob[2][2],0,127,0,3);
  break;
  case 4:
  PortT = map(valNob[2][3],0, 127, 0, 500);
  Octv *= 12;
  break;
  case 5:
  Lfo_form = map(valNob[3][0],0,127,0,3);
  break;
  case 6:
  Lfo_rate = map(valNob[3][1],0, 127, 25, 55);
  break;
  case 7:
  Dtn = map(valNob[3][2],0,127,0,500);
  break;
  case 8:
  noiseGain = map(valNob[3][3],0,127,0,200);
  break;
}
j++;
if(j>8){
  j=0;
  }

//page4
//Lfo_form = lfoFIntMap(valNob[3][0]);
//Lfo_rate = lfoRIntMap(valNob[3][1]);
//Lfo_rate = (float)Lfo_rate*0.1;
//Dtn = dtnIntMap(valNob[3][2]);
//Lfo_form = map(valNob[3][0],0,127,0,3);
//Lfo_rate = map(valNob[3][1],0, 127, 25, 55);
//Dtn = map(valNob[3][2],0,127,0,50);
Dtn = (float)Dtn*0.1;
if (valNob[3][2] == 0){
  Dtn = 0;
}
//noiseGain = noiseIntMap(valNob[3][3]);
//noiseGain = map(valNob[3][3],0,127,0,255);

//VCO
//curve[0][0] = cerv0IntMap(valNob[1][0]);
//curve[0][1] = cerv1IntMap(valNob[1][0]);
//curve[0][2] = cerv2IntMap(valNob[1][0]);
//curve[0][3] = cerv3IntMap(valNob[1][0]);
//curve[0][4] = cerv4IntMap(valNob[1][0]);
//curve[0][5] = cerv5IntMap(valNob[1][0]);
curve[0][0] = map(valNob[1][0], 0, 63, 255, 0);
curve[0][1] = map(valNob[1][0], 0, 63, 0, 255);
curve[0][2] = map(valNob[1][0], 64, 127, 255, 0);
//curve[0][3] = map(valNob[1][0], 43, 84, 0, 255);
//curve[0][4] = map(valNob[1][0], 85, 127, 255, 0);
//curve[0][5] = map(valNob[1][0], 85, 127, 0, 255);


if((0<=input[0])&&(input[0]<63)){
  sinGain[0] = curve[0][0];
}else{
  sinGain[0] = 0;
}

if((0<=input[0])&&(input[0]<63)){
  triGain[0] = curve[0][1];
}

if((64<input[0])&&(input[0]<126)){
  triGain[0] = curve[0][2];
}else if(input[0]>127){
  triGain[0] = 0;
}

//if((43<input[0])&&(input[0]<84)){
//  sawGain[0] = curve[0][3];
//}else if(input[0]<42){
//  sawGain[0] = 0;
//}
//
//if((85<input[0])&&(input[0]<=127)){
//  sawGain[0] = curve[0][4];
//}
//
//if((85<input[0])&&(input[0]<=127)){
//  squGain[0] = curve[0][5];
//}else if(input[0]<84){
//  squGain[0] = 0;
//}

//curve[1][0] = cerv0IntMap(valNob[1][1]);
//curve[1][1] = cerv1IntMap(valNob[1][1]);
//curve[1][2] = cerv2IntMap(valNob[1][1]);
//curve[1][3] = cerv3IntMap(valNob[1][1]);
//curve[1][4] = cerv4IntMap(valNob[1][1]);
//curve[1][5] = cerv5IntMap(valNob[1][1]);
//curve[1][0] = map(valNob[1][1], 0, 42, 255, 0);
//curve[1][1] = map(valNob[1][1], 0, 42, 0, 255);
//curve[1][2] = map(valNob[1][1], 43, 84, 255, 0);
curve[1][3] = map(valNob[1][1], 0, 63, 0, 255);
curve[1][4] = map(valNob[1][1], 64, 127, 255, 0);
curve[1][5] = map(valNob[1][1], 64, 127, 0, 255);

//if((0<=input[1])&&(input[1]<42)){
//  sinGain[1] = curve[1][0];
//}else{
//  sinGain[1] = 0;
//}
//
//if((0<=input[1])&&(input[1]<42)){
//  triGain[1] = curve[1][1];
//}
//
//if((43<input[1])&&(input[1]<84)){
//  triGain[1] = curve[1][2];
//}else if(input[1]>85){
//  triGain[1] = 0;
//}

if((1<input[1])&&(input[1]<63)){
  sawGain[1] = curve[1][3];
}else if(input[1]<0){
  sawGain[1] = 0;
}

if((64<input[1])&&(input[1]<=127)){
  sawGain[1] = curve[1][4];
}

if((64<input[1])&&(input[1]<=127)){
  squGain[1] = curve[1][5];
}else if(input[1]<63){
  squGain[1] = 0;
}

syncIn = (time2>>1)/1000;

if(tmp_bpm < 500){
 Bpm = tmp_bpm;
}else if(tmp_bpm > 500){
  Bpm = syncIn;
  }

kDelay.set(Bpm);

kDelay2.set(Bpm);
int stp_cnt2;
if(kDelay2.ready()){
  if(stp_cnt2 < 2){
    switch(stp_cnt2){
    case  0:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  1:

      digitalWrite(SYNC_OUT, LOW);
    break;
    }
    kDelay2.start();
    stp_cnt2++;
  }else{
    stp_cnt2 = 0;
  }
  }
  
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
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
//      digitalWrite(SYNC_OUT, HIGH);
    break;
    }
    kEnvelope.set(attack_ms,decay_ms);
    kEnvelope.start();
    kDelay.start();
    stp_cnt++;
  }else{
    stp_cnt = 0;
  }
  }
aNoise.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
aTri.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
//aSaw.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
//aSqu.setFreq(mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft);
//aSin2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
//aTri2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
aSaw2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);
aSqu2.setFreq((mtof(scaleMap[tmp_scale][tmp_read]) + Octv + keySft)+Dtn);

if (PortT > 10){
  aPortamento.setTime(PortT);
  aPortamento.start(scaleMap[tmp_scale][tmp_read]);
  aSin.setFreq_Q16n16(aPortamento.next());
  aTri.setFreq_Q16n16(aPortamento.next());
//  aSaw.setFreq_Q16n16(aPortamento.next());
//  aSqu.setFreq_Q16n16(aPortamento.next());

//  aSin2.setFreq_Q16n16(aPortamento.next());
//  aTri2.setFreq_Q16n16(aPortamento.next());
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
//
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

Lfo = (128 + Lfo_s)<<8;
aLfo.set(Lfo, AUDIO_RATE / CONTROL_RATE);

Evgain = (int) kEnvelope.next();

//Filter
lpf.setResonance(Reso);
lpf.setCutoffFreq(cutoff_freq);

//Serial.print("1:");
// Serial.println(time1);
// Serial.print("2:");
// Serial.println(time2);
// Serial.print("3:");
// Serial.println(syncIn);
// Serial.print("4:");
// Serial.println(bpm);
// Serial.print("5:");
// Serial.println(mozziAnalogRead(4));
// Serial.print("6:");
// Serial.println(mozziAnalogRead(5));

}

int updateAudio(){
//int asig = (((aSin.next()*sinGain[0])>>8)+((aTri.next()*triGain[0])>>8)); 
//int asig2 = (((aSaw2.next()*sawGain[1])>>8)+((aSqu2.next()*squGain[1])>>8));
int asig = ((aSin.next()*sinGain[0])+(aTri.next()*triGain[0]))>>8; 
int asig2 = ((aSaw2.next()*sawGain[1])+(aSqu2.next()*squGain[1]))>>8;
int noise = (aNoise.next()*noiseGain)>>8;
int preMaster = (asig + asig2)>>1;//(asig + asig2)>>2 or >>1
int preMaster2 = (preMaster + noise)>>1;//(preMaster + noise)>>1
int pm = (Evgain*preMaster2)>>8;
if (Lfo_rate > 29){
pm = (int)((long)((long) pm * aLfo.next()) >> 15);
}else if(Lfo_rate < 29){
  Lfo_rate = 0;
  }
int master = lpf.next(pm);

return (int) master;
}

void loop(){
  audioHook();
}

void synC(){
 if (!mEasure) {
  time1 = mozziMicros();
  mEasure = true;
}
else{
if(digitalRead(SYNC_IN) == 1){
time2 = mozziMicros() - time1;
mEasure = false;
 }
}
}
