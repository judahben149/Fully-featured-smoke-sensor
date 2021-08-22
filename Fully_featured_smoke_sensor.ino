#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial mySerial(11, 10); //Rx and Tx respectively

const int buzzer = 12;
int led = 13;
int smokesensor = A0;

char phone[15] = {};

//LCD Config
const int rs = A2, en = A3, d4 = A4, d5 = A5, d6 = 1, d7 = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Keypad config
const byte numRows = 4;         //number of rows on the keypad
const byte numCols = 4;         //number of columns on the keypad

//Keypad map
char keymap[numRows][numCols] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {2, 3, 4, 5}; //Rows 0 to 3 //if you modify your pins you should modify this too
byte colPins[numCols] = {6, 7, 8, 9}; //Columns 0 to 3

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
char keypressed;

int sensorValue;  //variable to store sensor value

//variable declarations
boolean ussdprefixMatch = false;
boolean newMessage = false;
boolean prefixMatch = false;

char str3[10];//maximum size for str3
//char Grsp[200];//size for max

char rechargepin[20] = {};
char phoneno[14] = {};
int pin = 0, j = 0, y = 0, roww = 0;
char pinhere = "";
bool cancelled = false;
char str2[4] = {};
int str2_int;

const byte maxMessage = 200;
char Grsp[maxMessage];//size for max message
char Grsp2[maxMessage];

boolean lil = 0;//trying to use this to check for additional checks before reading from serial after recharging to either show successful or not
boolean ja = 0; //trying to use this one to actually check for the next A press when confirming the voucher pin



void setup()
{
  //Serial.begin(9600);
  pinMode(13, OUTPUT);//set buzzer to output
  buzzercall2();
  lcd.begin(16, 2); //initialize the LCD
  lcd.clear();//clear the lcd screen

  lcd.print("DEVICE ON!");
  delay(3000);
  lcd.clear();

  lcd.print("PLEASE WAIT!");
  delay(3000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("GAS SENSOR");
  lcd.setCursor(5, 1);
  lcd.print("WARMING UP!");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("AUTHENTICATING");
  lcd.setCursor(5, 1);
  lcd.print("GSM MODULE!");
  delay(3000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("READING PHONE NUMBER FROM STORAGE!");
  delay(800);
  for (int positionCounter = 0; positionCounter < 18; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(330);
  }
  delay(400);
  lcd.clear();

  mySerial.begin(9600); //Initialize the software serial activity

  //this loop aims to load the contents of the eeprom addresses into the char array "phoneno"
  for (int xxp = 0, xyp = 10; xxp < 15; xxp++) {
    phoneno[xxp] = EEPROM.read(xyp);
    xyp++;
  }

  checkbalance_startup();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CURRENT BALANCE");
  lcd.setCursor(8, 1);
  lcd.print(" = N");
  lcd.print(str2_int);

  delay(2500);
  lcd.clear();
}


void loop()
{
  keypressed = myKeypad.getKey();
//  if (keypressed == 'D') {
//    buzzercall2();
//    callrecharge();
//  }

  if (keypressed == '*') {
    buzzercall2();
    checkbalance();
  }

  if (keypressed == 'B') {
    buzzercall2();
    changenumber();
  }

  if (keypressed == '#') {
    lcd.clear();
    buzzercall2();
    lcd.setCursor(0, 0);
    lcd.print("Current phone no");
    lcd.setCursor(0, 1);
    lcd.print("+");
    lcd.print(phoneno);
    delay(3000);
    lcd.clear();
  }

  smoke();

  delay(50);
}

void changenumber() {

  int char_no = 0;
  int lcdrow = 0;
  bool changestatus = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Phone no:");

  while (keypressed != 'A') {
    keypressed = myKeypad.getKey();

    if (keypressed != NO_KEY && keypressed >= '0' && keypressed <= '9') {

      lcd.setCursor(lcdrow, 1);
      lcd.print(keypressed);

      phone[char_no] = keypressed;

      buzzercall();
      char_no++;
      lcdrow++;
    }

    if (keypressed == 'C') {
      changestatus = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("OPERATION");
      lcd.setCursor(4, 1);
      lcd.print("TERMINATED!");

      buzzercall2();
      delay(1300);
      lcd.clear();
      break;
    }
  }
  buzzercall();


  if (changestatus == true) {
    int xyp = 10;//xyp is used to set the address of the eeprom to be written to


    for (int xxp = 0; xxp < 14; xxp++) {//xxp is used both for the for loop
      //iteration and for the character in the char array "phone" pointed to
      EEPROM.put(xyp, phone[xxp]);
      xyp++;
    }

    xyp = 10;//xyp which has been declared before is used
    for (int xxp = 0; xxp < 14; xxp++) {
      phoneno[xxp] = EEPROM.read(xyp);
      xyp++;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PHONE NUMBER");
    lcd.setCursor(8, 1);
    lcd.print("CHANGED!");
    delay(1500);
    lcd.clear();
  }
  changestatus = true;
}

void smoke() {

  sensorValue = analogRead(smokesensor);
  lcd.setCursor(0, 0);
  lcd.print("Gas conc:");
  lcd.print(sensorValue);
  lcd.print("ppm");


  if (sensorValue > 60)
  {
    digitalWrite(led, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smoke detected!");
    buzzercall10();
    delay(1300);

    lcd.setCursor(0, 1);
    lcd.print("Conc: ");
    lcd.print(sensorValue);
    lcd.print(" ppm");

    delay(4000);
    lcd.clear();

    send_text();
    digitalWrite(led, LOW);
  }
}

void buzzercall1 () {
  for (int i = 0; i < 1; i++) {
    digitalWrite(buzzer, HIGH);
    delay(70);
    digitalWrite(buzzer, LOW);
    delay(15);
  }
}

void buzzercall() {
  digitalWrite(buzzer, HIGH);
  delay(150);
  digitalWrite(buzzer, LOW);
}

void buzzercall2 () {
  for (int i = 0; i < 2; i++) {
    digitalWrite(buzzer, HIGH);
    delay(70);
    digitalWrite(buzzer, LOW);
    delay(15);
  }
}

void buzzercall3 () {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(70);
    digitalWrite(buzzer, LOW);
    delay(15);
  }
}

void buzzercall10 () {
  for (int i = 0; i < 10; i++) {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(25);
  }
}


void send_text() {
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  mySerial.println("AT+CMGS=\"+" + String(phoneno) + "\""); //change ZZ with country code and xxxxxxxxxxx with phone number to sms
  //   + "+2348106270079\""
  delay(100);
  mySerial.print("WARNING! HIGH SMOKE CONCENTRATION (" + String(sensorValue) + "ppm) DETECTED!!"); //text content
  delay(100);
  mySerial.write(0x1A);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMS sent");
  lcd.setCursor(2, 1);
  lcd.print("successfully!");
  delay(2000);
  lcd.clear();

  checkbalance_startup();
}

void low_sms_text() {
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  mySerial.println("AT+CMGS=\"+" + String(phoneno) + "\""); //change ZZ with country code and xxxxxxxxxxx with phone number to sms

  delay(100);
  mySerial.print("WARNING! LOW AIRTIME (N" + String(str2_int - 4) + ").... RECHARGE AS SOON AS POSSIBLE!"); //text content
  delay(100);
  mySerial.write(0x1A);
}


void checkbalance() {

  //+CUSD: 0, "You are on SmartCONNECT.Main Bal: N30.42;Dial 
  //*123*1# for Bonus Bal.Dial *315#, Call 
  //@ 11k/s; N7 access fee on 1st call of the day applies", 15

  lcd.clear();
  lcd.print("USSD running....");
  mySerial.println("AT+CNMI=1,2,0,0,0");//
  delay(200);
  mySerial.println("AT+CUSD=1,\"*123#\"");
  delay(3500);

  while (mySerial.available() > 0)//
  {
    //readBytes returns number read not zero referenced
    byte numChars = mySerial.readBytes(Grsp, 100);//
    Grsp[numChars] = '\0';//null Terminate
    newMessage = true;
    //Serial.println(Grsp);
  }


  if (strstr(Grsp, "+CUSD") != 0 and newMessage == true)
  {
    //Serial.println("prefixMatch +CUSD");
    prefixMatch = true;
    newMessage = false;
  }

  else //if (newMessage == true)
  {
    // Serial.println("no prefixMatch");
    newMessage = false;
  }

  if (prefixMatch == true)
  {
    prefixMatch = false;

    char* strtokussd;
    char* strtokussd2;


    //strtokussd tokenizes the string with the first colon
    //then strtokussd2 takes in null, i.e it continues with the first string then looks for the next colon
    strtokussd = strtok(Grsp, ":");
    strtokussd2 = strtok(NULL, ":");

    //the same strtokussd2 then finds the first semicolon
    //find semicolon which is where the amount stops
    strtokussd2 = strtok(NULL, ";");
    //strcopy then copies what's between the second colon and the first semi-colon and fills it in str3
    strcpy(str3, strtokussd2);


    lcd.clear();
    lcd.print("Balance=");
    lcd.print(str3);
    delay(4000);
    lcd.clear();
  }
}

void callrecharge() {
  //y is set to ensure that pressing B will not take you back to this point in the program thereby losing data
  //y = 1; //set back to 0 after the recharge is complete
  cancelled = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter voucher: ");
  for (int x = 0; x < sizeof(rechargepin); x++) {
    rechargepin[x] = {};
  }
  recharge();

  if (keypressed == 'A' && strlen(rechargepin) >= 5) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Please wait...");
    buzzercall1();
    delay(500);
    //    lcd.setCursor(0, 1);
    //    lcd.print(rechargepin);
    //delay(3000);
    //keypressed = NO_KEY;
    confirmation();
    if (cancelled == false) {
      ussdrecharge(); //set y back to 0 after the recharge is complete
      y = 0;
      delay(2000);
      lcd.clear();
    }
  }

  else if (keypressed == 'A' && strlen(rechargepin) < 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("False pin input");
    y = 0;
    delay(2500);
    lcd.clear();
  }
}


void recharge() {
  pin = 0;
  roww = 0;
  while (keypressed != 'A') {
    keypressed = myKeypad.getKey();

    if (keypressed != NO_KEY && keypressed >= '0' && keypressed <= '9') {     //If the char typed is numeric and not "nothing"

      lcd.setCursor(roww, 1);                                 //This to write the key pressed on the LCD, whenever a key is pressed it's position is controlled by j
      lcd.print(keypressed);
      rechargepin[pin] = keypressed;
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
      pin++;
      roww++;
    }

    if (keypressed == 'C') {
      y = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Operation");
      lcd.setCursor(6, 1);
      lcd.print("cancelled!");
      buzzercall3();
      delay(1500);
      lcd.clear();
      cancelled = true;
      break;
      //goto cancelhere;
    }
  }
}

void confirmation() {
  if (cancelled == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Correct? Press A");
    lcd.setCursor(0, 1);
    lcd.print(rechargepin);
    //keypressed = NO_KEY;
    //delay(3000);
    while (1) {
      keypressed = myKeypad.getKey();
      if (keypressed == 'A') {
        buzzercall1();
        cancelled = false;
        break;
      }

      else if (keypressed == 'C') {
        cancelled = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Operation");
        lcd.setCursor(6, 1);
        lcd.print("cancelled!");
        buzzercall3();
        lcd.clear();
        break;
      }
    }
  }
}



void ussdrecharge() {
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("USSD code ");
  lcd.setCursor(5, 1);
  lcd.print("running....");


  mySerial.print("AT+CUSD=1,");
  mySerial.print("\"*126*");
  mySerial.print(rechargepin);
  mySerial.print("#\"");
  for (int ww = 0; ww < sizeof(Grsp); ww++) {
    Grsp[ww] = (char)0;
  }

  while (1) {
    keypressed = myKeypad.getKey();
    int xx = 1;

    while (mySerial.available() > 0) {
      byte numChars = mySerial.readBytes(Grsp, maxMessage - 1); //change this Grsp oo so it will be different variable name when combining with ussd test!!
      Grsp[numChars] = '\0';//null Terminate
      //newMessage == true;
      //Serial.println("");
      //Serial.println(Grsp);

      if (strstr(Grsp, "successful") != 0)// && newMessage == true
      {
        //Serial.println("Recharge Successful");
        lcd.clear();
        lcd.print("Recharge Successful");
        //Serial.print("Recharge Successful");

        delay(2000);
        //this for loop clears out the char array Grsp
        for (int ww = 0; ww < sizeof(Grsp); ww++) {
          Grsp[ww] = (char)0;
        }
        y = 0;
        xx = 0;
        break;
      }

      else if (strstr(Grsp, "used") != 0) {
        //Serial.println("Voucher used");
        lcd.clear();
        lcd.print("Voucher used");
        //Serial.print("Voucher used");

        lcd.setCursor(0, 1);
        lcd.print("Try again");
        for (int ww = 0; ww < sizeof(Grsp); ww++) {
          Grsp[ww] = (char)0;
        }
        y = 0;
        xx = 0;
        break;
      }

      else if (strstr(Grsp, "Invalid") != 0) {
        //Serial.println("Invalid pin");
        lcd.clear();
        lcd.print("Invalid voucher");
        //Serial.print("Invalid voucher");

        lcd.setCursor(0, 1);
        lcd.print("Try again");
        for (int ww = 0; ww < sizeof(Grsp); ww++) {
          Grsp[ww] = (char)0;
        }
        y = 0;
        xx = 0;
        break;
      }
    }

    if (xx == 0) {
      break;
    }


    if (keypressed == 'C') {
      y = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Operation");
      lcd.setCursor(6, 1);
      lcd.print("cancelled!");
      buzzercall3();
      break;
    }
  }
  delay(3000);
  lcd.clear();
}

void checkbalance_startup() {
  lcd.clear();
  lcd.print("CHECKING BALANCE");
  mySerial.println("AT+CNMI=1,2,0,0,0");//
  delay(200);
  mySerial.println("AT+CUSD=1,\"*123#\"");
  delay(3500);

  while (mySerial.available() > 0)//
  {
    //readBytes returns number read not zero referenced
    byte numChars = mySerial.readBytes(Grsp2, 100);//
    Grsp2[numChars] = '\0';//null Terminate
    newMessage = true;
    //Serial.println(Grsp);
  }


  if (strstr(Grsp2, "+CUSD") != 0 and newMessage == true)
  {
    //Serial.println("prefixMatch +CUSD");
    prefixMatch = true;
    newMessage = false;
  }

  else if (newMessage == true)
  {
    // Serial.println("no prefixMatch");
    newMessage = false;
  }

  if (prefixMatch == true)
  {
    prefixMatch = false;

    char* strtokussd;
    char* strtokussd2;


    //strtokussd tokenizes the string with the first colon
    //then strtokussd2 takes in null, i.e it continues with the first string then looks for the next colon
    strtokussd = strtok(Grsp2, ":");
    strtokussd2 = strtok(NULL, ":");

    //the same strtokussd2 then finds the first semicolon
    //find semicolon which is where the amount stops
    strtokussd2 = strtok(NULL, ";");
    //strcopy then copies what's between the second colon and the first semi-colon and fills it in str3
    strcpy(str3, strtokussd2);

    for (int x = 0; x < 3; x++) { //" N30.42"
      str2[x] = str3[x + 2];
    }
    str2_int = atoi(str2);

    if (str2_int < 50) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LOW AIRTIME!");
      lcd.setCursor(4, 1);
      lcd.print("RECHARGE!!");

      low_sms_text();
    }
  }
  lcd.clear();
}
