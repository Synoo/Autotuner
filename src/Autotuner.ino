// AutoTuner V2.01
// by Synoo a.k.a. Patrick Buijs(18-3-2017)
// Modified by Arie Buijs (11-04-2017) (checked by Patrick Buijs)

#include <Servo.h>             // Servo library for the Servo motor
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LiquidCrystal_I2C libraby for the lcd screen
#include <SWR.h>

Servo servo;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Analog IN
int fwdPin                      = 2;          // Number of thr forward signal Pin from SWR bridge
int refPin                      = 3;          //Number of the reflected signal pin SWR bridge

// Digital IN/OUT
int tunePin                     = 3;          // Number of the button pin for start tuning
int buttonIncrementPin          = 4;          // Number of the increment button pin
int buttonDecrementPin          = 5;          // Number of the decrement button pin
int ServoAnt1Pin                = 6;          // Number of the Ant1 servo pin
int powerRelayPin               = 7;          // Number of the relays stuurspanning Servo
int bandChoicePin               = 8;          // Number of the band choise pin
int twenty_BandPin              = 9;          // Number op relay pin for 20 meter band
int thirty_BandPin              = 10;         // Number op relay pin for 30 meter band
int forty_BandPin               = 11;         // Number op relay pin for 40 meter band
int ServoAnt2Pin                = 12;         // Number of the Ant2 servo pin

int bandChoiceButtonPushCounter = 5;          // counter for the number of button presses

int bestServoPos                = 0;
int totalRefl                   = 0;
int refl                        = 0;
int pos                         = 0;
int bestRefl                    = 1023;
int buttonState                 = 0;
int buttonIncrementState        = 0;
int buttonDecrementState        = 0;

int forward;
int reflected;

int currentBand = 0;


float swr;
float m_SWR;
float test_forward;
float reflection;

void setup() {
  pinMode(ServoAnt1Pin, OUTPUT);
  pinMode(ServoAnt2Pin, OUTPUT);
  pinMode(powerRelayPin, OUTPUT);
  pinMode(twenty_BandPin, OUTPUT);
  pinMode(thirty_BandPin, OUTPUT);
  pinMode(forty_BandPin, OUTPUT);
  pinMode(tunePin, INPUT);
  pinMode(buttonIncrementPin, INPUT);
  pinMode(buttonDecrementPin, INPUT);
  pinMode(bandChoicePin, INPUT);

  servo.attach(ServoAnt1Pin);

  digitalWrite(tunePin, HIGH);
  digitalWrite(buttonIncrementPin, HIGH);
  digitalWrite(buttonDecrementPin, HIGH);
  digitalWrite(twenty_BandPin, HIGH);
  digitalWrite(thirty_BandPin, HIGH);
  digitalWrite(forty_BandPin, HIGH);
  digitalWrite(powerRelayPin, HIGH);
  digitalWrite(bandChoicePin, LOW);

  defaultDisplay();
}

void loop() {
  //Temporary
//  test();

  swrCalc();    // compute SWR

  if(digitalRead(bandChoicePin) == HIGH) {
    delay(500);
    if(bandChoiceButtonPushCounter > 5) {
      bandChoiceButtonPushCounter = 0;
    } else {
      bandChoiceButtonPushCounter++;
    }
  }

  switch(bandChoiceButtonPushCounter) {
    case 0:
      setRelays15Meter(); // 15 meter
      break;
    case 1:
      setRelays17Meter(); // 17 meter
      break;
    case 2:
      setRelays20Meter(); // 20 meter
      break;
    case 3:
      setRelays30Meter(); // 30 meter
      break;
    case 4:
      setRelays40Meter();
      break;
    case 5:
      setRelaysAutomaticMeter();
      break;
  }

  if(m_SWR >= 2) {
    if (digitalRead(tunePin) == LOW || (m_SWR >= 2)) {
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      lcd.clear();
      lcd.setCursor(7, 2);
      lcd.print("TUNING");

      switch(bandChoiceButtonPushCounter) {
        case 0:
          tuning(1040, 1200, 1.3);
          break;
        case 1:
          tuning(1350, 1500, 1.8);
          break;
        case 2:
          tuning(1350, 1500, 1.6);
          break;
        case 3:
          tuning(1410, 1570, 1.6);
          break;
        case 4:
          tuning(1200, 1700, 1.3);
          break;
      }

      displayResults();  //TEMPORARY
      delay(50);
    } else {
      digitalWrite(powerRelayPin, HIGH);
      delay(100);
    }
  }

  if(digitalRead(buttonIncrementPin) == LOW) {
    incrementBestServoPos();
  }

   if(digitalRead(buttonDecrementPin) == LOW) {
    decrementBestServoPos();
   }
}

void tuning(int startPosition, int endPosition, float endSwr){
  for(pos = startPosition; pos <= endPosition; pos +=1){
    servo.writeMicroseconds(pos);
    swrCalc();

    Serial.print(m_SWR);

    if(m_SWR >= 2){
      delay(1);
    } else if(m_SWR < 2){
      delay(200);
    }

    if(m_SWR < endSwr){
      bestServoPos = pos;
      break;
    }
  }
}

// This function is for what to display when the arduino starts
void defaultDisplay(){
  lcd.begin(20,4);
  lcd.backlight();
}

// This fuction is for incrementing the servo pos by 1
void incrementBestServoPos(){
  bestServoPos += 3;
  setServoPosition();
  delay(500);
}

// This fuction is for decrementing the servo pos by 1
void decrementBestServoPos(){
  bestServoPos -= 3;
  setServoPosition();
  delay(500);
}

// This function first activates the relays, then sets the servo position and then turns the relays off.
void setServoPosition(){
  digitalWrite(powerRelayPin, LOW);
  delay(100);
  servo.writeMicroseconds(bestServoPos);
  delay(100);
  digitalWrite(powerRelayPin, HIGH);

  lcd.clear();
  displayResults();
}

void setRelays15Meter(){
  digitalWrite(twenty_BandPin, HIGH);
  digitalWrite(thirty_BandPin, HIGH);
  digitalWrite(forty_BandPin, HIGH);
  lcd.setCursor(5,1);
  lcd.print("         ");
  lcd.setCursor(0,0);
  lcd.print("15 Meter");
}

void setRelays17Meter(){
  digitalWrite(twenty_BandPin, HIGH);
  digitalWrite(thirty_BandPin, HIGH);
  digitalWrite(forty_BandPin, HIGH);
  lcd.setCursor(0,0);
  lcd.print("17 Meter");
}

void setRelays20Meter(){
  digitalWrite(twenty_BandPin, LOW);
  digitalWrite(thirty_BandPin, HIGH);
  digitalWrite(forty_BandPin, HIGH);
  lcd.setCursor(0,0);
  lcd.print("20 Meter");
}

void setRelays30Meter(){
  digitalWrite(twenty_BandPin, HIGH);
  digitalWrite(thirty_BandPin, LOW);
  digitalWrite(forty_BandPin, HIGH);
  lcd.setCursor(0,0);
  lcd.print("30 Meter");
}

void setRelays40Meter(){
  digitalWrite(twenty_BandPin, HIGH);
  digitalWrite(thirty_BandPin, HIGH);
  digitalWrite(forty_BandPin, LOW);
  lcd.setCursor(0,0);
  lcd.print("40 Meter");
}

void setRelaysAutomaticMeter(){
  lcd.setCursor(5,1);
  lcd.print("Automatic");
//  Serial.begin(9600);
//  Serial.print("AUTOMATIC");

  if(currentBand == 0){
    if(m_SWR >= 2){
      setRelays15Meter();
      delay(10);
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      tuning(1040, 1200, 1.3);
      digitalWrite(powerRelayPin, HIGH);

      if(m_SWR >= 2){
        currentBand += 1;
      }
    }
  }
  if(currentBand == 1){
    if(m_SWR >= 2){
      setRelays17Meter();
      delay(10);
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      tuning(1350, 1500, 1.7);
      digitalWrite(powerRelayPin, HIGH);

      if(m_SWR >= 2){
        currentBand += 1;
      }
    }

  }
  if(currentBand == 2){
    if(m_SWR >= 2){
      setRelays20Meter();
      delay(10);
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      tuning(1350, 1500, 1.7);
      digitalWrite(powerRelayPin, HIGH);

      if(m_SWR >= 2){
        currentBand += 1;
      }
    }
  }

  if(currentBand == 3){
    if(m_SWR >= 2){
      setRelays30Meter();
      delay(10);
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      tuning(1410, 1570, 1.6);
      digitalWrite(powerRelayPin, HIGH);

      if(m_SWR >= 2){
        currentBand += 1;
      }
    }
  }

  if(currentBand == 4){
    if(m_SWR >= 2){
      setRelays40Meter();
      delay(10);
      digitalWrite(powerRelayPin, LOW);
      delay(20);
      tuning(1000, 1700, 1.3);
      digitalWrite(powerRelayPin, HIGH);

      if(m_SWR >= 2){
        currentBand += 1;
      }
    }

  }
  if(currentBand == 5){
    currentBand = 0;
  }

}

// MAGIC

void swrCalc() {
  float m_MaxSWR;
  word m_MinPower;
  float m_ScaleFWD, m_ScaleREF;     // scale factors
  float m_AlphaFwd, m_AlphaRef;  // the smoothing constant
  float p;
  float wf;

  m_MaxSWR = 25.0;
  m_MinPower = 0;
  m_ScaleFWD = m_ScaleREF = 1.0;
  m_AlphaFwd = 0.5;
  m_AlphaRef = 0.5;

  forward = (m_AlphaFwd * analogRead(fwdPin));       // read forward voltage
  reflected = (m_AlphaRef * analogRead(refPin));   // read reverse voltage

  if (reflected == 0 || forward < m_MinPower) {
    wf = 1.0;
  } else if (reflected >= forward) {
    wf = m_MaxSWR;
  } else {
    #ifdef USE_VOLTAGE_CALC
    wf = (float)(forward + reflected) / (float)(forward - reflected);
    #else
    wf = (float)forward / (float)reflected;
    wf = sqrt(wf);
    wf = (1.0 + wf) / (1.0 - wf);
    #endif
    wf = abs(wf);
  }
    // clip the SWR at a reasonable value
  if (wf > m_MaxSWR) wf = m_MaxSWR;

  // store the final result
  m_SWR = wf;
  if (m_SWR == 1.00){
    lcd.setCursor(0,3);
    lcd.print(String("RX                "));
  }
  else {
    lcd.setCursor(0,3);
    lcd.print(m_SWR);
    lcd.setCursor(6,3);
    lcd.print(String("            "));
  }
  if (m_SWR > 3.00){
    lcd.setCursor(6,3);
    lcd.print(String("! HIGH SWR !"));
   }
}



// TEMPORARY

//void test(){
//  // displaying forward and reflected signal
//  reflection = analogRead(refPin);
//  test_forward = analogRead(fwdPin);
//    lcd.setCursor(0, 2);
//    lcd.print("F = ");
//    if(test_forward >= 10 && refl < 100){
//    lcd.setCursor(3, 2);
//  } else {
//    lcd.setCursor(4, 2);
//    lcd.print(" ");
//    lcd.setCursor(5, 2);
//  }
//    lcd.print(test_forward);
//
//    lcd.setCursor(10, 2);
//    lcd.print("R =   ");
//    if(reflection >= 10 && refl < 1000){
//    lcd.setCursor(14, 2);
//  } else {
//    lcd.setCursor(14, 2);
//    lcd.print(" ");
//    lcd.setCursor(16, 2);
//  }
//  lcd.print(reflection);
//}

// This function is for displaying the results of the calculation.
void displayResults(){
  lcd.clear();
//  lcd.setCursor(0, 1);
//  lcd.print("Position:");
  lcd.setCursor(16, 0);
  lcd.print(String(bestServoPos));
}
