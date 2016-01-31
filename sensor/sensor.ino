#include <EEPROM.h>

#define trigPin 12
#define echoPin 11
#define light 10
#define led 13

#define DIST 150
#define LIGHT_VAL 380

int global;
int numbers[11][7] = { 
{ 1,1,1,1,1,1,0 }, // = 0
{ 0,1,1,0,0,0,0 }, // = 1
{ 1,1,0,1,1,0,1 }, // = 2
{ 1,1,1,1,0,0,1 }, // = 3
{ 0,1,1,0,0,1,1 }, // = 4
{ 1,0,1,1,0,1,1 }, // = 5
{ 1,0,1,1,1,1,1 }, // = 6
{ 1,1,1,0,0,0,0 }, // = 7
{ 1,1,1,1,1,1,1 }, // = 8
{ 1,1,1,0,0,1,1 }, // = 9
{ 0,0,1,1,1,1,1 }  // = E
}; 
int digits[4] = {0,0,0,0};
int zeros;
#define CLKpin 2
#define LATCHpin 3
#define DATApin 4

void setup() {
    Serial.begin (9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(light, OUTPUT);
    pinMode(led, OUTPUT);
    int i = 0;
    global = 0;
    zeros = 0;
        
    Serial.print("total number of zeroes read = ");
//    EEPROM.write(0,0);
    int rr = EEPROM.read(0);
    Serial.println(rr);
    
    int j;
    for (i=0;i<11;i++){
        for (j=0;j<7;j++){
            if (numbers[i][j] == 0){
                numbers[i][j] = 1;
            }else{
                numbers[i][j] = 0;
            }
        }
    }

    pinMode(CLKpin, OUTPUT);
    pinMode(LATCHpin, OUTPUT);
    pinMode(DATApin, OUTPUT);

    for (i=5;i<9;i++) {
      pinMode(i,OUTPUT);
    }
    
  delay(1000);
    
  cli();

  //set timer0 interrupt at 2kHz
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = 200;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR2B |= (1 << CS22) | (1 << CS20);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);


  sei();

}

int dist_cpy;
char glb = 0;
int outputValue = 0;

void loop() {

    digitalWrite(led, LOW);
    int distance = getDistance();
    Serial.print(distance);
    Serial.println(" cm");

    //    if (distance > 3000) global++;
    if (distance < DIST){
        delay(25);
        //remeasurement
        distance = getDistance(); 
    }

    if (distance < DIST && outputValue < LIGHT_VAL) {        
        global++;
        Serial.print(" <150cm trigger, on for 60 seconds(");
        Serial.print(global);
        Serial.println(")");
        //turn on light/Light for the first time
        digitalWrite(light,HIGH);

        int i = 0;
        int permanent = 0;
        //60 seconds (250ms * 240 = 60seconds)
        for (i=0;(i<240 || permanent == 1) && i < 1200;i++){
            distance = getDistance();
            if (distance < 10 && distance >0){
                Serial.print(distance);
                Serial.println(" cm");
                if (permanent == 0){
                    digitalWrite(led, HIGH);
                    Serial.println(" <10cm trigger: ON");
                    permanent = 1;
                    delay(10000);//10 seconds to get out of the way
                }else{
                    digitalWrite(led, LOW);
                    Serial.println(" <10cm trigger: OFF");
                    permanent = 0;
                    digitalWrite(light,LOW);
                    delay(10000);
                    //it was on, and was now force turned off...
                    i = 1000;
                }
            }
            delay(250); // delay 250ms
        }
    }
    digitalWrite(light,LOW);
    if (zeros >= 10){
        digitalWrite(light, LOW);
        //permanent stall mode
        int i = 0;
        while(i < (60 * 30 / 2)){
            digitalWrite(led, LOW);
            delay(1000);  
            digitalWrite(led, HIGH);
            delay(1000);
            i++;
        }
        zeros = 0;
    }
    int j;
    
    // read the analog in value:
    int sensorValue = analogRead(A0);            
    // map it to the range of the analog out:
    outputValue = sensorValue;
    
    glb++;
    if (glb == 16){
        dist_cpy = outputValue;
        glb = 0;
    }
    int tmp = dist_cpy;    
    for (j=0;j<4;j++){
        digits[j] = dist_cpy % 10;
        dist_cpy = dist_cpy / 10;
        Serial.print(digits[j]);
        Serial.print(" ");
    }
    dist_cpy = tmp;
    Serial.println("-----");
    delay(250); // delay 250ms
}

int getDistance(){
    cli();
    long duration, dist;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    dist = duration / 58;
    sei();
    if (dist == 0) {
        zeros++;
        EEPROM.write(0,EEPROM.read(0)+1);
    }

    return dist;
}

char digitUpTo = 3;
int digitPins[4] = {5,6,7,8};

ISR(TIMER2_COMPA_vect){

    digitalWrite(digitPins[digitUpTo%4], LOW);
    digitUpTo--;
    if (digitUpTo== -1) digitUpTo = 3;
    digitalWrite(digitPins[digitUpTo%4], HIGH);
    writeShiftReg(numbers[digits[digitUpTo%4]]);
}

void writeShiftReg(int *data){
    int i;
    byte writeout = 0;

    for (i=0;i<7;i++){
        writeout = writeout | (data[i] << i);
    }
    digitalWrite(LATCHpin, LOW);
    shiftOut(DATApin, CLKpin, MSBFIRST, writeout);
    digitalWrite(LATCHpin, HIGH);
}
