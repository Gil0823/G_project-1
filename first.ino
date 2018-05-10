#include <SoftwareSerial.h>
#include <Servo.h>           //  ********명령어리스트******** 
#define GAME_START 'a'      //   * a : 게임시작             *
#define GAME_STOP 'b'      //    * b : 게임중지             *
#define GAMEMODE_0 'c'    //     * c : 조율모드             *
#define GAMEMODE_1 'd'   //      * d : 솔로플레이           *
#define GAMEMODE_2 'f'  //       * f : 듀오플레이           *
                       //        ****************************
   
const int moter_pin[7] = {A0, A1, A2, 6, 9, 10, 11};  // 서보모터 포트 
const int senser_pin[7] = {2, 3, 4, 5, 7, 8, 12};  // 충격감지센서 포트 
const int score[7] = {10, 10, 30, 30, 50, 50, 100};  // 고유점수 
char recieve;  // 노트북에서 수신한 명령 저장용 변수 
 
typedef struct target {
    Servo moter;
    int stat_stand;  // 기립 여부  
    int score;  // 고유점수  
} Target;
 
Servo moter[7];
Target target[7];
 
void gamemode_0(void);  // 솔로플레이  
void gamemode_1(void);  // 듀오플레이  
void resetTarget(Target *);  // Target형 객체들의 멤버변수을 초기화 해준다(쓰레기값 출력방지) 
void standup(int);  
void standown(int); 
bool checkAllDead(void);  // 모든 과녁이 쓰러졌는지 확인함(모두 쓰러져있을시 true를 반환한다) 
bool checkStopCommend(void);  // 커맨더(UNO3)에서 게임중지명령 수신여부를 체크함 
 
void setup(void) {
    Serial.begin(9600); 
    static int i = 0;  // 해당함수전용 반복문 인덱스  
    for(i = 0; i < 7; i++) {
        resetTarget(&target[i]);
        target[i].score = score[i];  // 과녁별 고유점수 부가 
        target[i].moter = moter[i];  // 과녁별 서보모터객체 부여 
        target[i].moter.attach(moter_pin[i]);  // 서보모터 등록
    }
    for(i = 0; i < 7; i++) {
        pinMode(senser_pin[i], INPUT);  // 충격감지센서 등록 
    }
}
 
void loop(void) {
    static int i = 0;
    
    if(Serial.available() > 0) {  // 노트북 으로부터 명령대기 
        recieve = Serial.read();  // 수신된 명령을 recieve에 저장  
        Serial.println("Recieve!");  // 명령 받앋을시 시리얼모니터에 출력(디버그용으로 작성함) 
        switch(recieve) {
          case GAME_START:
            for(i = 0; i < 7; i++) {
                standup(i);
            }
            break;
          case GAME_STOP:
            for(i = 0; i < 7; i++) {
                standown(i);
                resetTarget(&target[i]);  // 이거 왜 썻지 
            }
            break; 
          case GAMEMODE_0:
            gamemode_0();
            break;
          case GAMEMODE_1:
            gamemode_1();
            break;
          case GAMEMODE_2:
            gamemode_2();
            break;
          default:
          ;  // 빈문장 
        }
    }
    delay(500);
}

unsigned int c = 0;
unsigned int p = 0;

void gamemode_0(void) {  // 조율모드
    p = 0;
    c = 0;
    static int i = 0;
    
    for(;;) {
        c = millis();
        
        if(checkStopCommend() == true) {  // 커맨더(UNO3)로부터 게임중지 명령 왔는지 확인 
          break;
        }
        for(i = 0; i < 1; i++) {
            if(digitalRead(senser_pin[i]) == HIGH) {  // 피격여부 확인  
                standown(i);  // 해당 인덱스의 과녁을 다운시킴 
                target[i].stat_stand = 0;  // 해당 인덱스의 기립상태 갱신 
                Serial.print(target[i].score); Serial.println("scored!"); 
            }
            if(target[i].stat_stand == 0) {  // 피격여부 확인
              if(c-p >= 1000) {  // 1초 후에
                standup(i);  // 스탠드업
                target[i].stat_stand = 1;
                 p = c;
                 Serial.print(i); Serial.println("번째 업"); 
              } 
            }
        }
    }
}
 
void gamemode_1(void) {  // 솔로플레이
    static int i = 0;
    static int score = 0;  // 최종점수 
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더(UNO3)로부터 게임중지 명령 왔는지 확인 
        break;
        }
        if(checkAllDead() == true) {  // 모든 과녁이 다운됐는지 확인 
            for(i = 0; i < 6; i++) {  // 만약 그럴시 모든 과녁을 다 일으킴 
                standup(i);
            }  
        }
        else {
            for(i = 0; i < 6; i++) {
                if(digitalRead(senser_pin[i]) == HIGH) {  // 피격여부 확인
                    Serial.print(target[i].score);
                    Serial.println("점 흭득!");
                    standown(i);
                    score += target[i].score;
                }
            delay(50);
            }
        }
    }
    Serial.write(score);  // 최종점수를 커맨더(UNO3)에 송신함 
    score = 0;  // 다음판을 위해 최종점수 초기화 
}
 
void gamemode_2(void) { // 듀오플레이 ( 솔직히 말하면 솔로플레이랑 아예 같다 ) 
    static int i = 0;
    static int score = 0;
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더(UNO3)로부터 게임중지 명령 왔는지 확인 
        break;
        }
        if(checkAllDead() == true) {  // 모든 과녁이 다운됐는지 확인 
            for(i = 0; i < 5; i++) {  // 만약 그럴시 모든 과녁을 다 일으킴 
                standup(i);
            }
        }
        for(i = 0; i < 6; i++) {
            if(digitalRead(senser_pin[i]) == HIGH) {  // 피격여부 확인
                Serial.print(target[i].score);
                Serial.println("점 흭득!");
                standown(i);
                target[i].stat_stand = 0;
                score += target[i].score;
            }
            delay(20);
        }
    }
    Serial.write(score);  // 최종점수를 커맨더(UNO3)에 송신함 
    score = 0;  // 다음판을 위해 최종점수 초기화
}
 
void resetTarget(Target * obj) {
    obj->stat_stand = false;  // 초기 기립상태는 다운상태 
    obj->score = 0;
}
 
void standup(int pin) {
    target[pin].moter.write(10);
    target[pin].stat_stand = 1;  // 기립상태 갱신  
}
 
void standown(int pin) {
    target[pin].moter.write(90);
    target[pin].stat_stand = 0;  // 기립상태 갱신 
}
 
bool checkAllDead(void) {
    static int i = 0;
    static int count = 0;
  
    for(i = 0; i < 6; i++) {
        if(target[i].stat_stand == 0) {  // 기립상태 확인 
            count++;  // 다운된 과녁수를 계산함 
        }   
    }
    if(count >= 6) {  // 다운된 과녁수가 5개이상인지 확인 ( 오류로 5개가 넘을시를 위해 조건을 이상으로 설정함 ) 
       count = 0;  // 다음 턴을 위해 카운트수를 초기화  
       return true;
    }
    else {
        return false;
    }
}
 
bool checkStopCommend(void) {  
    static int i = 0;
    
  if(Serial.available() > 0) {  // 커맨더(UNO3)로부터 명령대기 
        recieve = Serial.read();  // 수신된 명령을 recieve에 저장
        if(recieve == GAME_STOP) {  // 수신된 명령이 게임중지인지 확인  
            for(i = 0; i < 6; i++) {
                standown(i);  // 모든 과녁을 다운시킴 
            }
        return true;
        }
        else {
            return false;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감 
        return false;
    }
}