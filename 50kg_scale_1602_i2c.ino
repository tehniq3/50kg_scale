/* weighing for 50kg max
 * program wrote Nicu FLORICA (niq_ro)
 * https://nicuflorica.blogspot.com/search?q=cantar
 * display with MAX7219 and 8 number 7-segment led display
 * tensiometric sensor for 50kg with 3 wire
 * v.1.0 - 30.08.2020, Craiova - Romania
 * v.1,0.a - eliminate Serial monmitoring
 */

#include <HX711_ADC.h> // Arduino library by Olav Kallhovd sept2017 - https://github.com/olkal/HX711_ADC
#include <EEPROM.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define HX711_dout 14 //mcu > HX711 dout pin
#define HX711_sck 15  //mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck); //HX711 constructor:

#define buton1 2
#define buton2 3

const int calVal_eepromAdress = 0;

unsigned long timp;  // measurement time
unsigned long pauza = 1000; // pause between measurements 
float calibrationValue; // calibration value
long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
float newCalibrationValue;

float greutate;  // weight value
float greutate1;  // weight value (absolute value, positive)
byte mod = 0;  
    // 0 - measurement
    // 1 - tare
    // 2 - calibrate

void setup() {
  delay(50);
  lcd.begin();  // initialize the LCD
  //lcd.init();   // initialize others LCD
  lcd.backlight(); // Turn on the blacklight and print a message
  lcd.clear();  // clear the screen

pinMode(buton1, INPUT);
pinMode(buton2, INPUT);
digitalWrite(buton1, HIGH);
digitalWrite(buton2, HIGH);
  
  lcd.setCursor(0, 0);
  lcd.print("Cantar max. 50kg");
  lcd.setCursor(0, 1);
  lcd.print("SW. ver.1.0/2020");
  delay(1500);
  lcd.setCursor(0, 1);
  lcd.print("  Nicu FLORICA  ");
  delay(1500);
  lcd.setCursor(0, 1);
  lcd.print("    (niq_ro)    ");
  delay(1500);
  lcd.clear();  // clear the screen

  LoadCell.begin();
  calibrationValue = -1; // uncomment this if you want to set the calibration value in the sketch
/*
#if defined(ESP8266)|| defined(ESP32)
  EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
*/
  EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
   // Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
   // Serial.println("Startup is complete");
  }

} // end setup



void loop() { 
static boolean newDataReady = 0;
//if (LoadCell.update()) newDataReady = true;

  if (mod == 0)
  {
    LoadCell.update();  // // check for new data/start next conversion
    if (millis() > timp + pauza) {
      greutate = float(LoadCell.getData()/1000.);
   //   Serial.print("Load_cell output val: ");
   //   Serial.println(greutate);
  //    newDataReady = 0;
      timp = millis();
    }
  lcd.setCursor(0, 0);
  lcd.print("Greutate:");
  lcd.setCursor(7, 1);
  if (greutate < 10.) lcd.print(" ");
  if (greutate < 0.)
  {
    lcd.print("-");
    greutate1 = - greutate;
  }
  if (greutate > 0.)
  {
    lcd.print(" ");
    greutate1 = greutate;
  }  
  lcd.print(greutate1);
  lcd.print("kg");

if (digitalRead(buton2) == LOW)
{
  lcd.clear();
 // Serial.println("Tare beginning...");
  lcd.setCursor(0, 0);
  lcd.print("Aducere la zero!");
  LoadCell.tareNoDelay();
 // LoadCell.update();
  delay(1000);
 // lcd.clear();
  mod = 1;
}

if (digitalRead(buton1) == LOW)
{
  lcd.clear();
  //Serial.println("Calibrate beginning...");
  lcd.setCursor(0, 0);
  lcd.print("Calibrare!");
  delay(1000);
 // lcd.clear();
  mod = 3;
}
  } // end mod = 0

if (mod == 1)
{
  if (LoadCell.getTareStatus() == false)  
  {
   // Serial.println("Tare incomplete");
    lcd.setCursor(0, 1);
    lcd.print("   in lucru !   ");
    delay(2000);
    mod = 0;
    lcd.clear();
  }
  if (LoadCell.getTareStatus() == true) 
  {
   // Serial.println("Tare complete");
    lcd.setCursor(0, 1);
    lcd.print("   rezolvat !   ");
    delay(2000);
    mod = 0;
    lcd.clear();
  }
  if (LoadCell.update()) newDataReady = true;
  if (LoadCell.getTareStatus() == true) 
  {
   // Serial.println("Tare complete");
    lcd.setCursor(0, 1);
    lcd.print("   rezolvat !   ");
    delay(2000);
    mod = 0;
    lcd.clear();
  }
} // mod = 2

// check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
   // Serial.println("Tare complete");
    lcd.setCursor(0, 1);
    lcd.print("V");
    delay(100);
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }

if (mod == 3)
{
  lcd.clear();
  calibrare();
  mod = 0;
}
  
  } // end main loop

void calibrare() {
 // Serial.println("***");
 // Serial.println("Start calibration:");
 // Serial.println("Place the load cell an a level stable surface.");
 // Serial.println("Remove any load applied to the load cell.");
  lcd.setCursor(0, 0);
  lcd.print(" Eliminati orice ");
  lcd.setCursor(0, 1);
  lcd.print("  greutate !!!  ");
  delay(5000);
  lcd.clear();
  
  boolean _resume = false;

 // Serial.println("Now, place 1kg on the loadcell.");
//  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");
  lcd.setCursor(0, 0);
  lcd.print("    Puneti o    ");
  lcd.setCursor(0, 1);
  lcd.print("sarcina de 1kg !");
  delay(5000);
//  lcd.clear();
  float known_mass = 1000.;
    LoadCell.update();

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

 // Serial.print("New calibration value has been set to: ");
 // Serial.print(newCalibrationValue);
//  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
/*
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
*/
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);

/*
 #if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
*/
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
     //   Serial.print("Value ");
     //   Serial.print(newCalibrationValue);
    //    Serial.print(" saved to EEPROM address: ");
    //    Serial.println(calVal_eepromAdress);
        _resume = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Factor calibrare");
  lcd.setCursor(0, 1);
  lcd.print(newCalibrationValue);
  delay(5000);
  lcd.clear();

//  Serial.println("End calibration");
//  Serial.println("***");
}
