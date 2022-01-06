#include <Servo.h>
#define SERVO_PIN     11
#define LPT 2 // scan loop coumter
#define BUZZ_PIN     13

#define IN1  7    //Right motor(K1/K2) direction Pin 1
#define IN2  8    //Right motor(K1/K2) direction Pin 2
#define IN3  9    //Left motor(K3/K4) direction Pin 1
#define IN4  10   //Left motor(K3/K4) direction Pin 2
#define ENA  5    //ENA PWM speed pin for Right motor(K1/K2)
#define ENB  6    //ENB PWM speed pin for Left motor(K3/K4)

#define Echo_PIN    2 // Ultrasonic Echo pin connect to D11
#define Trig_PIN    3  // Ultrasonic Trig pin connect to D12

#define FAST_SPEED  150 
#define SPEED  100   
#define TURN_SPEED  150
#define BACK_SPEED1  100
#define BACK_SPEED2  90 

int leftscanval, centerscanval, rightscanval, ldiagonalscanval, rdiagonalscanval;
const int distancelimit = 12; //distance limit for obstacles in front           
const int sidedistancelimit = 12; //minimum distance in cm to obstacles at both sides (the car will allow a shorter distance sideways)
int distance;
int numcycles = 0;

const int turntime = 400; //Time the robot spends turning (miliseconds)
const int backtime = 400; //Time the robot spends turning (miliseconds)

int thereis;
Servo head;

void go_Advance()  //motor rotate clockwise -->robot go ahead
{
  digitalWrite(IN4,LOW);
  digitalWrite(IN3,HIGH); 
  digitalWrite(IN2,LOW);
  digitalWrite(IN1,HIGH);
}
void go_Back() //motor rotate counterclockwise -->robot go back
{
  digitalWrite(IN4,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN2,HIGH );
  digitalWrite(IN1,LOW);
}
void stop_Stop() //motor brake -->robot stop
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4,LOW); 
  set_Motorspeed(0,0);
}
void go_Right()  //left motor rotate clockwise and right motor rotate counterclockwise -->robot turn right
{
  digitalWrite(IN4,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN1,HIGH);
  set_Motorspeed(SPEED,0);
  
}
void go_Left() //left motor rotate counterclockwise and right motor rotate clockwise -->robot turn left
{
  digitalWrite(IN4,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN1,LOW);
  set_Motorspeed(0,SPEED);
 
}
/*set motor speed */
void set_Motorspeed(int lspeed,int rspeed) //change motor speed
{
  analogWrite(ENB,lspeed);//lspeed:0-255
  analogWrite(ENA,rspeed);//rspeed:0-255   
}


int watch(){
  long echo_distance;
  digitalWrite(Trig_PIN,LOW);
  delayMicroseconds(5);                                                                              
  digitalWrite(Trig_PIN,HIGH);
  delayMicroseconds(15);
  digitalWrite(Trig_PIN,LOW);
  echo_distance=pulseIn(Echo_PIN,HIGH);
  echo_distance=echo_distance*0.01657; //how far away is the object in cm
  Serial.println((int)echo_distance);
  return round(echo_distance);
}

String watchsurrounding(){
/*  obstacle_status is a binary integer, its last 5 digits stands for if there is any obstacles in 5 directions,
 *   for example B101000 last 5 digits is 01000, which stands for Left front has obstacle, B100111 means front, right front and right ha
 */
 
int obstacle_status =B100000;
  centerscanval = watch();
  if(centerscanval<distancelimit){
    stop_Stop();
    obstacle_status  =obstacle_status | B100;
    }
  head.write(120);
  delay(100);
  ldiagonalscanval = watch();
  if(ldiagonalscanval<distancelimit){
    stop_Stop();
     obstacle_status  =obstacle_status | B1000;
    }
  head.write(170); //Didn't use 180 degrees because my servo is not able to take this angle
  delay(300);
  leftscanval = watch();
  if(leftscanval<sidedistancelimit){
    stop_Stop();
     obstacle_status  =obstacle_status | B10000;
    }

  head.write(90); //use 90 degrees if you are moving your servo through the whole 180 degrees
  delay(100);
  centerscanval = watch();
  if(centerscanval<distancelimit){
    stop_Stop();
    obstacle_status  =obstacle_status | B100;
    }
  head.write(40);
  delay(100);
  rdiagonalscanval = watch();
  if(rdiagonalscanval<distancelimit){
    stop_Stop();
    obstacle_status  =obstacle_status | B10;
    }
  head.write(0);
  delay(100);
  rightscanval = watch();
  if(rightscanval<sidedistancelimit){
    stop_Stop();
    obstacle_status  =obstacle_status | 1;
    }
  head.write(90); //Finish looking around (look forward again)
  delay(300);
   String obstacle_str= String(obstacle_status,BIN);
  obstacle_str= obstacle_str.substring(1,6);
  
  return obstacle_str; //return 5-character string standing for 5 direction obstacle status
}

void auto_avoidance(){

  ++numcycles;
  if(numcycles>=LPT){ //Watch if something is around every LPT loops while moving forward 
     stop_Stop();
    String obstacle_sign=watchsurrounding(); // 5 digits of obstacle_sign binary value means the 5 direction obstacle status
      Serial.print("begin str=");
      Serial.println(obstacle_sign);
      if( obstacle_sign=="10000"){
      Serial.println("SLIT right");
      set_Motorspeed(FAST_SPEED,SPEED);
      go_Advance();
 
      delay(turntime);
      stop_Stop();
    }
     else    if( obstacle_sign=="00001"  ){
     Serial.println("SLIT LEFT");
     set_Motorspeed(SPEED,FAST_SPEED);
     go_Advance();
  
     delay(turntime);
     stop_Stop();
    }
    else if( obstacle_sign=="11100" || obstacle_sign=="01000" || obstacle_sign=="11000"  || obstacle_sign=="10100"  || obstacle_sign=="01100" ||obstacle_sign=="00100"  ||obstacle_sign=="01000" ){
     Serial.println("hand right");
     go_Right();
     set_Motorspeed(TURN_SPEED,TURN_SPEED);
     delay(turntime);
     stop_Stop();
    } 
    else if( obstacle_sign=="00010" || obstacle_sign=="00111" || obstacle_sign=="00011"  || obstacle_sign=="00101" || obstacle_sign=="00110" || obstacle_sign=="01010" ){
    Serial.println("hand left");
    go_Left();//Turn left
    set_Motorspeed(TURN_SPEED,TURN_SPEED);
    delay(turntime);
    stop_Stop();
    }
 
    else if(  obstacle_sign=="01111" ||  obstacle_sign=="10111" || obstacle_sign=="11111"  ){
    Serial.println("hand back left");
    go_Back();
    set_Motorspeed( BACK_SPEED1,BACK_SPEED2);
    delay(backtime);
    stop_Stop();
        } 
    else if( obstacle_sign=="11011"  ||    obstacle_sign=="11101"  ||  obstacle_sign=="11110"  || obstacle_sign=="01110"  ){
    Serial.println("hand back right");
    go_Back();
    set_Motorspeed(BACK_SPEED2,BACK_SPEED1);
    delay(backtime);
    stop_Stop();
        }    
  
    else Serial.println("no handle");
    numcycles=0; //Restart count of cycles
  } else {
    set_Motorspeed(SPEED,SPEED);
    go_Advance();  // if nothing is wrong go forward using go() function above.
    delay(backtime);
    stop_Stop();
  }
  
  //else  Serial.println(numcycles);
  
  distance = watch(); // use the watch() function to see if anything is ahead (when the robot is just moving forward and not looking around it will test the distance in front)
  if (distance<distancelimit){ // The robot will just stop if it is completely sure there's an obstacle ahead (must test 25 times) (needed to ignore ultrasonic sensor's false signals)
 Serial.println("final go back");
  go_Back();
  set_Motorspeed(BACK_SPEED1,BACK_SPEED2);
  delay(backtime);
      ++thereis;}
  if (distance>distancelimit){
      thereis=0;} //Count is restarted
  if (thereis > 25){
  Serial.println("final stop");
    stop_Stop(); // Since something is ahead, stop moving.
    thereis=0;
  }
}

//******************************************************************************************************
// Bluetooth Variables & Setup
int i;
String str;                                                        // raw string received from android to arduino
int blueToothVal;                                                  // stores the last value sent over via bluetooth
//Bluetooth
void bluetooth()
{
 while (Serial.available())                                    // Read bluetooth commands over Serial1 // Warning: If an error with Serial1 occurs, make sure Arduino Mega 2560 is Selected
 {  
  {  
      str = Serial.readStringUntil('\n');                      // str is the temporary variable for storing the last sring sent over bluetooth from Android device
      //Serial.print(str);                                      // for testing purposes
  } 
    
    blueToothVal = (str.toInt());                               //  convert the string 'str' into an integer and assign it to blueToothVal
    Serial.print(F("BlueTooth Value "));
    Serial.println(blueToothVal);    


 switch (blueToothVal) 
 {
      case 0:                                
        Serial.println(F("Forward"));
        go_Advance();
        set_Motorspeed(80,80);
        break;

      case 4:                 
        Serial.println(F("Reverse"));
        go_Back();
        set_Motorspeed(80,80);
        break;

      case 2:         
        Serial.println(F("Left"));
        go_Left();
        set_Motorspeed(80,0);
        break;
        
      case 1:                     
        Serial.println(F("Right"));
        go_Right();
        set_Motorspeed(0,80);
        break;
        
      case 3:
      i=3;                                            
        Serial.println(F("Stop Car"));
        stop_Stop();
        break; 

      case 5:
      i=0; // finish with waypoints
      while (i<=20){
        Serial.print(F("Obstacle Avoidance ON"));
        auto_avoidance();
        i++;
      }
        break;
      

 } // end of switch case
}
}

void setup() {
  /******L298N******/
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT); 
  pinMode(ENA, OUTPUT);  
  pinMode(ENB, OUTPUT);

  stop_Stop();//stop move
  /*init HC-SR04*/
  pinMode(Trig_PIN, OUTPUT); 
  pinMode(Echo_PIN,INPUT); 
  digitalWrite(Trig_PIN,LOW);
  /*init servo*/
  head.attach(SERVO_PIN); 
  head.write(90);
  delay(2000);
  Serial.begin(9600);
 
}


void loop(){ 
 bluetooth();
}
