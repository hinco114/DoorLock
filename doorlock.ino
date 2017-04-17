/*
Arduino Door lock project
SerialCommunication Numbering 
1 : Enroll
2 : Delete


해야할것 : 시리얼 전부 해제하기
*/

//FingerPrints Header
#include "FPS_GT511C3.h"
//SoftwareSerial Header
#include "SoftwareSerial.h"
//Ultrasonic Sensor
#include "NewPing.h"




//set of PIN
#define ENR 12
#define IDN 11
#define DOOR 10
#define LR 4
#define LY 5
#define LG 6
#define TRIGGER_PIN  8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
FPS_GT511C3 fps(2, 3);

//Global Variable
int incommingVal = 0;
String income = "";
boolean stringComplete = false;
int dtcnt=0;

//Needed default settings
void setup() {
	pinMode(ENR,INPUT_PULLUP);
	pinMode(IDN,INPUT_PULLUP);
	pinMode(DOOR,OUTPUT);
	Serial.begin(9600);
	fps.Open();
	digitalWrite(DOOR,1);		//Default Signal is 1 
	fps.SetLED(false);
} 
/*
void serialEvent() {
	incommingVal = Serial.read();
}
*/

void serialEvent() {
	while(Serial.available())
	{
		//if(Serial.read()== 13){
		//	Serial.println("ddd");
		//	break;
		//}
		char r = (char)Serial.read();
		income = income + r;
		if (r=='\n')
			stringComplete=true;
	}
}

void Ledon(int led_num){
	pinMode(led_num,OUTPUT);
	digitalWrite(led_num,1);
}

void Ledoff(int led_num){
	pinMode(led_num,OUTPUT);
	digitalWrite(led_num,0);
}

void Opendoor() {
	digitalWrite(DOOR,0);
	delay(50);
	digitalWrite(DOOR,1);
	Serial.println("Door Opend!");
	Ledon(LG);
	delay(1000);
	Ledoff(LG);
	delay(500);
	Ledon(LG);
	delay(1000);
	Ledoff(LG);
}

void Identify() {
	// Identify fingerprint test
	fps.SetLED(true);
	Serial.println("Please press finger");
	int sc =0;
	while(sc<50)
	{
		Ledon(LY);
		if (fps.IsPressFinger())
		{
			fps.CaptureFinger(false);
			int id = fps.Identify1_N();
			if (id <200)
			{
				Serial.print("Verified ID:");
				Serial.println(id);
				Ledoff(LY);
                Opendoor();
				break;
			}
			else
			{
				Serial.println("Finger not found");
				Ledoff(LY);
				Ledon(LR);
				delay(2000);
				Ledoff(LR);
				break;
			}
		}
		else
		{
			//Serial.println("Please press finger");
		}
		Ledoff(LY);
		sc++;
		delay(100);
	}
	Ledoff(LY);
	fps.SetLED(false);
}

void Enroll() {
	// find open enroll id
	fps.SetLED(true);
	int enrollid = 0;
	bool usedid = true;
	while (usedid == true)
	{
		usedid = fps.CheckEnrolled(enrollid);
		if (usedid==true) enrollid++;
	}
	fps.EnrollStart(enrollid);

	// enroll
	Serial.print("Press finger to Enroll user num #");
	Serial.println(enrollid);
	while(fps.IsPressFinger() == false) delay(100);
	bool bret = fps.CaptureFinger(true);
	int iret = 0;
	if (bret != false)
	{
		Serial.println("Remove finger");
		Ledon(LR);
		fps.Enroll1(); 
		while(fps.IsPressFinger() == true) delay(100);
		Serial.println("Press same finger again");
		while(fps.IsPressFinger() == false) delay(100);
		bret = fps.CaptureFinger(true);
		if (bret != false)
		{
			Serial.println("Remove finger");
			Ledon(LY);
			fps.Enroll2();
			while(fps.IsPressFinger() == true) delay(100);
			Serial.println("Press same finger yet again");
			while(fps.IsPressFinger() == false) delay(100);
			bret = fps.CaptureFinger(true);
			if (bret != false)
			{
				Serial.println("Remove finger");
				Ledon(LG);
				iret = fps.Enroll3();
				if (iret == 0)
				{
					Serial.println("Enrolling Successfull");
				}
				else
				{
					Serial.print("Enrolling Failed with error code:");
					Serial.println(iret);
				}
			}
			else Serial.println("Failed to capture third finger");
		}
		else Serial.println("Failed to capture second finger");
	}
	else Serial.println("Failed to capture first finger");
	Ledoff(LY);
	Ledoff(LR);
	Ledoff(LG);
	fps.SetLED(false);
	Ledon(LG);
	Ledon(LY);
	Ledon(LR);
	delay(500);
	Ledoff(LY);
	Ledoff(LR);
	Ledoff(LG);
	delay(500);
	Ledon(LY);
	Ledon(LR);
	Ledon(LG);
	delay(500);
	Ledoff(LY);
	Ledoff(LR);
	Ledoff(LG);
}

int Delete(int id) {
	if(fps.CheckEnrolled(id)==0) {
		Serial.println(" Not Found");
		return 0;
	}
	fps.DeleteID(id);
	Serial.println(" Deleted");
	return 1;
}

void loop(){
	if(digitalRead(ENR)==0) //등록버튼
		Enroll();	
	if((digitalRead(IDN)==0)) //인식버튼
		Identify();
	if(sonar.ping_cm()<=100){
		while((sonar.ping_cm()<=100)&& (dtcnt<=15)){
			Serial.println(sonar.ping_cm());
			delay(100);
			dtcnt++;
		}
		if(dtcnt>=15)
			Identify();
		dtcnt = 0;
	}
	if(stringComplete){		//시리얼로 읽은값이 있을 때
	//첫문자가 1 입력된 경우
		if(income[0]=='1'){
			Opendoor();
		}
	//첫문자가 2 입력된 경우
		else if(income[0]=='2'){
			income.remove(0,2);	//income에서 0번째부터 2개 지움
			int user = income.toInt();	//user에 user번호 저장
			Serial.print("User #");
			Serial.print(user);
			Delete(user);	//user 삭제
		}
		income="";
		stringComplete=false;
	}
	delay(100);
}