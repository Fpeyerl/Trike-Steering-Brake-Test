int brMoveToLimit(int);

#define EXTEND 1
#define RETRACT -1
#define STOP 0
#define MARGIN 10  //allowable margin of err between actual position and setpoint

//uses DROK dual H-bridge driver board
int brIN3 = 7;        //in3
int brIN4 = 8;        //in4
int brENB = 6;        //enable pin (pwm)
int brSensorPin = A1;  //brake fb
int brSensorVal;        //brake fb value 0 - 1023


int y_JoyPin = A2;      //this is the drive/brake joystick Y axis
int x_JoyPin = A3;
int y_JoyVal, x_JoyVal;
int y_JoyVal_max=0, y_JoyVal_min=1000, y_JoyVal_center=0;
int y_JoyVal_maxA=0, y_JoyVal_minA=1000;


int brMinReading;    //actual maximum fb value 0-1023
int brMaxReading;    //actual minimum fb value 0-1023

int brSpeed=200;
int setpoint=50;
long unsigned prev=0;

void setup() {
  Serial.begin(9600);
  pinMode(brIN3, OUTPUT);        //these two pins control direction of actuator
  pinMode(brIN4, OUTPUT);        //for the brake actuator
  pinMode(brENB, OUTPUT);
  pinMode(brSensorPin, INPUT);  //fb pin from brake actuator
  digitalWrite(brENB, LOW);      //initially turn off brake actuator

Serial.print("Extending Actuator to maximum ... ");
delay(1000);
brMoveToLimit(EXTEND);          //get brake drive actuator extend limit
delay(1000);                    //give it time to get there
brMaxReading=analogRead(brSensorPin);
Serial.print("brMaxReading is "); Serial.println(brMaxReading);
delay(1000);


Serial.print("Retracting Actuator to minimum ... ");
delay(1000);
brMoveToLimit(RETRACT);
delay(1000); 
brMinReading = analogRead(brSensorPin); //get brake drive actuator retract limit
Serial.print("brMinReading is "); Serial.println(brMinReading);
delay(1000);

Serial.println("Extending Actuator to maximum (brake off)");
delay(1000);
brMoveToLimit(EXTEND);          //get brake drive actuator extend limit
delay(1000);                    //give it time to get there

//testing joystick on just the Y (brake axis)
Serial.println();
Serial.println("Lets Test brake joystick");
delay(1000);
Serial.println("Move joystick all the way forward");
delay(1000);
y_JoyVal_max =analogRead(y_JoyPin);   
Serial.print("Joystick max = ");Serial.println(y_JoyVal_max);
Serial.println();
Serial.println("Move joystick all the way back ");
delay(1000);
y_JoyVal_min =analogRead(y_JoyPin);      //joystick position 0-1023 
Serial.print("Joystick min = "); Serial.println(y_JoyVal_min);

y_JoyVal_minA=1023;
y_JoyVal_maxA=0;
Serial.println();
Serial.println("Release joystick (center joystick)");
delay(1000);
uint32_t prev=millis();

  //this while loop tests the stability of the joystick by taking readings while centered
  //since the joystick is does not move the counts should be close to the same from max to min
  while(millis() - prev < 3000) {  //for 3 seconds
   
      y_JoyVal_center =analogRead(y_JoyPin);      //joystick position 0-1023 
      delay(50);
     
      if(y_JoyVal_center < y_JoyVal_minA){      //if joy < min value store new min value
          y_JoyVal_minA = y_JoyVal_center;
      }
      else if(y_JoyVal_center > y_JoyVal_maxA)
          y_JoyVal_maxA = y_JoyVal_center;
  }

  y_JoyVal_center=(y_JoyVal_maxA + y_JoyVal_minA)/2;    //this sets the joystick center position in counts which is later mapped to actuator max counts (brake off)
 
  Serial.println("The Counts below should about the same");
  Serial.print("Joystick min = "); Serial.println(y_JoyVal_minA);
  Serial.print("Joystick max = "); Serial.println(y_JoyVal_maxA);
  Serial.println();
  delay(3000);  //300
 
prev=millis();

}

void loop(){

y_JoyVal =analogRead(y_JoyPin);      //joystick position 0-1023 
setpoint = map(y_JoyVal, y_JoyVal_min, y_JoyVal_center, brMinReading, brMaxReading);
 
  if((millis()- prev) > 2000){
        Serial.print("SP = "); Serial.println(setpoint); 
        brSensorVal=analogRead(brSensorPin);
        Serial.print("BRAKE = "); Serial.println(brSensorVal); 
        Serial.println();
        prev=millis();
  }
 
   
    //digitalWrite(driveEnable, LOW);                                          //we are braking so turn off main drive motor
    brSensorVal=analogRead(brSensorPin);                                        //read brSensor; divide by two to scale sensor fb value to joystick value
    //brPosition=(setpoint-brSensorVal)) ;                                        //speed is calculated based on diff between setpoint and where the act. is

    if( (abs(setpoint - brSensorVal)) < MARGIN ){
        brDriveActuator(0, brSpeed);
    } 
    else if(setpoint < brSensorVal) {
          brDriveActuator(-1, brSpeed);
    }
    else if(setpoint > brSensorVal) {
        brDriveActuator(1, brSpeed);
    }
}
   


void brDriveActuator(int Direction, int Speed){  //this is for the steering acuator only
  switch(Direction){
    case 1:      //extension
     
      digitalWrite(brIN4, 1);
      digitalWrite(brIN3, 0);
      analogWrite(brENB, Speed);
      break;
 
    case 0:      //stopping
      digitalWrite(brIN4, 0);
      digitalWrite(brIN3, 0);
      analogWrite(brENB,  0);
      break;

    case -1:      //retraction
      digitalWrite(brIN4, 0);
      digitalWrite(brIN3, 1);
      analogWrite(brENB, Speed);
      break;
  }
}

/* finds the limit of the actuator feedback signal 
inputs: Direction values: EXTEND -1  RETRACT  1  */
int brMoveToLimit(int Direction){
  
 int prevReading=0;
  int currReading=0;
 
  do{
    prevReading = currReading;
    brDriveActuator(Direction, 200);
    currReading = analogRead(brSensorPin);
 
  }while(prevReading != currReading);
 
  return currReading;
}
