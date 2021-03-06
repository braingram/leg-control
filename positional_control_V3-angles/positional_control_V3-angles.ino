


// Desired (hip, thigh, knee) angles
float commandAngle[] = {0.00, 83.00, 20.00};

// Block for reading sensors and setting the PWM to drive the solenoids
float bit_resolution = pow(2,10)-1;

float PWM_percent[] = {45, 50, 50}; //PWM percentage to drive the solenoids at - this will translate into speed
float PWM_value[] = {0, 0, 0};

int hipGoal;
int thighGoal;
int kneeGoal;
int sensorGoal[] = {hipGoal, thighGoal, kneeGoal};
int goingHot[] = {0,0,0};
int kneeSensorReading;
int thighSensorReading;
int hipSensorReading;
int sensorReading[] = {hipSensorReading, thighSensorReading, kneeSensorReading};
int kneeSensorPin = 17;
int thighSensorPin = 18;
int hipSensorPin = 20;
int sensorPin[] = {hipSensorPin, thighSensorPin, kneeSensorPin};
// this is the distance in sensor reading that is close enough for directed movement
// I am putting this here so we can avoid chasing our tails early in positional control
float closeEnough = 5; 


//These are mapped to the right front leg
int kneePWM2_extend = 6;
int kneePWM1_retract = 5;
int thighPWM2_down = 3;
int thighPWM1_up = 4;
int hipPWM1_forward = 9;
int hipPWM2_reverse = 10;

const int pwm_pins[] = {kneePWM1_retract, kneePWM2_extend, thighPWM1_up, thighPWM2_down, hipPWM1_forward, hipPWM2_reverse};

//Deadman button -- with the joystick I'll be using a momentary switch on the panel.  It will need to be held down or jumpered 
//to set the joystick as "hot"
int deadMan_pin = 0;
int deadMan = 0; //JOYSTICK is OFF if pin is low (pull high to enable joystick).

//enable pins for motor divers
int enable_pin = 1;
int enable_pin_hip = 2;

int M1FB_pin = 21;
int M2FB_pin = 22;
int M1FB_hip_pin = 23;
int current_reading_pin[] = {M2FB_pin, M1FB_pin, M1FB_hip_pin};

//values for home position of leg
//x home is knee fully retracted (cylindar fully extended) i.e. large pot value 
//y home is thigh fully up (cylindar fully retracted) i.e. small pot value
//z home is hip in the ~middle i.e. mid pot value
int homePosition[] = {900, 200, 350}; 

//Sensor reading to angle of joint block
//These sensor values are for the right front leg

int hipPotMax = 722;
int hipPotMin = 93;
//float hipAngleMin = -40.46;
//float hipAngleMax = 40.46;
//float hipSensorUnitsPerDeg;

int thighPotMax = 917;
int thighPotMin = 34;
//float thighAngleMin = -6;
//float thighAngleMax = 84;
//float thighSensorUnitsPerDeg;

int kneePotMax = 934;
int kneePotMin = 148;
//float kneeAngleMin = 13;
//float kneeAngleMax = 123;
//float kneeSensorUnitsPerDeg;

float currentAngles[] = {0.00, 0.00, 0.00};
float currentAnglesR[] = {0.00, 0.00, 0.00};

const float pi = 3.141593;

//leg link lengths hip, thigh, and knee
const int L1 = 11;
const int L2 = 54;
const int L3 = 72; //72 inches is from knee joint to ankle joint

//angles in degrees - hip is zero when straight out from the robot body
//hip angle is zero when the thigh is parallel to the ground
//knee angle is between thigh and calf (13 degrees fully retracted) 
//
//float currentX;
//float currentY;
//float currentZ;

float currentPos[] = {0.00, 0.00, 0.00};

void setup() {
  Serial.begin(9600); 
//  Serial.setTimeout(10); 

  for (int i = 0; i < 3; i++) {
   PWM_value[i] = (PWM_percent[i]/100) * bit_resolution;
  }
 //setup analog wite resolution and PWM frequency

///*disable when testing on < teensy 3.0
 
  analogWriteResolution(10);
  //max PWM freqency of the motor driver board is 20kHz
  analogWriteFrequency(3, 20000);
  analogWriteFrequency(5, 20000);
 
//*/ 
 
  //setup deadman, enable, and error digital I/O pins 
  pinMode(deadMan_pin, INPUT);
  pinMode(enable_pin, OUTPUT);
  digitalWrite(enable_pin, LOW);
  pinMode(enable_pin_hip, OUTPUT);
  digitalWrite(enable_pin_hip, LOW);

}

void loop() {
  
   //Read deadman first
   deadMan = digitalRead(deadMan_pin);
   //deadMan = 1;  //This is here for debugging but can be seriously dangious on the real robot if not commented out
   if (deadMan == 0) { // if deadman is off -- joystick is deactivated, then disable driver boards and write zero to PWM lines
     all_off();
   }
   else {
      digitalWrite(enable_pin, HIGH);
      digitalWrite(enable_pin_hip, HIGH);
   } 

  //take sensor readings
 // Serial.print("current angles (hip, thigh, knee): (");
  for (int i = 0; i < 3; i++) {
    sensorReading[i] = analogRead(sensorPin[i]);
    currentAngles[i] = sensorToAngle(i, sensorReading[i]);
    currentAnglesR[i] = ((currentAngles[i] * 71)/4068);
    Serial.print(currentAngles[i]);
    if (i == 2) {
      Serial.print('\n');
    }
    else {
      Serial.print('\t');
    }
  }

  //publish current (x,y,z);
//  currentPos[0] = cos(currentAnglesR[0]) * (L1 + L2*cos(currentAnglesR[1]) + L3*cos(currentAnglesR[1] + currentAnglesR[2] - pi));
//  currentPos[1] = currentPos[0] * tan(currentAnglesR[0]);
//  currentPos[2] = (L2 * sin(currentAnglesR[1])) + (L3 * sin(currentAnglesR[1] + currentAnglesR[2] - pi));

  //short version of angles and x,y,z
  //format:
  //x y z hipangle  thighangle  kneeangle
//  Serial.print(currentPos[0]);
//  Serial.print('\t');
//  Serial.print(currentPos[1]);
//  Serial.print('\t');
//  Serial.print(currentPos[2]);
//  Serial.println('\n');
  
  while (Serial.available() > 0) {

    commandAngle[0] = Serial.parseFloat();
    commandAngle[1] = Serial.parseFloat();
    commandAngle[2] = Serial.parseFloat();
    
//    Serial.print("goal angle (hip, thigh, knee): ");
//    Serial.print("(");
//    Serial.print(commandAngle[0]);
//    Serial.print(", ");
//    Serial.print(commandAngle[1]);
//    Serial.print(", ");
//    Serial.print(commandAngle[2]);
//    Serial.println(")");
    
    //look for newline or carriage return 
    if (Serial.read() == '\n') {
      //now calculate angle to sensor reading

      //Serial.println("GOT SOMETHING!!!!");
      
      for (int i=0; i<3; i++) { 
      sensorGoal[i] = angleToSensor(i, commandAngle[i]);
      goingHot[i] = 1; 
      }
    }
  }
      
   //Now check sensors and compare the readings to sensor goals set above
   for (int i = 0; i < 3; i++) {
   if (goingHot[i] == 1 && deadMan == 1) {
    //compare current sensor reading to goal.
    //if sensor reading is within a % of goal then stop and set goingHot to zero
    //read sensor
    sensorReading[i] = analogRead(sensorPin[i]);
    //compare sensor reading to goal and only move if not close enough
    if (abs(sensorReading[i] - sensorGoal[i]) >= closeEnough) {
      
      if (sensorReading[i] > sensorGoal[i]) {
        if (i == 2) { // i == 2 is the knee
          analogWrite(kneePWM2_extend, PWM_value[i]); 
          //print_reading(i, sensorReading[i], sensorGoal[i], "extending knee");
        }
        else if (i == 1) { // i == 1 is the thigh
          analogWrite(thighPWM1_up, PWM_value[i]);
          //print_reading(i, sensorReading[i], sensorGoal[i], "bringing thigh up");
        }      
        else if (i == 0) { // hip
          analogWrite(hipPWM2_reverse, PWM_value[i]);
          //print_reading(i, sensorReading[i], sensorGoal[i], "bringing hip back - reverse");
        }
      }
      else if (sensorReading[i] < sensorGoal[i]) {
        if (i == 2) { // i == 0 is the knee
          analogWrite(kneePWM1_retract, PWM_value[i]); 
          //print_reading(i, sensorReading[i], sensorGoal[i], "retracting knee");
        }
        else if (i == 1) { // i == 1 is the thigh
          analogWrite(thighPWM2_down, PWM_value[i]);
          //print_reading(i, sensorReading[i], sensorGoal[i], "bringing thigh down");
        }      
        else if (i == 0) { // hip
          analogWrite(hipPWM1_forward, PWM_value[i]);
          //print_reading(i, sensorReading[i], sensorGoal[i], "bringing hip forward");
        }
      }
    }
    else {
      goingHot[i] = 0;  //if joint was in motion and the goal was reached - turn motion flag and solenoids off
      if (i == 2) { //if knee was moving - turn both solenoids off
        analogWrite(kneePWM2_extend, 0);
        analogWrite(kneePWM1_retract, 0);
      }
      if (i == 1) {
        analogWrite(thighPWM2_down, 0);
        analogWrite(thighPWM1_up, 0);
      } 
      if (i == 0) {
        analogWrite(hipPWM1_forward, 0);
        analogWrite(hipPWM2_reverse, 0);
      }
//      Serial.println(">>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<");
//      Serial.println("reached goal");
    }
   } 
   }
  
  
  //this is a short delay so I can read the serial while programing
  delay(10);
}

//This is a simple program to make sure the driver board is not enabled if the joystick is turned off (deadMan is low)
void all_off() {
  digitalWrite(enable_pin, LOW);
  digitalWrite(enable_pin_hip, LOW);
  for (int i = 0; i < 6; i++) {
    analogWrite(pwm_pins[i], 0);
  }
  for (int i = 0; i < 3; i++) {
  goingHot[i] = 0;
  }
  //Serial.println("Joystick is deactivated -- all off!");
}


//int i, int sensor, int goal, String 'joint'

void print_reading(int i, int sensor, int goal, String joint) {
  Serial.print(i);
  Serial.print('\t');
  Serial.print(sensor);
  Serial.print('\t');
  Serial.print(goal);
  Serial.print('\t');
  Serial.println(joint);
}

float sensorToAngle(int joint, int sensorReading) {
  static int cylinderMinLength; 
  static int cylinderMaxLength;
  static float cylinderTravel;
  float currentCylinderLength;
  static float C1; //This is the length of one side of the triangle - not the cylinder
  static float C2; ; //This is the other side
  static float beta; //This is a constant angle between the changing angle calculated here (alpha) and the desired angle - e.g. theta1 for hip
  float alpha; //This is the angle that is changing when the cylinder length changes
  static float deadBugTheta; //This is the desired angle (e.g. theta2 plus beta and alpha) at deadbug (fully retracted and centered)
  float theta; //This is the final ouput angle 
  static int sensorMax;
  static int sensorMin;

  switch (joint) {
        case 0:
          //Serial.println("HIP");
          sensorMax = 722;
          sensorMin = 93;
          cylinderMinLength = 16;
          cylinderTravel = 8;
          C1 = 6.83905;
          C2 = 19.62051;
          beta = 77.92503;
          deadBugTheta = -7.8707;
          break;
        case 1:
          //Serial.println("THIGH");
          sensorMax = 917;
          sensorMin = 34;
          cylinderMinLength = 24;
          cylinderTravel = 14;
          C1 = 10.21631;
          C2 = 33.43093;
          beta = 26.6594;
          deadBugTheta = 129.6249;
          break;
        case 2:
          //Serial.println("KNEE");
          sensorMax = 934;
          sensorMin = 148;
          cylinderMinLength = 20;
          cylinderTravel = 12;
          C1 = 25.6021;
          C2 = 7.4386;
          beta = 35.8658;
          deadBugTheta = 194.1805;
          break;
      }
        float sensorUnitsPerInch = (sensorMax - sensorMin) / (cylinderTravel);
//      Serial.print("sensor units per inch: ");
//      Serial.println(sensorUnitsPerInch);
      int sensorReading_sensorMin = sensorReading - sensorMin;
//      Serial.print("sensorReading_sensorMin: ");
//      Serial.println(sensorReading_sensorMin);
      
      currentCylinderLength = ((sensorReading - sensorMin) / sensorUnitsPerInch) + cylinderMinLength;
//      Serial.print("currentCylindarLength: ");
//      Serial.println(currentCylinderLength);
      
      float alphaR = acos((sq(C1) + sq(C2) - sq(currentCylinderLength))/(2*C1*C2));
      alpha = (alphaR * 4068) / 71;
      if (joint == 0) { 
        //The hip is inverted logic because the sensor is at minimum when the joint is at minimum angle 
        //The other joints are at their maximum angle whne the sensor is fully retracted                
        theta = deadBugTheta - beta + alpha;
      }
      else {
        theta = deadBugTheta - beta - alpha;
      }
//      Serial.print("alpha: ");
//      Serial.println(alpha);
//      Serial.print("theta in void: ");
//      Serial.println(theta);

     // float thetaR = (theta * 71) / 4068;
      return theta;
}

float angleToSensor(int joint, float angle) {
  static int cylinderMinLength; 
  static int cylinderMaxLength;
  static float cylinderTravel;
  float currentCylinderLength;
  static float C1; //This is the length of one side of the triangle - not the cylinder
  static float C2; ; //This is the other side
  static float beta; //This is a constant angle between the changing angle calculated here (alpha) and the desired angle - e.g. theta1 for hip
  float alpha; //This is the angle that is changing when the cylinder length changes
  static float deadBugTheta; //This is the desired angle (e.g. theta2 plus beta and alpha) at deadbug (fully retracted and centered)
  static float sensorMax;
  static float sensorMin;
  float sensorGoal; //This is the final output from joint and angle input

  switch (joint) {
        case 0:
          //Serial.println("HIP");
          sensorMax = 722.00;
          sensorMin = 93.00;
          cylinderMinLength = 16;
          cylinderTravel = 8;
        //  cylinderMaxLength = cylinderMinLength + cylinderTravel;
          C1 = 6.83905;
          C2 = 19.62051;
          beta = 77.92503;
          deadBugTheta = -7.8707;
          break;
        case 1:
          //Serial.println("THIGH");
          sensorMax = 917.00;
          sensorMin = 34.00;
          cylinderMinLength = 24;
          cylinderTravel = 14;
        //  cylinderMaxLength = cylinderMinLength + cylinderTravel;
          C1 = 10.21631;
          C2 = 33.43093;
          beta = 26.6594;
          deadBugTheta = 129.6249;
          break;
        case 2:
          //Serial.println("KNEE");
          sensorMax = 934.00;
          sensorMin = 148.00;
          cylinderMinLength = 20;
          cylinderTravel = 12;
        //  cylinderMaxLength = cylinderMinLength + cylinderTravel;
          C1 = 25.6021;
          C2 = 7.4386;
          beta = 35.8658;
          deadBugTheta = 194.1805;
          break;
      }
      
      //calculate alpha (the internal angle opposite the cylinder in the cylinder triangle
      if (joint == 0) { 
        //The hip is inverted logic because the sensor is at minimum when the joint is at minimum angle 
        //The other joints are at their maximum angle whne the sensor is fully retracted                
        alpha = angle - deadBugTheta + beta;
      }
      else {
        alpha = deadBugTheta - beta - angle;
      }
      float alphaR = (alpha * 71) / 4068;
//      Serial.print("alpha: ");
//      Serial.println(alpha);
      
      //calculate cylinder length from alpha
      float cylinderGoalLength = sqrt((sq(C2) + sq(C1) - (2*C1*C2*cos(alphaR))));
      float pistonGoal = cylinderGoalLength - cylinderMinLength;
      float sensorUnitsPerInch = (sensorMax - sensorMin) / (cylinderTravel);
//      Serial.print("sensor units per inch: ");
//      Serial.println(sensorUnitsPerInch);
      sensorGoal = (pistonGoal * sensorUnitsPerInch) + sensorMin;
   
      return sensorGoal;           
}


