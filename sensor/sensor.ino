//#include <EEPROM.h>

#define trigPin 12
#define echoPin 11
#define light 10
#define led 13

#define DIST 150
#define LIGHT_VAL 500

int global;
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
//    int rr = EEPROM.read(0);
//    Serial.println(rr);

    pinMode(CLKpin, OUTPUT);
    pinMode(LATCHpin, OUTPUT);
    pinMode(DATApin, OUTPUT);
}

int dist_cpy;

void loop() {

    digitalWrite(led, LOW);
    int distance = getDistance();
    Serial.print(distance);
    Serial.println(" cm");

    int sensorValue = analogRead(A0);            
    Serial.print("light val = ");
    Serial.println(sensorValue);

    //    if (distance > 3000) global++;
    if (distance < DIST){
        delay(25);
        //remeasurement
        distance = getDistance(); 
    }

    if (distance < DIST && sensorValue < LIGHT_VAL) {        
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
        while(i < (60 * 10 / 2)){
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
    // map it to the range of the analog out:
    
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
        //EEPROM.write(0,EEPROM.read(0)+1);
    }

    return dist;
}

