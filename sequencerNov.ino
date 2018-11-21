#include <MozziGuts.h> // Mozziの基本ヘッダファイル
#include <Oscil.h> // オシレータのテンプレート
#include <EventDelay.h>
#include <Ead.h>
#include <mozzi_midi.h> // MozziでMIDIを用いるためのヘッダ
#include <tables/sin2048_int8.h> // サイン波のテーブル

// SIN2048_DATA2048のデータをaSinに格納
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);

// コントロールレートをあらかじめ定義
#define CONTROL_RATE 64

#include <EventDelay.h>
EventDelay kDelay;
Ead kEnvelope(CONTROL_RATE);



// アナログピンマッピング
#define SYNC_CONTROL         2 // For ribbon controler(SoftPot)
#define FUNC_SELECTOR        0
#define SCALE_SELECTOR       1
#define SYNC_IN              3
// シーケンサーステップ.(Digital)
#define  SYNC_OUT          13
#define  STEP_1            12
#define  STEP_2            11
#define  STEP_3            10
#define  STEP_4            8 //D9はオーディオ出力に使うから飛ばし
#define  STEP_5            7
#define  STEP_6            6
#define  STEP_7            5
#define  STEP_8            4



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

//envelope後でpotにアサイン
int gain;
unsigned int attack = 100;
unsigned int decay = 10;

void setup(){
  startMozzi(CONTROL_RATE);
  kDelay.start(250); 
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
  pinMode(SYNC_IN,INPUT);
 
  //シンクイン途中
  if(mozziAnalogRead(SYNC_IN) >4){
    //stp_cnt++;
  }
  
//scale
uint8_t  tmp_scale = map(mozziAnalogRead(SCALE_SELECTOR),0, 1023, 0, 5);    
//BPM
uint16_t tmp_bpm = map(mozziAnalogRead(FUNC_SELECTOR),0,1023,0,500);
kDelay.set(tmp_bpm);
// 各ステップからanalogread(A4)に流れてくるとこ
uint16_t  tmp_read = map(mozziAnalogRead(SYNC_CONTROL),0, 1023, 0, 6);

aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));

 gain = (int) kEnvelope.next(); //各ステップからtemp_read通って出てきたところでnext

digitalWrite(SYNC_OUT, LOW);

if(kDelay.ready()){
  if(stp_cnt <= 7){
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_1 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_2 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_3 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_4 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_5 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_6 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_7 == HIGH); //トリガー
  }
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
if(kDelay.ready()){
    kEnvelope.start(attack,decay); //エンベロープスタート
    kDelay.start(STEP_8 == HIGH); //トリガー
  }
      break;

    }
        kDelay.start(tmp_bpm);
    stp_cnt++;

  }else{
    stp_cnt = 0;
  }
   }

 

//Serial.print("bpm: ");
//  Serial.println(digitalRead(SYNC_OUT));
}

int updateAudio(){
return (gain*aSin.next())>>8; //ゲインを掛けてビットシフト
}

void loop(){
  audioHook();
}