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

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_NO_ALIAS_2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri2(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu2(SQUARE_NO_ALIAS_2048_DATA);
LowPassFilter lpf;

#define CONTROL_RATE 64

#include <EventDelay.h>
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



int scaleMap[6][7] = {
  {60, 62, 64, 65, 67, 69, 71}, //major
  {60, 62, 63, 65, 67, 69, 70}, //dorian
  {60, 62, 64, 67, 69, 72, 74}, //penta
  {60, 63, 65, 67, 70, 72, 75}, //minor penta 
  {60, 62, 63, 65, 67, 68, 70}, //minor
  {60, 61, 64, 65, 67, 68, 71} //gypsy
};

//seq
int stp_cnt;
//envelope gaim
int gain;
//pages
int valNob[2][4] = {
  {0,0,0,0},
  {0,50,50,100}
};
int pageState0 = 0;
int pageState1 = 0;
byte Flag[2][4] = {
  {0,0,0,0},
  {0,0,0,0}
};
int const nob[4] = {A0,A1,A2,A3};

//vco
int curve[2][6];
int input[2] = {0,0};
int sinGain[2];
int triGain[2];
int sawGain[2];
int squGain[2];
int preMaster1;
int preMaster2;

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250);
  lpf.setResonance(10);
  //Serial.begin(9600);
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
}

void updateControl() {
  
// from step to A4
int  tmp_read = map(mozziAnalogRead(A4),0, 1023, 0, 6);

//pageSwitcher
if(digitalRead(SWITCH1) == LOW){
      pageState0 = 1;
    }else{
      pageState1 = 1;
    }

//page1
if(pageState0 == 1){
    for(int i=0; i<4; i++){
      if(Flag[0][i] == 0){
      if(valNob[0][i] == map(mozziAnalogRead(nob[i]),0,1023,0,127)){
      Flag[0][i] = 1;
        }
      }else{
        valNob[0][i] = map(mozziAnalogRead(nob[i]),0,1023,0,127);
      }
    }
    pageState0 = 0;
    for(int i=0; i<4; i++){
      Flag[1][i] = 0;
    }
  }

//BPM
int tmp_bpm = map(valNob[0][0],0,127,0,500);

//step_num
int stp_num = map(valNob[0][1],0,127,0,16);

//step_num揺らぎ軽減
if (stp_num >= 14 && stp_num < 16){
    stp_num++;
}

//page2
if(pageState1 == 1){
    for(int i=0; i<4; i++){
      if(Flag[1][i] == 0){
      if(valNob[1][i] == map(mozziAnalogRead(nob[i]),0,1023,0,127)){
      Flag[1][i] = 1;
        }
      }else{
        valNob[1][i] = map(mozziAnalogRead(nob[i]),0,1023,0,127);
      }
    }
    pageState1 = 0;
    for(int i=0; i<4; i++){
      Flag[0][i] = 0;
    }
  }

//scale
int tmp_scale = map(valNob[1][0],0,127,0,5);

//AD(noSR)
unsigned int attack = 10;
unsigned int decay = map(valNob[0][2],0,127,0,5000);

  input[0] = map(valNob[1][1],0,127,0,1023);
  curve[0][0] = map(valNob[1][1], 0, 341, 100, 0);
  curve[0][1] = map(valNob[1][1], 0, 341, 0, 100);
  curve[0][2] = map(valNob[1][1], 342, 682, 100, 0);
  curve[0][3] = map(valNob[1][1], 342, 682, 0, 100);
  curve[0][4] = map(valNob[1][1], 683, 1023, 100, 0);
  curve[0][5] = map(valNob[1][1], 683, 1023, 0, 100);

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

  input[1] = map(valNob[1][2],0,127,0,1023);
  curve[1][0] = map(valNob[1][2], 0, 341, 100, 0);
  curve[1][1] = map(valNob[1][2], 0, 341, 0, 100);
  curve[1][2] = map(valNob[1][2], 342, 682, 100, 0);
  curve[1][3] = map(valNob[1][2], 342, 682, 0, 100);
  curve[1][4] = map(valNob[1][2], 683, 1023, 100, 0);
  curve[1][5] = map(valNob[1][2], 683, 1023, 0, 100);

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

preMaster1 = map(valNob[1][3],0, 127, 0, 255);
preMaster2 = map(valNob[1][3],0, 127, 255, 0);

//  input[2] = map(valNob[1][3],0, 1023, 0, 1023);
//  curve[2][0] = map(valNob[1][3],0, 1023, 0, 255);
//  curve[2][1] = map(valNob[1][3],0, 1023, 255, 0);
//  
//  if((0<=input)&&(input<=1023)){
//      preMaster1 = curve[2][0];
//  }
//  
//  if((0<=input)&&(input<=1023)){
//      preMaster2 = curve[2][1];
//  }else{
//      preMaster2 = 0;
//  }
  
aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aTri.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSaw.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSqu.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

aSin2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aTri2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSaw2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
aSqu2.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

gain = (int) kEnvelope.next(); //各ステップからaSin通って出てきたところでnext

//Filter
uint8_t cutoff_freq = map(valNob[0][3],0, 1023, 10, 255);
  lpf.setCutoffFreq(cutoff_freq);


digitalWrite(SYNC_OUT, LOW);

//if(tmp_bpm > 4500){//ピン未宣言
  //シンクイン途中
//  if(mozziAnalogRead(A5) >4){
    //stp_cnt++;
//  }

//if(tmp_bpm < 4500){
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
      kEnvelope.start(attack,decay);
    break;
    case  1:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  2:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, HIGH);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  3:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  4:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, HIGH);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay); //エンベロープスタート
    break;
    case  5:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  6:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, HIGH);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  7:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  8:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, HIGH);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  9:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  10:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, HIGH);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  11:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  12:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, HIGH);
      digitalWrite(STEP_8, LOW);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  13:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    case  14:
      digitalWrite(STEP_1, LOW);
      digitalWrite(STEP_2, LOW);
      digitalWrite(STEP_3, LOW);
      digitalWrite(STEP_4, LOW);
      digitalWrite(STEP_5, LOW);
      digitalWrite(STEP_6, LOW);
      digitalWrite(STEP_7, LOW);
      digitalWrite(STEP_8, HIGH);
      digitalWrite(SYNC_OUT, HIGH);
      kEnvelope.start(attack,decay);
    break;
    case  15:
      digitalWrite(SYNC_OUT, HIGH);
    break;
    }
      kDelay.start(tmp_bpm);
    stp_cnt++;
  }else{
    stp_cnt = 0;
  }
  }

//Serial.print("bpm: ");
//  Serial.println(vol_val);
//    Serial.println(map(valNob[0][1],0,1023,0,16));
  // Serial.println(preMaster1);
  // Serial.println(mozziAnalogRead(A4));
  // Serial.println(valNob[1][3]);
}

int updateAudio(){
//  int asig = gain*(lpf.next(aSin.next()))>>8;

int asig = gain*(lpf.next(((aSin.next()*sinGain[0]+aTri.next()*triGain[0]+aSaw.next()*sawGain[0]+aSqu.next()*squGain[0])>>3)>>8))>>8;
int asig2 = gain*(lpf.next(((aSin.next()*sinGain[1]+aTri.next()*triGain[1]+aSaw.next()*sawGain[1]+aSqu.next()*squGain[1])>>3)>>8))>>8;
int master = (asig*preMaster1 + asig2*preMaster2)>>4;
return (int) master;
}

void loop(){
  audioHook();
}
