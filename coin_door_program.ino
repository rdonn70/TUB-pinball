int credit_coin = 1;
int credit_service = 1;
int volume_up = 1;
int volume_down = 1;
int debug_mode = 1;
int tilt_switch = 1;
int credit = 0;
int volume_u = 0;
int volume_d = 0;
int debug = 0;

int last_credit_coin = 1;
int last_credit_service = 1;
int last_volume_up = 1;
int last_volume_down = 1;
int last_debug_mode = 1;

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.print("COIN_DOOR");
}

void loop() {
  credit_coin = digitalRead(5);
  credit_service = digitalRead(7);
  volume_up = digitalRead(2);
  volume_down = digitalRead(6);
  debug_mode = digitalRead(3);
  tilt_switch = digitalRead(4);

  if(credit_service == 0 && last_credit_service == 1) {
    credit = 1;
  } else if(credit_coin == 0 && last_credit_coin == 1 && tilt_switch == 1) {
	  credit = 1;
  } else {
    credit = 0;
  }
  if(volume_up == 0 && last_volume_up == 1) {
	  volume_u = 1;
  } else {
	  volume_u = 0;
  }
  if(volume_down == 0 && last_volume_down == 1) {
	  volume_d = 1;
  } else {
    volume_d = 0;
  }
  if(debug_mode == 0 && last_debug_mode == 1) {
    debug = 1;
  } else {
    debug = 0;
  }
  
  last_credit_coin = credit_coin;
  last_credit_service = credit_service;
  last_volume_up = volume_up;
  last_volume_down = volume_down;
  last_debug_mode = debug_mode;
  
  if(credit == 1 || volume_u == 1 || volume_d == 1 || debug == 1) {
    Serial.print(String(credit) + String(volume_u) + String(volume_d) + String(debug));
    credit = 0;
  }
  
  delay(50); // debounce
}
