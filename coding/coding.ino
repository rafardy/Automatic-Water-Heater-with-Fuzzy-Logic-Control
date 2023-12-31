//Raihan Fadhlirachman Ardyas

#include <DallasTemperature.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <EEPROM.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
char customKey, mulaiKey;
//char stringAngka[2]
int indexKeypad = 0;
const byte ROWS = 4;
const byte COLS = 4;
 
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
 
byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};
 
float x, mf1, mf2, mf3, mf4, r1, r2, r3, r4 , u;
int u1 = 0, u2 = 200, u3 = 500, u4 = 1000;
int nilaiAktual, sisa, nilaisp;
int ssr = 3;
int sp;
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
 
void(*resetFunc)(void) = 0;
 
void hitung(){
//perhitungan membership function
int a = -100, b = -50, c = 0, d = 5;
  mf1 = max(min(min((x-a)/(b-a),(d-x)/(d-c)),1),0);
 
int a1 = 0, b1 = 5, c1 = 10;
  mf2 = max(min((x-a1)/(b1-a1),(c1-x)/(c1-b1)),0);
 
int a2 = 5, b2 = 15, c2 = 29;
  mf3 = max(min((x-a2)/(b2-a2),(c2-x)/(c2-b2)),0);
 
int a3 = 22, b3 = 30, c3 = 50, d3 = 55;
  mf4 = max(min(min((x-a3)/(b3-a3),(d3-x)/(d3-c3)),1),0);
 
}
void fuzzy_rule(){
//menerapkan hasil perhitungan membership function ke aturan
  hitung();
  r1 = mf1*u1; //jika error negatif, maka aktuator off
  r2 = mf2*u2; //jika error positif kecil, maka aktuator on sebentar
  r3 = mf3*u3; //jika error positif sedang, maka aktuator on sedang
  r4 = mf4*u4; //jika error positif besar, maka aktuator on lama
}
void defuzzyfikasi(){
  //defuzzyfikasi
  fuzzy_rule();
  u = (r1+r2+r3+r4)/(mf1+mf2+mf3+mf4);
 
}
 
void setsp(){
  lcd.setCursor(0, 0);
  lcd.print("SET SP  ");
 
  customKey = customKeypad.getKey();
 
  if(customKey >= '0' && customKey <= '9')
  {
    sp = sp*10 + (customKey - '0');
    lcd.setCursor(0, 1);
    lcd.print(sp);
    //lcd.print(customKey);
    //sp = customKey;
  }
 
  if(customKey == '*')
  {
    lcd.clear();
    delay(1000);
    nilaisp = sp;
    return;
  }
 
  setsp();
}
 
void ceksp(){
  lcd.setCursor(0, 0);
  lcd.print("SP = ");
  lcd.print(nilaisp);
 
  customKey = customKeypad.getKey();
 
  if(customKey == '*')
  {
    lcd.clear();
    delay(1000);
    return;
  }
 
  ceksp();
}
 
 
void mulai(){
  if(sp == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Mohon Set SP");
    delay(2000);
    return;
  }
 
  sensor.requestTemperatures();
  nilaiAktual = sensor.getTempCByIndex(0);
  Serial.print("DATA, TIME,");
  Serial.println(nilaiAktual);
  x = sp - nilaiAktual;
  defuzzyfikasi();
  Serial.print("Nilai Aktual : ");  
  Serial.println(nilaiAktual);  
  Serial.print("X : ");
  Serial.println(x);
 
  //print nilai membership function
  Serial.print("Nilai mf1: ");
  Serial.println(mf1);
  Serial.print("Nilai mf2: ");
  Serial.println(mf2);
  Serial.print("Nilai mf3: ");
  Serial.println(mf3);
  Serial.print("Nilai mf4: ");
  Serial.println(mf4);
 
 
  Serial.print("L1");
  Serial.println(nilaiAktual);
  lcd.setCursor(0, 0);
  lcd.print("x = ");
  lcd.print(x);
  lcd.print("    ");
  lcd.setCursor(0, 1);
  lcd.print("Suhu : ");
  lcd.print(nilaiAktual);
  lcd.print("    ");
  delay(200);
 
  //print nilai defuzzyfikasi
  Serial.print("Nilai Deffuzy : ");
  Serial.println(u);
 
  //membuat hasil defuzzyfikasi menjadi pwm untuk mengatur ssr 
  digitalWrite(ssr, HIGH);
  delay(u);
  sisa = 1000 - u;
  digitalWrite(ssr, LOW);
  delay(sisa);
 
  if(u=0){
    sisa = 1000 - u;
    digitalWrite(ssr, LOW);
    delay(sisa);
  }
  customKey = customKeypad.getKey();
  if(customKey == '*'){
    lcd.clear();
    digitalWrite(ssr, 0);
    resetFunc(); //call reset
    delay(800);
    return;
  }
  mulai();  
}
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(ssr, OUTPUT); 
  nilaisp = sp;
  sensor.begin();
  Serial.println("CLEARDATA");
  Serial.println("LABEL, Waktu, DS18B20(C)");
  delay (500);
  lcd.clear();
 
  lcd.setCursor(0, 0);
  lcd.print("HEATER OTOMATIS");
  delay(3000);
  lcd.clear();
}
 
void loop() {
  customKey = customKeypad.getKey();
 
 
  if(x == 0){
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1.SET SP");
  }
 
  if(x == 1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("2.MULAI");
  }
 
  if(x == 2){
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("3.CEK SP");
  }
  switch(customKey)
  {
    case'0'...'9':
      break;
 
    case '#':
      break;
 
    case '*':
      break;
 
    case 'A':
    x++;
      break;
 
    case 'B':
    x--;
      break;
 
    case 'C':
      break;
 
    case 'D':
    if(x == 0){
      lcd.clear();
      //analogWrite(ssr, 0);
      setsp();
    }
    if(x == 1){
      lcd.clear();
      mulai();
    }
    if(x == 2){
      lcd.clear();
      //analogWrite(ssr, 0);
      ceksp();
    }
     break;   
  }
}  
