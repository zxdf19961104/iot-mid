#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"
#include "SimonSays.h"

#define TONE_PIN A0
#define NUMBER 4
#define FONT_SIZE 2 

#define OLED_MOSI   12
#define OLED_CLK   13
#define OLED_DC    10
#define OLED_CS    9
#define OLED_RESET 11
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

const int switches[NUMBER] = {7, 5, 3, 1};
const int leds[NUMBER] = {8, 6, 4, 2};
const int notes[NUMBER] = {
	NOTE_C4, NOTE_D4, NOTE_E4,NOTE_F4,
};
int life = 3;
int q_num = 3;
int *questions = NULL;
int *answers = NULL;
int answer_num = 0;
unsigned long lastClickTime;

State state = STATE_START;

int noteStart[] = {
	NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, 
	NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4
};
int noteDurationsStart[] = {16, 8, 16, 8, 16, 4, 16, 16, 16, 16, 8, 16, 4};

int noteCorrect[] = {NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_G4};
int noteDurationsCorrect[] = {32, 32, 32, 32, 32};

int noteWrong[] = {
	NOTE_D4,NOTE_D4
};
int noteDurationsWrong[] = {16, 16,};

Melody melodys[MELODY_MAX] = {
	{noteStart, noteDurationsStart, 13},
	{noteCorrect, noteDurationsCorrect, 5},
	{noteWrong, noteDurationsWrong, 2},
};

void playtone(int *note, int *noteDurations, int num){
  for(int thisNote = 0; thisNote < num; thisNote++){
    int noteDuration = 3000 / noteDurations[thisNote];
    tone(TONE_PIN, note[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
  }
}

void playMelody(Melody_Enum me){
	playtone(melodys[me].note, melodys[me].duration, melodys[me].number);
}
void splash(){
  display.clearDisplay(); 
  display.setTextSize(1);
  display.setCursor(45,0);
  display.print("Arduino");
  display.setTextSize(1);
  display.setCursor(48,10);
  display.print("Simon");
  display.setTextSize(1);
  display.setCursor(50,20);
  display.print("says");
  display.display();
  delay(2000);
  }
void LevelDisplay(){
  int level = q_num-2;
  if(level<10){ 
  display.clearDisplay(); 
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print("Level:");
  display.setCursor(70,10);
  display.print(level);
  display.setTextSize(1);
  display.setCursor(80,0);
  display.print("life:");
  display.setTextSize(1);
  display.setCursor(120,0);
  display.print(life);
  display.display();}
  if(level>=10){ 
  display.clearDisplay(); 
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print("Level:");
  display.setCursor(70,10);
  display.print(level);
  display.setTextSize(1);
  display.setCursor(80,0);
  display.print("life:");
  display.setTextSize(1);
  display.setCursor(120,0);
  display.print(life);
  display.display();}
  }
void setup(){
	for(int i = 0; i < NUMBER; i++){
		pinMode(switches[i], INPUT);
		digitalWrite(switches[i], HIGH);
		pinMode(leds[i], OUTPUT);
		digitalWrite(leds[i], LOW);
	}
	randomSeed(analogRead(A1));
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(WHITE);
  splash();
  display.clearDisplay(); 
  
}

void reset(){
	free(questions);
	questions = NULL;
	
	answer_num = 0;
	free(answers);
	answers = NULL;
	
	for(int i = 0; i < NUMBER; i++){
		digitalWrite(leds[i], LOW);
	}
}

void playOneTone(int note, float delayScale){
	int noteDuration = 3000 / 8;
    tone(TONE_PIN, note, noteDuration);
	
    int pauseBetweenNotes = noteDuration * delayScale;
    delay(pauseBetweenNotes);

}
void playQuestionsTone(){
	for(int i = 0; i < q_num; i++){
		digitalWrite(leds[questions[i]], HIGH);
		playOneTone(notes[questions[i]], 1.3);
		digitalWrite(leds[questions[i]], LOW);
	}
}

boolean check(){
	for(int i = 0; i < q_num; i++){
		if(questions[i] != answers[i]){
			return false;
		}
	}
	return true;
}

void loop(){
    if(life<=0){
        q_num=3;
        life=3;
       display.clearDisplay();
       display.setTextColor(WHITE);
       display.setTextSize(2);
       display.setCursor(10,10);
       display.print("GAME OVER");
       display.display();
       display.startscrollright(0x00, 0x0F);
       playMelody(MELODY_START);
       delay(1000);
    }
  LevelDisplay();
	switch(state){
		case STATE_START:{
			reset();
			state = STATE_QUESTION;
			break;
		}	
		case STATE_QUESTION:{
			questions = (int *)(malloc(sizeof(int) * q_num));
			answers = (int *)(malloc(sizeof(int) * q_num));
			for(int i = 0; i < q_num; i++){
				questions[i] = random(0, NUMBER);
			}
			answer_num = 0;
			playQuestionsTone();
			lastClickTime = millis();
			state = STATE_ANSWER;
			break;
		}
		
		case STATE_ANSWER:{
			const unsigned long nowTime = millis();
			if(nowTime >= lastClickTime + 10000UL){
				state = STATE_WRONG;
				break;
			}
			
			for(int i = 0; i < NUMBER; i++){
				int ss = digitalRead(switches[i]);
				if(ss == LOW){
					digitalWrite(leds[i], HIGH);
					lastClickTime = nowTime;
					answers[answer_num] = i;
					answer_num++;
					playOneTone(notes[i], 1);
					digitalWrite(leds[i], LOW);
					delay(200);
					break;
				}
				
			}
			
			if(answer_num >= q_num){
				state = check() ? STATE_CORRECT : STATE_WRONG;
			}
			break;
		}
		
		case STATE_CORRECT:{
			q_num++;
       display.clearDisplay();
       display.setTextColor(WHITE);
       display.setTextSize(2);
       display.setCursor(0,10);
       display.print("CORRECT!! ");
       display.display();
			playMelody(MELODY_CORRECT);
			delay(500);
			state = STATE_START;
			break;
		}
		
		case STATE_WRONG:{
       life--;
       display.clearDisplay();
       display.setTextColor(WHITE);
       display.setTextSize(2);
       display.setCursor(0,10);
       display.print("WRONG!! ");
       display.display();
			playMelody(MELODY_WRONG);
			delay(500);
			state = STATE_START;
			break;
		}
		
		default:{
			state = STATE_START;
			break;
		}
	}
}



