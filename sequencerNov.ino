#include <MozziGuts.h> // Mozziの基本ヘッダファイル
#include <Oscil.h> // オシレータのテンプレート
#include <EventDelay.h>
#include <mozzi_midi.h> // MozziでMIDIを用いるためのヘッダ
#include <tables/sin2048_int8.h> // サイン波のテーブル

// SIN2048_DATA2048のデータをaSinに格納
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);

// トリガー(発音タイミング)のためのヘッダ
#include <EventDelay.h>
EventDelay kTriggerDelay;

int stp_cnt;

// コントロールレートをあらかじめ定義
#define CONTROL_RATE 64

// アナログピンマッピング（合ってるのか？）
#define SYNC_CONTROL         4 // For ribbon controler(SoftPot)
#define FUNC_SELECTOR        0
#define SCALE_SELECTOR       1

// シーケンサーステップ.(Digital)
#define  STEP_1            12
#define  STEP_2            11
#define  STEP_3            10
#define  STEP_4            8 //D9はオーディオ出力に使うから飛ばし
#define  STEP_5            7
#define  STEP_6            6
#define  STEP_7            5
#define  STEP_8            3
// bpm,scale
// #define  BPM_SEL              2
// #define  SCL_SEL              4




int scaleMap[6][7] = {
  {60, 62, 64, 65, 67, 69, 71}, //major
  {60, 62, 63, 65, 67, 69, 70}, //dorian
  {60, 62, 64, 67, 69, 72, 74}, //penta
  {60, 63, 65, 67, 70, 72, 75}, //minor penta 
  {60, 62, 63, 65, 67, 68, 70}, //minor
  {60, 61, 64, 65, 67, 68, 71} //gypsy
};


void setup(){
  startMozzi(CONTROL_RATE);
  kTriggerDelay.start(250); 
}

void updateControl() {
  // pinMode(BPM_SEL,OUTPUT);
  // pinMode(SCL_SEL,OUTPUT);
  //pinMode(PWM_PIN,OUTPUT);
  pinMode(STEP_1,OUTPUT);
  pinMode(STEP_2,OUTPUT);
  pinMode(STEP_3,OUTPUT);
  pinMode(STEP_4,OUTPUT);
  pinMode(STEP_5,OUTPUT);
  pinMode(STEP_6,OUTPUT);
  pinMode(STEP_7,OUTPUT);
  pinMode(STEP_8,OUTPUT);
  //pinMode(LED_PIN,OUTPUT);

    //Read Scale selector
    // digitalWrite(BPM_SEL,LOW);
    // digitalWrite(SCL_SEL, HIGH);
uint8_t  tmp_scale = map(mozziAnalogRead(SCALE_SELECTOR),0, 1023, 0, 5);    
    //Read BMP Data
    // digitalWrite(SCL_SEL,LOW);
    // digitalWrite(BPM_SEL,HIGH);
uint16_t tmp_bpm = map(mozziAnalogRead(FUNC_SELECTOR),0,1023,0,500);
kTriggerDelay.set(tmp_bpm);
// 各ステップからanalogread(A4)に流れてくるとこ
uint16_t  tmp_midi_note;  
uint16_t  tmp_read = map(mozziAnalogRead(SYNC_CONTROL),0, 1023, 0, 6);

   
  aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));



  
if(kTriggerDelay.ready()){
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
        // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
        // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
      // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
      // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
      // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
     // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
     // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
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
    // aSin.setFreq(mtof(scaleMap[tmp_scale][tmp_read]));
      // kTriggerDelay.start(tmp_bpm);
      break;

    }
        kTriggerDelay.start(tmp_bpm);
    stp_cnt++;

  }else{
    stp_cnt = 0;
  }
   }

}

int updateAudio(){
  return aSin.next();
}

void loop(){
  audioHook();
}