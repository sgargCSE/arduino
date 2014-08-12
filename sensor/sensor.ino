#define trigPin 12
#define echoPin 11
#define light 10
#define led 13

#define DIST 200

int global;
int numbers[10][7] = 
{ { 1,1,1,1,1,1,0 }, // = 0
{ 0,1,1,0,0,0,0 }, // = 1
{ 1,1,0,1,1,0,1 }, // = 2
{ 1,1,1,1,0,0,1 }, // = 3
{ 0,1,1,0,0,1,1 }, // = 4
{ 1,0,1,1,0,1,1 }, // = 5
{ 1,0,1,1,1,1,1 }, // = 6
{ 1,1,1,0,0,0,0 }, // = 7
{ 1,1,1,1,1,1,1 }, // = 8
{ 1,1,1,0,0,1,1 }}; // = 9

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
    

    //write this out to the 7seg display
    int j;
    for (j=2;j<9;j++){
      if(numbers[global%10][j-2] == 0){
        digitalWrite(j,HIGH);
      }else{
         digitalWrite(j,LOW); 
      }
    }
    delay(250); // delay 250ms
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
    return distance;
}
