#define BLYNK_SSL_USE_LETSENCRYPT

#include <BlynkSimpleEsp32_SSL.h>
#include <NewPing.h>
#include <MedianFilter.h>

char auth[] = "Your Auth Code";
char ssid[] = "WiFi SSID";
char pass[] = "WiFi Password";

// Define Motor Pin:
#define motorPin 25

// Define Trig and Echo pin:
#define trigPin_UG 33
#define echoPin_UG 35
#define trigPin_OH 32
#define echoPin_OH 34

// Define maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500 cm:
#define MAX_DISTANCE 400

// NewPing setup of pins and maximum distance.
NewPing sonar_UG = NewPing(trigPin_UG, echoPin_UG, MAX_DISTANCE);
NewPing sonar_OH = NewPing(trigPin_OH, echoPin_OH, MAX_DISTANCE);

MedianFilter filter_UG(31,0);
MedianFilter filter_OH(31,0);

int motor_auto = 1, motor = 0;
int ug_percentage = 0, ug_minimum = 10, ug_maximum = 100, ug_tank_alert = 0;
int oh_percentage = 0, oh_minimum = 50, oh_maximum = 100, oh_tank_alert = 0;

void setup()
{
 Serial.begin(115200); //Debug console
 delay(10);
 
 pinMode(motorPin,OUTPUT); //Motor
 pinMode(trigPin_UG,OUTPUT); //Trigger UG
 digitalWrite(trigPin_UG,LOW); //Trigger UG
 delayMicroseconds(500); // Wait a half a second
 pinMode(echoPin_UG,INPUT); //ECHO UG
 pinMode(trigPin_OH,OUTPUT); //Trigger OH
 digitalWrite(trigPin_OH,LOW); //Trigger OH
 delayMicroseconds(500); // Wait a half a second
 pinMode(echoPin_OH,INPUT); //ECHO OH
 
 Blynk.config(auth, "Blynk Server FQDN or IP Address", 443); //Connect to Blynk Cloud
 Blynk.connect();
}

void tank_motor()
{
 //Underground Tank
 delay(50);                      // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
 unsigned int o_UG,uS_UG = sonar_UG.ping(); // Send ping, get ping time in microseconds (uS).

 filter_UG.in(uS_UG);
 o_UG = filter_UG.out();
 unsigned int ug_distance = (o_UG / US_ROUNDTRIP_CM); // Convert ping time to distance in cm and print result (0 = outside set distance range)
 if(ug_distance<=130 && ug_distance>=25)
 {
  ug_percentage = (105 + 25 - ug_distance) / 1.05; // Calculating the percentage, 25 is empty space between sensor and water
  Blynk.virtualWrite(V4, ug_percentage);
 }
 
 //Underground Tank Notification
 if ((ug_percentage >= ug_maximum) && (ug_tank_alert != 1))
 {
   Blynk.notify(String("Underground Tank is FULL. Percentage: ") + ug_percentage);
   ug_tank_alert = 1;    
 }
 else if ((oh_percentage <= oh_minimum) && (ug_tank_alert != 2))
 {
  Blynk.notify(String("Underground Tank is EMPTY. Percentage: ") + ug_percentage);
  ug_tank_alert = 2;    
 }
 
 //Overhead Tank
 delay(50);                      // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
 unsigned int o_OH,uS_OH = sonar_OH.ping(); // Send ping, get ping time in microseconds (uS).

 filter_OH.in(uS_OH);
 o_OH = filter_OH.out();
 unsigned int oh_distance = (o_OH / US_ROUNDTRIP_CM); // Convert ping time to distance in cm and print result (0 = outside set distance range)
 if(oh_distance<=72 && oh_distance>=25)
 {
  oh_percentage = (47 + 25 - oh_distance) / 0.47; // Calculating the percentage, 25 is empty space between sensor and water
  Blynk.virtualWrite(V7, oh_percentage);
 }
 
 //Overhead Tank Notification
 if ((oh_percentage >= oh_maximum) && (oh_tank_alert != 1))
 {
   Blynk.notify(String("Overhead Tank is FULL. Percentage: ") + oh_percentage);
   oh_tank_alert = 1;    
 }
 else if ((oh_percentage <= oh_minimum) && (oh_tank_alert != 2))
 {
  Blynk.notify(String("Overhead Tank is EMPTY. Percentage: ") + oh_percentage);
  oh_tank_alert = 2;    
 }
 
 //Motor
 if(motor_auto==1 && motor==0)
 {
  if(oh_percentage<=oh_minimum && ug_percentage>=ug_minimum)
  {
   digitalWrite(motorPin,LOW); //Low Level Trigger Relay
   motor = 1;
   Blynk.notify("Powering ON Motor");
  }
 }
 else if(motor==1)
 {
  if((oh_percentage>=oh_maximum) || (ug_percentage<ug_minimum))
  {
   digitalWrite(motorPin,HIGH); //Low Level Trigger Relay
   motor = 0;
   Blynk.notify("Powering OFF motor");
  }
 }
}

BLYNK_WRITE(0)
{
 motor_auto = param.asInt();
}

BLYNK_WRITE(1)
{
 if(param.asInt() && motor_auto==0)
 {
  digitalWrite(motorPin,LOW); //Low Level Trigger Relay
  motor=1;
 }
 else
 {
  if(motor_auto==0)
  {
   digitalWrite(motorPin,HIGH); //Low Level Trigger Relay
   motor=0;
  }
 }
}

BLYNK_WRITE(2)
{
 ug_minimum=param.asInt();
}

BLYNK_WRITE(3)
{
 ug_maximum=param.asInt();
}

BLYNK_WRITE(5)
{
 oh_minimum=param.asInt();
}

BLYNK_WRITE(6)
{
 oh_maximum=param.asInt();
}

void loop()
{
 Blynk.run();
 tank_motor();
}
