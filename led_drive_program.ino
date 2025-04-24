/*
LED Driver Code

Hardware: 12 daisy chained 8 bit shift registers
Software: Takes in 13 bytes of serial data, outputs byte 1 to LED power control and bytes 2-13 of data to the LEDs. There are 32 LEDs available to call, each LED has R, G, B LEDs for a total of 96 addressable LEDs.

*/

#pragma GCC optimize "O0"

#include <digitalWriteFast.h>

char serial_data = 0; // char for storing read serial data.
uint8_t breathing_effect = 0;
bool breathing_direction = true; // true = up, false = down
byte led_code[13]; // array to hold 1 byte chunks of data * 12 shift registers + 1 byte for power FET control
byte data;

void shiftOuta(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST) {
			digitalWriteFast(dataPin, val & 1);
			val >>= 1;
		} else {	
			digitalWriteFast(dataPin, (val & 0x80) != 0);
			val <<= 1;
		}
    delayMicroseconds(10);
    digitalWriteFast(clockPin, HIGH);
		delayMicroseconds(1);
		digitalWriteFast(clockPin, LOW);
	}
}

void setup() {
  pinMode(5, OUTPUT); //PD5 - register clock pin (latch)
  pinMode(4, OUTPUT); //PD4 - shift register clock pin
  pinMode(3, OUTPUT); //PD3 - clear registers pin (active low)
  pinMode(6, OUTPUT); //PD6 - serial data pin
  pinMode(9, OUTPUT); //PB1 - LED Power, active low (can do PWM for dimming)
  
  digitalWrite(5, LOW);
  
  digitalWrite(3, LOW);
  delay(100);
  digitalWrite(3, HIGH); //clear registers, then allow for data
  
  digitalWrite(5, LOW);
  for(int i = 1; i < 13; i++) {
    shiftOuta(6, 4, LSBFIRST, 0);
  }
  delayMicroseconds(1);
  digitalWrite(5, HIGH);

  Serial.begin(9600);
  Serial.print("LED_DRIVE");
}

void loop() { 
  if(Serial.available() >= 13) {
    Serial.readBytes(led_code, 13);
    digitalWrite(3, LOW);
    delay(1);
    digitalWrite(3, HIGH);
    Serial.println("Data Received"); //debug

    digitalWrite(5, LOW);
    for(int i = 1; i < 13; i++) {
      data = led_code[i];
      shiftOuta(6, 4, LSBFIRST, data);
      Serial.println(data); //debug
    }
    delayMicroseconds(1);
    digitalWrite(5, HIGH);

    if(led_code[0] == 0xFF) {
      while(1) {
        if(Serial.available() > 0) {
          Serial.println("INTERRUPTED"); //debug
          analogWrite(9, 0);
          break;
        }
        analogWrite(9, breathing_effect);
        Serial.println(breathing_effect);

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
      Serial.print("solid brightness:"); //debug
      Serial.println(led_code[0]); //debug
    }
  }
}
