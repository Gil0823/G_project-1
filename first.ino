#include <SoftwareSerial.h>
#include <Servo.h>           //  ********명령어리스트******** 
#define GAME_START 'a'      //   * a : 게임시작             *
#define GAME_STOP 'b'      //    * b : 게임중지             *
#define GAMEMODE_0 'c'    //     * c : 조율모드             *
#define GAMEMODE_1 'd'   //      * d : 솔로플레이           *      
#define NUM_T 7         //       *****************************
   
const int moter_pin[7] = {A0, A1, A2, 6, 9, 10, 11};  // 서보모터 포트 
const int senser_pin[7] = {2, 3, 4, 5, 7, 8, 12};  // 충격감지센서 포트 
const int score[7] = {10, 10, 30, 30, 50, 50, 100};  // 고유점수 
const int busser = 13;  // 부저
char recieve;  // 커맨더에서 수신한 명령 저장용 변수 
 
typedef struct target {
    Servo moter;
    int stat_stand;  // 기립 여부  
    int score;  // 고유점수  
} Target;
 
Servo moter[7];
Target target[7];
 
void gamemode_0(void);  // 솔로플레이  
//void gamemode_1(void);  // 듀오플레이  
void resetTarget(Target *);  // Target형 객체들의 멤버변수을 초기화 해준다(쓰레기값 출력방지) 
void standup(int);  
void standown(int); 
bool checkAllDead(void);  // 모든 과녁이 쓰러졌는지 확인함(모두 쓰러져있을시 true를 반환한다) 
bool checkStopCommend(void);  // 커맨더에서 게임중지명령 수신여부를 체크함 
void moterdown_All(void);  // 모든 모터 등록 해체
void moterup_All(void);  // 모든 모터 재등록
 
void setup(void) {
    Serial.begin(9600); 
    static int i = 0;  // 해당함수전용 반복문 인덱스  
    pinMode(busser, OUTPUT);
    
    for(i = 0; i < NUM_T; i++) {
        resetTarget(&target[i]);
        target[i].score = score[i];  // 과녁별 고유점수 부가 
        target[i].moter = moter[i];  // 과녁별 서보모터객체 부여 
    }
    for(i = 0; i < NUM_T; i++) {
        pinMode(senser_pin[i], INPUT);  // 충격감지센서 등록 
    }
}
 
void loop(void) {
    static int i = 0;
    
    if(Serial.available() > 0) {  // 커맨더 으로부터 명령대기 
        recieve = Serial.read();  // 수신된 명령을 recieve에 저장  
        Serial.println("Recieve!");  // 명령 받앋을시 시리얼모니터에 출력(디버그용으로 작성함) 
        moterup_All();
        delay(1000);
        
        switch(recieve) {
            case GAME_START:
                for(i = 0; i < NUM_T; i++) {
                    standup(i);
                    
                }
                break;
            case GAME_STOP:
                for(i = 0; i < NUM_T; i++) {
                    standown(i);
                }
                break; 
           case GAMEMODE_0:
               gamemode_0();
               break;
           case GAMEMODE_1:
               gamemode_1();
               break;
           default:
               ;  // 빈문장 
        }
        moterdown_All();
    }
    delay(100);
}

void gamemode_0(void) {  // 조율모드
    static int i = 0;
    static unsigned int c = 0;
    static unsigned int p = 0;
    c = 0;
    p = 0;
    
    for(;;) {
        c = millis();
        
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
          break;
        }
        for(i = 0; i < NUM_T; i++) {
            if(digitalRead(senser_pin[i]) == HIGH) {  // 피격여부 확인  
                standown(i);  // 해당 인덱스의 과녁을 다운시킴 
                target[i].stat_stand = 0;  // 해당 인덱스의 기립상태 갱신
                Serial.print(i); Serial.println("번째");  
                Serial.print(target[i].score); Serial.println("점 흭득!"); 
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
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
            break;
        }
        if(checkAllDead() == true) {  // 모든 과녁이 다운됐는지 확인 
            for(i = 0; i < NUM_T; i++) {  // 만약 그럴시 모든 과녁을 다 일으킴 
                standup(i);
            }  
        }
        else {
            for(i = 0; i < NUM_T; i++) {
                if(digitalRead(senser_pin[i]) == HIGH) {  // 피격여부 확인
                    Serial.print(target[i].score); Serial.println("점 흭득!");
                    standown(i);
                    score += target[i].score;
                }
            delay(50);  // 확인요망
            }
        }
    }
    Serial.println(score);  // 최종점수를 모니터에 출력(이부분은 세그먼트로 바꿀예정) 
    score = 0;  // 다음판을 위해 최종점수 초기화 
}
 
/*void gamemode_2(void) { // 듀오플레이 ( 솔직히 말하면 솔로플레이랑 아예 같다 ) 
    static int i = 0;
    static int score = 0;
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
        break;
        }
        if(checkAllDead() == true) {  // 모든 과녁이 다운됐는지 확인 
            for(i = 0; i < NUM_T; i++) {  // 만약 그럴시 모든 과녁을 다 일으킴 
                standup(i);
            }
        }
        for(i = 0; i < NUM_T; i++) {
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
    Serial.write(score);  // 최종점수를 커맨더에 송신함 
    score = 0;  // 다음판을 위해 최종점수 초기화
}*/
 
void resetTarget(Target * obj) {
    obj->stat_stand = false;  // 초기 기립상태는 다운상태 
    obj->score = 0;
}
 
void standup(int pin) {
    Serial.println("업업업");
    target[pin].moter.write(10);
    target[pin].stat_stand = 1;  // 기립상태 갱신  
}
 
void standown(int pin) {
    digitalWrite(busser, HIGH);
    target[pin].moter.write(90);
    target[pin].stat_stand = 0;  // 기립상태 갱신
    digitalWrite(busser, LOW); 
}
 
bool checkAllDead(void) {
    static int i = 0;
    static int count = 0;
  
    for(i = 0; i < NUM_T; i++) {
        if(target[i].stat_stand == 0) {  // 기립상태 확인 
            count++;  // 다운된 과녁수를 계산함 
        }   
    }
    if(count >= NUM_T) {  // 다운된 과녁수가 NUM_T개이상인지 확인 ( 오류로 NUM_T개가 넘을시를 위해 조건을 이상으로 설정함 ) 
       count = 0;  // 다음 턴을 위해 카운트수를 초기화  
       return true;
    }
    else {
        return false;
    }
}
 
bool checkStopCommend(void) {  
    static int i = 0;
    
    if(Serial.available() > 0) {  // 커맨더로부터 명령대기 
        recieve = Serial.read();  // 수신된 명령을 recieve에 저장
        if(recieve == GAME_STOP) {  // 수신된 명령이 게임중지인지 확인  
            for(i = 0; i < NUM_T; i++) {
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

void moterdown_All(void) {
    static int i = 0;
  
    for(i = 0; i < NUM_T; i++) {
        target[i].moter.detach();
    }
}

void moterup_All(void) {
    static int i = 0;
  
    for(i = 0; i < NUM_T; i++) {
        target[i].moter.attach(moter_pin[i]);  // 서보모터 등록
    }
}

