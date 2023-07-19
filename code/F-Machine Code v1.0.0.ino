#define   SpeedPot        A0
#define   StrokeLengthPot A1
#define   Intensity       A2
#define   MotorDirection  2                  //HIGH is out, LOW is in.
#define   MotorPulse       3
#define   MotorEnable     4                  //Inverted logic, LOW is enabled, HIGH, is disabled.
#define   OutLimitSwitch  6
#define   InLimitSwitch   7
#define   Stop            12
#define   Buzzer          13
#define   Forward         LOW
#define   Backward        HIGH

bool  Direction;          //Used as a direction switch between forward and backward.
int   MinSpeed;           //Minimum delay between pulses (min speed).
int   MaxSpeed;           //Maximum delay between pulses (max speed).
int   StrokeLength;       //Total StrokeLength distance.
int   MaxStrokePos;       //Used in selftest to determine the max stroke length in pulses.
float StrokePos;          //Used in the for loops to keep track of stroke position.
float HoldPeriod;         //holding period (total - rampup - rampdown).
float RampPeriod;         //total StrokeLength that is ramping up.
float RampRate;           //The rate to increment or decrement.
float Intensity;          //% set by pot for holding period.
float TravelSpeed;        //Variable used to decrement and increment the delay between pulses of the motor.
float OutSpeed;           //Speed of the stroke moving out.
float InSpeed;            //Speed of the stroke retraction.
float Speed;
float SpeedSetting;



void setup() {
  Serial.begin(9600);

  pinMode(MotorPulse,      OUTPUT);
  pinMode(MotorDirection,  OUTPUT);
  pinMode(MotorEnable,     OUTPUT);
  pinMode(Buzzer,          OUTPUT);
  pinMode(StrokeLengthPot, INPUT);
  pinMode(RampSpeedPot,    INPUT);
  pinMode(SpeedPot,        INPUT);
  pinMode(Stop,            INPUT_PULLUP);
  pinMode(InLimitSwitch,   INPUT_PULLUP);
  pinMode(OutLimitSwitch,  INPUT_PULLUP);

  digitalWrite(MotorPulse,      LOW);
  digitalWrite(MotorDirection,  Forward);


  Direction = 1;         //Set initial direction to forwards.

  SelfTest();            //Run self-test to determine max stroke length.
}


void loop() {
  GetReadings();

  digitalWrite(MotorDirection, Forward);
  Move();

  digitalWrite(MotorDirection, Backward);
  Move();
}

void GetReadings() {

  SpeedSetting = analogRead(SpeedPot);      //A0
  Intensity    = float(map(analogRead(IntensityPot), 0, 1023, 5, 95)); //A2 % of cycle is spent holding at full speed between ramp up and ramp down. Example = 50% "Smoothness" between 10-90
  StrokeLength = map(analogRead(StrokeLengthPot), 0, 1023, 500, MaxStrokePos); //A1   Total StrokeLength in pulses taken from self test, full length is around 20,000
  MinSpeed     = map(SpeedSetting, 0, 1023, 1500, 100);  //Minimum delay between pulses (min speed). MINIMUM = 200
  MaxSpeed     = map(SpeedSetting, 0, 1023, 200, 35);   //Maximum delay between pulses (max speed). MINIMUM = 25
  HoldPeriod   = StrokeLength * (Intensity / 100); //Example: 20,000 * (50/100) = 20,000 * 0.5 = 10,000.
  RampPeriod   = (StrokeLength - HoldPeriod) / 2; //Example: 20,000 * (50/200) = 20,000 * 0.25 = 5,000.
  RampRate     = (MinSpeed - MaxSpeed) / RampPeriod;  //Example: (2,000 - 100) / 5,000 = 1900 / 5,000 = 0.38
  Speed = MinSpeed;

  Serial.print("Intensity: "); Serial.println(Intensity);
  Serial.print("StrokeLength: "); Serial.println(StrokeLength);
  Serial.print("HoldPeriod: "); Serial.println(HoldPeriod);
  Serial.print("RampPeriod: "); Serial.println(RampPeriod);
  Serial.print("RampRate: "); Serial.println(RampRate);
  Serial.print("MinSpeed: "); Serial.println(MinSpeed);
  Serial.print("MaxSpeed: "); Serial.println(MaxSpeed);


}
void Move() {
  StrokePos = 0;
  Serial.println("Ramp Up");
  while (StrokePos <= RampPeriod && digitalRead(InLimitSwitch) == HIGH && digitalRead(OutLimitSwitch) == HIGH) {
    digitalWrite(MotorPulse, HIGH);
    digitalWrite(MotorPulse, LOW);
    delayMicroseconds(Speed);
    Speed -= (RampRate);                    //Decrease "Speed" delay by "RampRate" until it gets to Max
    StrokePos ++;
  }

  StrokePos = 0;
  Serial.println("Hold");
  while (StrokePos <= HoldPeriod && digitalRead(InLimitSwitch) == HIGH && digitalRead(OutLimitSwitch) == HIGH) {         //StrokeLength the hold period distance at the set speed.
    digitalWrite(MotorPulse, HIGH);
    digitalWrite(MotorPulse, LOW);
    delayMicroseconds(Speed);
    StrokePos++;
  }

  StrokePos = 0;
  Serial.println("Ramp Down");
  while (StrokePos <= RampPeriod && digitalRead(InLimitSwitch) == HIGH && digitalRead(OutLimitSwitch) == HIGH) {         //StrokeLength "RampPeriod" decreasing "Speed" until StrokePos equals RampPeriod.
    digitalWrite(MotorPulse, HIGH);
    delayMicroseconds(Speed);
    digitalWrite(MotorPulse, LOW);
    Speed += RampRate;                    //Inccrease "Speed" delay by "RampRate" until it gets to Max
    StrokePos++;
  }

}


void SelfTest() {                                   //Move all the way forward (out), then all the way back to 0 (home), count the pulses in between to determine max stroke.
  Serial.println("Self test in progress...");
  digitalWrite(MotorDirection, Forward);            //Set motor to forward and move until limit switch goes high.
  while (digitalRead(OutLimitSwitch) == HIGH) {     //While the limit switch has not been triggered,
    digitalWrite(MotorPulse, HIGH);                 //pulse the motor slowly until the limit switch goes high.
    digitalWrite(MotorPulse, LOW);
    delayMicroseconds(200);
  }

  Serial.println("Out limit switch reached");

  digitalWrite(MotorDirection, Backward);            //Set motor to reverse and move until limit switch goes high.

  MaxStrokePos = 0;                                 //Reset stroke counter to 0.
  while (digitalRead(InLimitSwitch) == HIGH) {
    digitalWrite(MotorPulse, HIGH);
    digitalWrite(MotorPulse, LOW);
    delayMicroseconds(200);
    MaxStrokePos ++;                                 //Increase the max stroke pulse counter by 1.
  }
  MaxStrokePos -= 400;

  digitalWrite(MotorDirection, Forward);
  for (int i = 0; i <= 400; i ++) {
    digitalWrite(MotorPulse, HIGH);
    digitalWrite(MotorPulse, LOW);
    delayMicroseconds(400);
  }
  Serial.println("In limit switch reached");
  Serial.print("Max Stroke Position: ");
  Serial.println(MaxStrokePos);
  StrokePos = 0;                                     //Save current stroke position.
  Serial.print("Current stroke position: ");
  Serial.println(StrokePos);
  Serial.println("Self test complete.");
  digitalWrite(Buzzer, HIGH);            //Sound buzzer for 0.5s to let user know self test is complete.
  delay(300);
  digitalWrite(Buzzer, LOW);
  delay(300);
  digitalWrite(Buzzer, HIGH);            //Sound buzzer for 0.5s to let user know self test is complete.
  delay(300);
  digitalWrite(Buzzer, LOW);
  delay(2000);

}
