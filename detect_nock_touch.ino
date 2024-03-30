#include <CapacitiveSensor.h>

CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2);  // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
int touchPin = 13;  //タッチ検出確認用
int nockPin = 7;  //ノック出力
int checkPin = 5; //ノック検出用
int nockVec[] = {0, 0, 0, 0, 0}; //過去のノックを格納 0 or 1
int touchVec[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //過去のタッチを格納 0 ~ 最大値不明
int vecSize = sizeof(nockVec) / 2; //nockVecの配列の長さ

void setup() {
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);  // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(9600);
  pinMode(nockPin, OUTPUT);
  pinMode(touchPin, OUTPUT);
}

void loop() {
  long valueTouch = cs_4_2.capacitiveSensor(30);
  int pinRead = digitalRead(checkPin);
  digitalWrite(nockPin, HIGH);

  //ノック用配列の更新 0 or 1が格納される
  for (int i = 0; i < vecSize; i++) {
    if (i < vecSize-1) {
      nockVec[i] = nockVec[i+1];
    } else {
      nockVec[i] = (int)pinRead;
    }
  }

  //タッチ用配列の更新 0以上の整数が格納される
  for (int i = 0; i < vecSize*2; i++) {
    if (i < vecSize*2-1) {
      touchVec[i] = touchVec[i+1];
    } else {
      touchVec[i] = valueTouch;
    }
  }

  //ノック用の簡易ノイズ除去
  //1 0 1 1 1 or 1 0 0 1 1 or 1 0 0 0 1のとき、0 -> 1 に変換
  if (nockVec[0] && !nockVec[1]) {
    for (int i = 2; i < vecSize; i++) {
      if (nockVec[i]) {
        for (int j = 1; j < i; j++) {
          nockVec[j] = int(HIGH);
        }
        break; 
      }
    }
  }

  //タッチ用の簡易ノイズ除去
  if ((touchVec[0] > 400) && (touchVec[1] <= 400)) {
    for (int i = 2; i < vecSize*2; i++) {
      if (touchVec[i] > 400) {
        for (int j = 1; j < i; j++) {
          touchVec[j] = 1000;
        }
        break; 
      }
    }
  }
  
  // ノック検出　タッチ検出　->　Unityへのシリアル通信用, 検知したら1を渡し、それ以外は0を渡す
  if (nockVec[0] && touchVec[0] > 400) {
    Serial.println("V:1 T:1");
  } else if (nockVec[0] && touchVec[0] <= 400) {
    Serial.println("V:1 T:0");
  } else if (!nockVec[0] && touchVec[0] > 400) {
    Serial.println("V:0 T:1");
  } else {
    Serial.println("V:0 T:0");
  }

  //タッチ確認用
  if (valueTouch >= 1000) {
    digitalWrite(touchPin, HIGH);
  } else {
    digitalWrite(touchPin, LOW);
  }

  //Unity側とのfpsを合わせるため25にしている。これより高速だとキャッシュがたまり、低速のおそれあり
  delay(25);
}