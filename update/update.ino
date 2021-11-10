#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
StaticJsonDocument<1024> doc;

const char* ssid = "ASIA BROADBAND";
const char* password = "taniM@002";

String serverName = "http://tanimsk.pythonanywhere.com/update";

unsigned long lastTime = 0;
unsigned long clock_lastTime = 0;
unsigned long timerDelay = 60000;
bool toggled_screen = false;

//clock
int hour = 0;
int mins = 0;
int sec = 0;

//toggle_light
bool state1 = HIGH;
bool reading1;
bool previous1 = LOW;
unsigned long now_time1 = 0;

//toggle_screen
bool state2 = HIGH;
bool reading2;
bool previous2 = LOW;
unsigned long now_time2 = 0;

unsigned long debounce = 200;


//alarm
String task = "";
unsigned long alarm_time = 0;
bool alarm_is_on = false;
const int alarm_interval = 400;


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);

  lcd.print("Connecting");

  pinMode(D0, INPUT);
  pinMode(D6, INPUT);
  pinMode(D5, OUTPUT); // BUZZER

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (count < 5) {
      lcd.print(".");
      count++;
    }
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print("Connected!");
  digitalWrite(D5, HIGH);
  delay(200);
  digitalWrite(D5, LOW);
  delay(800);

  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(4, 1);
  lcd.print("Tanim Sk");
  delay(4000);
}

bool toggle_btn1 = false;
bool f_time = true;

void loop() {
  toggle_light();
  toggle_screen();

  if ((millis() - lastTime) > timerDelay || f_time == true) {

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      String serverPath = serverName;

      http.begin(client, serverPath.c_str());

      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        deserializeJson(doc, payload);

        hour = doc["time"]["H"];
        mins = doc["time"]["M"];
        sec = doc["time"]["S"];

      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
    f_time = false;
  }

  //clock
  if ( (millis() - clock_lastTime) >= 1000) {
    print_time_date();
    clock_lastTime = millis();
  }
  
  alarm();
}


void toggle_light(){
  reading1 = digitalRead(D0);

  if (reading1 == HIGH && previous1 == LOW && millis() - now_time1 > debounce){
    
    if (state1 == HIGH)
      state1 = LOW;
    else
      state1 = HIGH;

    now_time1 = millis();
    
    lcd.setBacklight(state1);
  }
  previous1 = reading1;
}

void toggle_screen(){
  reading2 = digitalRead(D6);

  if (reading2 == HIGH && previous2 == LOW && millis() - now_time2 > debounce){
    
    alarm_is_on = false;
    digitalWrite(D5, LOW);

    if (state2 == HIGH){
      state2 = LOW;
      toggled_screen = false;
      print_time_date();
    }
      
    else{
      state2 = HIGH;
      toggled_screen = true;
      print_weather();
    }
    
    now_time2 = millis();
  }
  previous2 = reading2;
}


void print_time_date() {
  sec++;
  if (sec >= 60) {
    sec = 0;
    mins++;
    if (mins >= 60) {
      mins = 0;
      hour++;
      if (hour == 0) {
        hour = 12;
      }
      else if (hour > 12) {
        hour - 12;
      }
    }
  }

  if(!toggled_screen){
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print(zfill(hour) + ":" + zfill(mins) + ":" + zfill(sec));
  
    lcd.setCursor(1, 1);
    lcd.print("date: ");
    lcd.print(String(doc["date"]));
  }
}


void print_weather(){
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("temp: " + String(doc["temp"]));
  
  lcd.setCursor(2, 1);
  lcd.print(String(doc["humidity"]) + ", " + String(doc["weather_type"]));
}


bool alarm_stat = false;
void alarm(){
  if (task != String(doc["task"]) && String(doc["task"]) != ""){
    task = String(doc["task"]);
    alarm_is_on = true;
    toggled_screen = true;
    Serial.println(task);

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(task);
  }

  if(alarm_is_on && (millis() - alarm_time) > alarm_interval){
    lcd.setBacklight(HIGH);
    alarm_stat ? digitalWrite(D5, HIGH):digitalWrite(D5, LOW);
    alarm_stat = !alarm_stat;
    alarm_time = millis();
  }
}


String zfill(int t){
  if(t < 10){
    return "0"+String(t);
  }
  else{
    return String(t);
  }
}
