//PD2: Bumper 1 Solenoid
//PD3: Bumper 2 Solenoid
//PD4: Bumper 3 Solenoid
//PD5: Bumper 1 Switch
//PD6: Bumper 2 Switch
//PD7: Bumper 3 Switch
//PB0: Bumper 1 LED Activate
//PB1: Bumper 2 LED Activate
//PB2: Bumper 3 LED Activate

#include <Wire.h>

int score_bumper1 = 0;
int score_bumper2 = 0;
int score_bumper3 = 0;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, OUTPUT); //PD3
  pinMode(4, OUTPUT); //PD4
  pinMode(5, INPUT); //PD5
  pinMode(6, INPUT); //PD6
  pinMode(7, INPUT); //PD7
  pinMode(8, OUTPUT); //PB0
  pinMode(9, OUTPUT); //PB1
  pinMode(10, OUTPUT); //PB2

  Wire.begin(0x03);
  Wire.onRequest(send);
  Wire.onReceive(receive);
}

void loop() {
  bool bumper1_switch = digitalRead(5);
  bool bumper2_switch = digitalRead(6);
  bool bumper3_switch = digitalRead(7);

  if(bumper1_switch == 1) {
    digitalWrite(2, 1);
    digitalWrite(8, 1);
    delay(1);
    digitalWrite(2, 0);
    delay(20);
    digitalWrite(8, 0);
    score_bumper1 = 1;
  }
  if(bumper2_switch == 1) {
    digitalWrite(3, 1);
    digitalWrite(9, 1);
    delay(1);
    digitalWrite(3, 0);
    delay(20);
    digitalWrite(9, 0);
    score_bumper2 = 1;
  }
  if(bumper3_switch == 1) {
    digitalWrite(4, 1);
    digitalWrite(10, 1);
    delay(1);
    digitalWrite(4, 0);
    delay(20);
    digitalWrite(10, 0);
    score_bumper3 = 1;
  }
  delay(100);
}

void send() {
  char score = (score_bumper1 << 7) | (score_bumper2 << 6) | (score_bumper3 << 5) | (0 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0);
  score_bumper1 = 0;
  score_bumper2 = 0;
  score_bumper3 = 0;
  Wire.write(score);
}

void receive() {

}