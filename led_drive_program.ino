/*
LED Driver Code

Hardware: 12 daisy chained 8 bit shift registers
Software: Takes in 13 bytes of serial data, outputs byte 1 to LED power control and bytes 2-13 of data to the LEDs. There are 32 LEDs available to call, each LED has R, G, B LEDs for a total of 96 addressable LEDs.

*/

char serial_data = 0; // char for storing read serial data.
uint8_t breathing_effect = 0;
bool breathing_direction = true; // true = up, false = down
byte led_code[13]; // array to hold 1 byte chunks of data * 12 shift registers + 1 byte for power FET control

void setup() {
  pinMode(5, OUTPUT); //PD5 - register clock pin (latch)
  pinMode(4, OUTPUT); //PD4 - shift register clock pin
  pinMode(3, OUTPUT); //PD3 - clear registers pin (active low)
  pinMode(6, OUTPUT); //PD6 - serial data pin
  pinMode(9, OUTPUT); //PB1 - LED Power, Active High (can do PWM for dimming)
  
  digitalWrite(3, LOW);
  delay(100);
  digitalWrite(3, HIGH); //clear registers, then allow for data
  
  Serial.begin(9600);
  Serial.print("LED_DRIVE");
}

void loop() { 
  if(Serial.available() >= 13) {
    Serial.readBytes(led_code, 13);

    digitalWrite(5, LOW);
    for(int i = 1; i < 13; i++) {
      shiftOut(6, 4, LSBFIRST, led_code[i]);
    }
    digitalWrite(5, HIGH);
    
    if(led_code[0] == 0b11111111) {
      while(1) {
        if(Serial.available() > 0) {
          analogWrite(9, 0);
          break;
        }

        analogWrite(9, breathing_effect);

        if(breathing_effect > 250) {
          breathing_direction = false; 
        } else if(breathing_effect <= 0) {
          breathing_direction = true;
        }
        if(breathing_direction == true) {
          breathing_effect += 1;
          delay(25);
        } else {
          breathing_effect -= 1;
          delay(25);
        }
      }
    } else {
      analogWrite(9, led_code[0]);
    }
  }
}