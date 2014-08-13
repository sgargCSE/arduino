#include <EEPROM.h>

#define trigPin 12
#define echoPin 11
#define light 10
#define led 13

#define DIST 200

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
    for (i=2;i<9;i++) {
      pinMode(i,OUTPUT);
    }
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

    cli();
    //set timer1 interrupt at 10Hz
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = 157;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1A |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    sei();

    delay(1000);
}


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

    if (distance < DIST) {        
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
    if (zeros >= 6){
        digitalWrite(light, LOW);
        //permanent stall mode
        while (1==1){
            digitalWrite(led, LOW);
            delay(1000);  
            digitalWrite(led, HIGH);
            delay(1000);
        }
    }
    int j;
    for (j=0;j<4;j++){
        digits[j] = distance % 10;
        distance = distance / 10;
    }
    delay(125); // delay 250ms
}

int getDistance(){
    long duration, distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration / 58;

    if (distance == 0) {
        zeros++;
        EEPROM.write(0,EEPROM.read(0)+1);
    }

    return distance;
}

void writeShiftReg(int *data){
    int i;
    int writeout = 0;

    for (i=0;i<8;i++){
        writeout = writeout | (data[i] << i);
    }

    digitalWrite(LATCHpin, LOW);
    shiftOut(DATApin, CLKpin, MSBFIRST, writeout);
    digitalWrite(LATCHpin, HIGH);
}

int digitUpTo = 0;
int digitPins[4] = {6,7,8,9};

ISR(TIMER1_COMPA_vect){//timer1 interrupt 10Hz
    digitalWrite(digitPins[digitUpTo%4], LOW);
    digitUpTo++;
    digitalWrite(digitPins[digitUpTo%4], HIGH);
    writeShiftReg(numbers[digits[digitUpTo%4]]);
}
