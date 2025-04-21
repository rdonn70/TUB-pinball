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
long score = 0;

bool bumper1_last_state = 0;
bool bumper2_last_state = 0;
bool bumper3_last_state = 0;

void setup() {
  pinMode(2, OUTPUT); //PD2
  pinMode(3, OUTPUT); //PD3
  pinMode(4, OUTPUT); //PD4
  pinMode(5, INPUT_PULLUP); //PD5
  pinMode(6, INPUT_PULLUP); //PD6
  pinMode(7, INPUT_PULLUP); //PD7
  pinMode(8, OUTPUT); //PB0
  pinMode(9, OUTPUT); //PB1
  pinMode(10, OUTPUT); //PB2

  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

  Wire.begin(0x03);
  Wire.onRequest(send);
  Wire.onReceive(receive);
}

void loop() {
  bool bumper1_switch = digitalRead(5);
  bool bumper2_switch = digitalRead(6);
  bool bumper3_switch = digitalRead(7);

  if(bumper1_switch == 0) {
    digitalWrite(8, HIGH);
	  digitalWrite(2, HIGH);
    delay(10);
    digitalWrite(2, LOW);
    delay(50);
    digitalWrite(8, LOW);
  }
  if(bumper2_switch == 0) {
    digitalWrite(9, HIGH);
	  digitalWrite(3, HIGH);
    delay(10);
    digitalWrite(3, LOW);
    delay(50);
    digitalWrite(9, LOW);
  }
  if(bumper3_switch == 0) {
    digitalWrite(10, HIGH);
	  digitalWrite(4, HIGH);
    delay(10);
    digitalWrite(4, LOW);
    delay(50);
    digitalWrite(10, LOW);
  }
  delay(25);

  if(bumper1_switch != bumper1_last_state) {  
    bumper1_last_state = bumper1_switch;  
    if(bumper1_switch == HIGH) {
		  score += 100;
	  }
  }
  if(bumper2_switch != bumper2_last_state) {  
    bumper2_last_state = bumper2_switch;  
    if(bumper2_switch == HIGH) {
		  score += 100;
	  }
  }
  if(bumper3_switch != bumper3_last_state) {  
    bumper3_last_state = bumper3_switch;  
    if(bumper3_switch == HIGH) {
		  score += 100;
	  }
  }
}

void send() {
  char score = (score_bumper1 << 7) | (score_bumper2 << 6) | (score_bumper3 << 5) | (0 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0);
  score_bumper1 = 0;
  score_bumper2 = 0;
  score_bumper3 = 0;
  Wire.write(score);
  score = 0;
}

void receive() {

}
