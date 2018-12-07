#include <MozziGuts.h> // Mozziの基本ヘッダファイル
#include <Oscil.h> // オシレータのテンプレート
#include <EventDelay.h>
#include <Ead.h>
#include <mozzi_midi.h> // MozziでMIDIを用いるためのヘッダ
#include <tables/sin2048_int8.h> // サイン波のテーブル
#include <LowPassFilter.h>

// SIN2048_DATA2048のデータをaSinに格納
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
LowPassFilter lpf;

// コントロールレートをあらかじめ定義
#define CONTROL_RATE 64

#include <EventDelay.h>
EventDelay kDelay;
Ead kEnvelope(CONTROL_RATE);


// シーケンサーステップ.(Digital)
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
int valPOT[2][4];
int pageState0 = 0;
int pageState1 = 0;
byte Flag[2][4];

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250);
 lpf.setResonance(0);
  Serial.begin(9600);
  
}

void updateControl() {
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

//if(digitalRead(SWITCH2) == HIGH){//ピン未宣言
  //シンクイン途中
//  if(mozziAnalogRead(A5) >4){
    //stp_cnt++;
//  }


// from step to A4
int  tmp_read = map(mozziAnalogRead(A4),0, 1023, 0, 6);

if(digitalRead(SWITCH1) == LOW){
      pageState0 = 1;
    }else{
      pageState1 = 1;
    }

//page1
if(pageState0 == 1){

  if(Flag[0][0] == 0){
    if(valPOT[0][0] == map(mozziAnalogRead(A0),0,1023,0,500)){//bpm
    Flag[0][0] = 1;
    }
  }else{
    valPOT[0][0] = map(mozziAnalogRead(A0),0,1023,0,500);
  }

  if(Flag[0][1] == 0){
    if(valPOT[0][1] == map(mozziAnalogRead(A1),0,1023,0,16)){//step
    Flag[0][1] = 1;
    }
  }else{
    valPOT[0][1] = map(mozziAnalogRead(A1),0,1023,0,16);
  }

if(Flag[0][2] == 0){
    if(valPOT[0][2] == map(mozziAnalogRead(A2),0,1023,0,2550)){//decay
    Flag[0][2] = 1;
    }
  }else{
    valPOT[0][2] = map(mozziAnalogRead(A2),0,1023,0,2550);
  }

if(Flag[0][3] == 0){
    if(valPOT[0][3] == map(mozziAnalogRead(A3),0, 1023, 0, 255)){//filter
    Flag[0][3] = 1;
    }
  }else{
    valPOT[0][3] = map(mozziAnalogRead(A3),0, 1023, 10, 255);
  }

  pageState0 = 0;
  
} else {
  for(int i =0; i<4; i++){
  Flag[0][i] = 0;
  }
}

//BPM
int tmp_bpm = valPOT[0][0];

//step_num
int stp_num = valPOT[0][1];

//step_num揺らぎ軽減
if (stp_num >= 14 || stp_num < 16){
    stp_num++;
}

//page2
if(pageState1 == 1){

  if(Flag[1][0] == 0){
    if(valPOT[1][0] == map(mozziAnalogRead(A0),0,1023,0,5)){//scale
    Flag[1][0] = 1;
    }
  }else{
    valPOT[1][0] = map(mozziAnalogRead(A0),0,1023,0,5);
  }

  if(Flag[1][1] == 0){
    if(valPOT[1][1] == map(mozziAnalogRead(A1),0,1023,0,255)){//未設定
    Flag[1][1] = 1;
    }
  }else{
    valPOT[1][1] = map(mozziAnalogRead(A1),0,1023,0,255);
  }

  if(Flag[1][2] == 0){
    if(valPOT[1][2] == map(mozziAnalogRead(A1),0,1023,0,255)){//未設定
    Flag[1][2] = 1;
    }
  }else{
    valPOT[1][2] = map(mozziAnalogRead(A1),0,1023,0,255);
  }

  if(Flag[1][3] == 0){
    if(valPOT[1][3] == map(mozziAnalogRead(A1),0,1023,0,255)){//未設定
    Flag[1][3] = 1;
    }
  }else{
    valPOT[1][3] = map(mozziAnalogRead(A1),0,1023,0,255);
  }

  pageState1 = 0;
  
} else {
  for(int i =0; i<4; i++){
  Flag[1][i] = 0;
  }
}

//scale
//  tmp_scale = map(valPOT[0][1],0,1023,0,5);
int tmp_scale = valPOT[1][0];

unsigned int attack = 10;
// decay = map(valPOT[1][1],0,1023,0,255);
unsigned int decay = valPOT[0][2];



aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

gain = (int) kEnvelope.next(); //各ステップからaSin通って出てきたところでnext

//Filter
byte cutoff_freq = valPOT[0][3];
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
//  Serial.println(mozziAnalogRead(A0));
    Serial.println(valPOT[0][3]);
  Serial.println(map(mozziAnalogRead(A3),0, 1023, 10, 255));
//  Serial.println(Flag[0]);
//  Serial.println(valPOT[1][0]);
}

int updateAudio(){
int asig = gain*(lpf.next(aSin.next()))>>8;
return (int) asig;
}

void loop(){
  audioHook();
}