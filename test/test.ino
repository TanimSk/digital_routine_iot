int old_time;
int current_time;

void setup(){
  Serial.begin(115200);
  pinMode(D1, INPUT);
}

bool state;
bool old_state = false;
int threshold = 300;

void loop(){  
  state = digitalRead(D1);
  
  if (state != old_state){
    old_state = state;
    if(state){
      Serial.println("pressed");
      delay(500);
    }
  }    
}
