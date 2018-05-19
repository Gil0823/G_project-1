#include <avr/pgmspace.h>
#include <avr/interrupt.h> 
#include <avr/io.h>
#include <Servo.h>           
//  ********명령어리스트******** 
#define GAME_START 'a'  // 게임시작 
#define GAME_STOP 'b'  // 게임중지 
#define GAMEMODE_0 'c'  //  조율모드  
#define GAMEMODE_1 'd'  // 솔로플레이   
#define NUM_T 3  // 과녁수
#define FLOW 38  // 도트 매트릭스로 인한 1초 상수 
// ( 특정 변수가 38까지 수가 증가하는 동안 도트매트릭스가 장면을 출력하는데 1초를 유지하고 있음 )

int c4 = 0;  // 도트매트릭스 출력 장면 유지변수

const int __attribute__((progmem)) numbers[][16]={
0x00,0x00,0x00,0x7c0,0xc60,0xc60,0xc60,0xc60,0xc60,0xc60,0xc60,0xc60,0x7c0,0x00,0x00,0x00, //0
0x00,0x00,0x00,0x180,0x1c0,0x180,0x180,0x180,0x180,0x180,0x180,0x180,0x180,0x00,0x00,0x00, //1
0x00,0x00,0x00,0x7c0,0xc60,0xc60,0xc00,0x600,0x300,0x180,0xc0,0x60,0xfe0,0x00,0x00,0x00, //2
0x00,0x00,0x00,0x7c0,0xc60,0xc60,0xc00,0x780,0xc00,0xc00,0xc60,0xc60,0x7c0,0x00,0x00,0x00, //3
0x00,0x00,0x00,0x700,0x700,0x780,0x780,0x6c0,0x6c0,0x660,0xfe0,0x600,0x600,0x00,0x00,0x00, //4
0x00,0x00,0x00,0xfe0,0x60,0x60,0x60,0x7e0,0xc60,0xc00,0xc00,0xc60,0x7c0,0x00,0x00,0x00, //5
0x00,0x00,0x00,0x7c0,0xc60,0x60,0x60,0x7e0,0xc60,0xc60,0xc60,0xc60,0x7c0,0x00,0x00,0x00, //6
0x00,0x00,0x00,0xfe0,0xc00,0xc00,0x600,0x600,0x300,0x300,0x180,0x180,0x180,0x00,0x00,0x00, //7
0x00,0x00,0x00,0x7c0,0xc60,0xc60,0xc60,0x7c0,0xc60,0xc60,0xc60,0xc60,0x7c0,0x00,0x00,0x00, //8
0x00,0x00,0x00,0x7c0,0xc60,0xc60,0xc60,0xc60,0xfc0,0xc00,0xc00,0xc60,0x7c0,0x00,0x00,0x00, //9
0x00,0x00,0x9f8,0x820,0xf20,0x860,0x890,0x908,0x00,0xff0,0x810,0x810,0xff0,0x00,0x00,0x00,  // 점
0x00,0x00,0x380,0x00,0xfe0,0x100,0x280,0x440,0x830,0x100,0x100,0x1ff8,0x00,0x00,0x00,0x00, //초
};
const int __attribute__((progmem)) gameend[][16]={
0x00,0x00,0xa78,0xa40,0xa40,0xa40,0xbc0,0xa40,0xa20,0xa20,0xa10,0xa08,0xa00,0x00,0x00,0x00, // 게
0x00,0x00,0x8f0,0x908,0x908,0x908,0x8f0,0x800,0x00,0xff0,0x810,0x810,0xff0,0x00,0x00,0x00, // 임
0x00,0x00,0xf70,0x840,0x840,0x420,0x1ff8,0x00,0xff0,0x10,0xff0,0x10,0xff0,0x00,0x00,0x00, // 끝
0x00,0x00,0x00,0x00,0x00,0x00,0x270,0x188,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ~
};
const int __attribute__((progmem)) new_rec[][16]={
0x00,0x00,0x9f8,0x900,0x900,0x900,0x900,0x880,0x880,0x840,0x820,0x818,0x800,0x00,0x00,0x00, //기
0x00,0x00,0xff0,0x800,0xff0,0x10,0xff0,0x100,0x1ff8,0x00,0xff0,0x800,0x800,0x00,0x00,0x00, //록
0x00,0x00,0x9f8,0xf00,0x900,0x880,0xf40,0x838,0x00,0x7e0,0x810,0x810,0x7e0,0x00,0x00,0x00, //경
0x00,0x00,0x840,0x840,0x840,0x8a0,0x920,0xa10,0x808,0x800,0x10,0x10,0xff0,0x00,0x00,0x00, //신
};
/////////////////////////////////////////// 도트매트릭스용( 전광판 )
const int A = 2;
const int B = 3;
const int C = 4;
const int D = 5;
const int Data_Red = 6;
const int Data_Green = 7;
const int Clk = 8;
const int LE = 9;
const int OE = 10;
unsigned char flag_cnt = 0; //도트매트릭스에 표시 될 색상 카운팅!
unsigned char move_motion = 0;
unsigned int Move_cnt = 16;
unsigned int Move_cnt2 = 0;
bool flag_Oe = 0;
const int none = 0;  // 숫자 '0' 표기용
const int constant_S = 10;  // 문자 '점' 표기용
const int constant_T = 11;  // 문자 '초' 표기용
/////////////////////////////////////////// 아두이노용
const int SW1 = 11;  // 게임 컨트롤용 버튼 ( 게임모드1 진입용 )
const int SW2 = 12;  // 게임 컨트롤용 버튼 ( 게임중지 진입용 )
const int busser = 13;  // 부저
const int moter_pin[3] = {A0, A1, A2};  // 서보모터 포트 
const int senser_pin[3] = {A3, A4, A5};  // 충격감지센서 포트 
const int score[3] = {10, 30, 50};  // 고유점수 
const int stand_array_Z[3] = {120, 0, 55};  // 과녁 영점계수 배열 ( 다운상태 )
const int stand_array_C[3] = {0, 106, 160};  // 과녁 영점계수 배열 ( 업상태 )
const int timeLimit = 60;  // 제한시간(초)

typedef struct target {  // 과녁 구조체
    Servo moter;
    int stat_stand;  // 기립 여부  
    int score;  // 고유점수  
    int zero;  // 모터위치 바닥 상태 좌표
    int stand_coefficient;  // 모터위치 업 상태 좌표
} Target;

Servo moter[3];
Target target[3];
int record[50] = {0, };  // 점수 기록
int print_buf[4] = {0, 0, 0, 0};  // 전광판 출력문자 지정 버퍼
/* 예를 들어 print_buf가 {1, 3, 4, 6} 이고 출력 문자 배열이 numbers 이면
 *  도트매트릭스에 출력 명령을 내릴시 1346이 표시됨
 */

//////////////////////// ( 게임모드 ) ///////////////////////////////

void gamemode_0(void);  // 조율모드
void gamemode_1(void);  // 솔로플레이

//////////////////////// ( 부가기능 ) ///////////////////////////////

void print_bufetTarget(Target *);  // Target형 객체들의 멤버변수을 초기화 해준다(쓰레기값 출력방지) 
void standup(int, bool);           // 과녁을 세워줌
void standown(int);                // 과녁을 넘어뜨림(작동X)
void busserOn(void);               // 부저울림
void compare(int score);           // 기록들 비교
bool checkAllDead(void);           // 모든 과녁이 쓰러졌는지 확인함(모두 쓰러져있을시 true를 반환한다) 
bool checkTimeOver(void);          // 제한시간이 됐는지 확인함
bool checkStopCommend(void);       // 커맨더에서 게임중지명령 수신여부를 체크함 
char buttonChecking(void);         // 어느버튼이눌렸는지 체크
int decomposeDigits(int);          // 받은 수의 자릿수만을 판별해주는 함수 

//////////////////////// ( 절전기능 ) ///////////////////////////////

void moterdown_All(void);  // 모든 모터 등록 해체
void moterup_All(void);  // 모든 모터 재등록

//////////////////////// ( 도트매트릭스용 함수 ) ////////////////////

void row_dynamic();
void shift_Register(unsigned char out);
void ActivePulse();
void dot_display_shift(int obj[][16], unsigned char first,unsigned char second,unsigned char third,unsigned char fourth);
void printDot_score(int digit, int * print_buf);  // 시간 출력용
void printDot_time(int digit, int * print_buf);  // 점수 출력용

ISR(TIMER2_OVF_vect) {  // 도트매트릭스용 인터럽트 함수
    static bool output = HIGH; 
    static unsigned int cnt=0;
    digitalWrite(1, output);
    output = !output;
  
    if(flag_Oe) {
		digitalWrite(OE, LOW);
    	cnt++;
    	if(cnt>=5) {
    		flag_Oe=0;
    		cnt=0;
    		digitalWrite(OE, HIGH);
		}
    }
	TCNT2=0xe7;
}
 
void setup(void) {
    static int i = 0;  // 해당함수전용 반복문 인덱스  
    
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(C, OUTPUT);
    pinMode(D, OUTPUT);
    pinMode(Data_Red, OUTPUT);
    pinMode(Data_Green, OUTPUT);
    pinMode(Clk, OUTPUT);
    pinMode(LE, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(busser, OUTPUT);
    TCCR2A=0x00;
    TCCR2B=0x04;
    TIMSK2=0x01;
    TCNT2=0xe7;
    SREG=0x80;

    for(i = 0; i < NUM_T; i++) {
        resetTarget(&target[i]);
        target[i].score = score[i];  // 과녁별 고유점수 부가 
        target[i].moter = moter[i];  // 과녁별 서보모터객체 부여 
        target[i].zero = stand_array_Z[i];  // 과녁별 영점조절
        target[i].stand_coefficient = stand_array_C[i];  // 과녁별 영점조절
        target[i].moter.write(target[i].zero);  // 모터 위치 초기화 ( 바닥상태로 )
    }
    for(i = 0; i < NUM_T; i++) {
        pinMode(senser_pin[i], INPUT);  // 충격감지센서 등록 
    }
    for(i = 0; i < 50; i++) {  
        record[i] = 0;  // 쓰레기 값 방지용 배열원소 초기화
    }
}
 
void loop(void) {
    static int i = 0;
    char recieve = buttonChecking(); 
    
    if(recieve) {  // 커맨더 으로부터 명령대기 
        for(int i = 0; i < 4; i++) {
        	print_buf[i] = 0;
        }
        moterup_All();
        switch(recieve) {
            case GAME_START:
                for(i = 0; i < NUM_T; i++) {
                    standup(i, 0);
                }
                break;
            case GAME_STOP:
                for(i = 0; i < NUM_T; i++) {
                    // standown(i);
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
        moterdown_All();  // 대기상태 진입전 모든 모터 절전모드 설정
    }
    delay(100);
}

void gamemode_0(void) {  
    static int i = 0;
    static unsigned int c[3] = {0, };
    static unsigned int p[3] = {0, };
    
    for(i = 0; i < 3; i++) {  // 초기화
    	c[i] = 0;
    	p[i] = 0;
    }
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
          break;
        }
        for(i = 0; i < NUM_T; i++) {
            if(digitalRead(senser_pin[i]) == LOW) {  // 피격여부 확인  
                target[i].stat_stand = 0;  // 해당 인덱스 과녁의 기립상태 갱신
                for(;;) {  // 이 부분 수정 필요
                	c[i] = millis();
                	if(c[i] - p[i] >= 3000) {  // 3초 후에 ( 쿨타임 시스템 )
                	    standup(i, 0);
               		    target[i].stat_stand = 1;  // 해당 인덱스 과녁의 기립상태 갱신
               		    p[i] = c[i]; 
                	    break;
                  	}
               	}
            }
        }
    }
}
 
void gamemode_1(void) {  
    static int i = 0;
    static int score = 0;  // 최종점수 
    static unsigned int c[3] = {0, };
    static unsigned int p[3] = {0, };
    score = 0;
    for(i = 0; i < 3; i++) {  // 초기화
      c[i] = 0;
      p[i] = 0;
    }
// 도트매트릭스 특성상 어느 장면을 출력하여 그 장면을 유지하려면 출력명령을 계속 내려줘야함 그점을 유의하여 코드를 봐주시기 바람.
    for(;;) {
        if(checkTimeOver() == true) {  // 제한시간 다 돼었는지 확인
        	break;
        }
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인
            busserOn();
            busserOn();
            break;
        }
        for(i = 0; i < NUM_T; i++) {
            if(digitalRead(senser_pin[i]) == LOW) {  // 피격여부 확인  
                target[i].stat_stand = 0;  // 해당 인덱스의 기립상태 갱신
                score += target[i].score;
                busserOn();
                c4 += 4;
                for(;;) {
                	c[i] = millis();
                    if(c[i] - p[i] >= 3000) {  // 3초 후에 ( 쿨타임 시스템 )
                    	standup(i, 1);  // 스탠드업
                    	target[i].stat_stand = 1;
                    	p[i] = c[i];
                    	break;
                    }
                    c4++;
                    printDot_time(decomposeDigits(timeLimit - (c4/FLOW)), print_buf);
                }
            }
        }
    }
    digitalWrite(busser, HIGH);
    for(int i = 0; i < 78; i++) {
    	dot_display_shift(gameend, 0, 1, 2, 3);  // 게임종료를 전광판에 출력
    }
    digitalWrite(busser, LOW);
    for(int i = 0; i < 160; i++) {
    	printDot_score(decomposeDigits(score), print_buf);  // 최종점수를 전광판에 출력
    }
    compare(score);  // 기록 등록&기록갱신인지 확인
}
 
void resetTarget(Target * obj) {
    obj->stat_stand = false;  // 초기 기립상태는 다운상태 
    obj->score = 0;
}
 
void standup(int pin, bool s) {
    static int i = 0;
  
    if(s) {  
        for(i = 0; i < 19; i++) {
            target[pin].moter.write(target[pin].stand_coefficient);
            c4++;
            printDot_time(decomposeDigits(timeLimit - (c4/FLOW)), print_buf);
        }
        for(i = 0; i < 19; i++) {
            target[pin].moter.write(target[pin].zero);
            c4++;
            printDot_time(decomposeDigits(timeLimit - (c4/FLOW)), print_buf);
        }
        target[pin].stat_stand = 1;  // 기립상태 갱신       
    }
    else {
        target[pin].moter.write(target[pin].stand_coefficient);
        delay(500);
        target[pin].moter.write(target[pin].zero);
        delay(500);
        target[pin].stat_stand = 1;  // 기립상태 갱신       
    }
}
 
void standown(int pin) {
    /*digitalWrite(busser, HIGH);
    target[pin].moter.write(target[pin].zero);
    target[pin].stat_stand = 0;  // 기립상태 갱신
    digitalWrite(busser, LOW); */
}

void busserOn(void) {
	digitalWrite(busser, HIGH);
	delay(100);  // 이 부분 수정 요망 ( 딜레이로 인하여 인식 씹힘 현상발생 )
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

bool checkTimeOver(void) {
    c4++;
    printDot_time(decomposeDigits(timeLimit - (c4/FLOW)), print_buf);
    if(c4 >= (timeLimit*FLOW)) {
    	c4 = 0;
        return true;
    }
    else {
    	return false;
    }
}
 
bool checkStopCommend(void) {  
    static int i = 0;
    char recieve = buttonChecking();
    
    if(recieve) {  // 커맨더로부터 명령대기 
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

char buttonChecking(void) {
	if(digitalRead(SW1) == HIGH) {  // 게임모드1 진입 명령인가?
    	delay(100);
    	return 'd';
    } 
	else if(digitalRead(SW2) == HIGH) {  // 게임중지 명령인가?
    	delay(100);
    	return 'b';
	}
	else {
    	return 0;
	}
}

// ************************ ( 절전기능 ) ************************

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

// ************************ ( 도트 매트릭스용 함수 ) ************************

void row_dynamic() {
    static unsigned int str_cnt=0;   
    
    switch(str_cnt) { //ROW SHIFT!
		case 0:digitalWrite(A, LOW); digitalWrite(B, LOW); digitalWrite(C, LOW); digitalWrite(D, LOW); break;         //1행 LED
		case 1:digitalWrite(A, HIGH); digitalWrite(B, LOW); digitalWrite(C, LOW); digitalWrite(D, LOW); break;        //2행 LED
		case 2:digitalWrite(A, LOW); digitalWrite(B, HIGH); digitalWrite(C, LOW); digitalWrite(D, LOW); break;        //3행 LED
		case 3:digitalWrite(A, HIGH); digitalWrite(B, HIGH); digitalWrite(C, LOW); digitalWrite(D, LOW); break;       //4행 LED
	    case 4:digitalWrite(A, LOW); digitalWrite(B, LOW); digitalWrite(C, HIGH); digitalWrite(D, LOW); break;        //5행 LED
		case 5:digitalWrite(A, HIGH); digitalWrite(B, LOW); digitalWrite(C, HIGH); digitalWrite(D, LOW); break;       //6행 LED
		case 6:digitalWrite(A, LOW); digitalWrite(B, HIGH); digitalWrite(C, HIGH); digitalWrite(D, LOW); break;       //7행 LED
		case 7:digitalWrite(A, HIGH); digitalWrite(B, HIGH); digitalWrite(C, HIGH); digitalWrite(D, LOW); break;      //8행 LED
		case 8:digitalWrite(A, LOW); digitalWrite(B, LOW); digitalWrite(C, LOW); digitalWrite(D, HIGH); break;        //9행 LED
		case 9:digitalWrite(A, HIGH); digitalWrite(B, LOW); digitalWrite(C, LOW); digitalWrite(D, HIGH); break;       //10행 LED
		case 10:digitalWrite(A, LOW); digitalWrite(B, HIGH); digitalWrite(C, LOW); digitalWrite(D, HIGH); break;      //11행 LED
		case 11:digitalWrite(A, HIGH); digitalWrite(B, HIGH); digitalWrite(C, LOW); digitalWrite(D, HIGH); break;     //12행 LED
		case 12:digitalWrite(A, LOW); digitalWrite(B, LOW); digitalWrite(C, HIGH); digitalWrite(D, HIGH); break;      //13행 LED
		case 13:digitalWrite(A, HIGH); digitalWrite(B, LOW); digitalWrite(C, HIGH); digitalWrite(D, HIGH); break;     //14행 LED
		case 14:digitalWrite(A, LOW); digitalWrite(B, HIGH); digitalWrite(C, HIGH); digitalWrite(D, HIGH); break;     //15행 LED
		case 15:digitalWrite(A, HIGH); digitalWrite(B, HIGH); digitalWrite(C, HIGH); digitalWrite(D, HIGH); break;    //16행 LED
	} 
    str_cnt++; 
    if(str_cnt==16) {
    str_cnt=0;   
    }         
}

void shift_Register(unsigned char out) {
	unsigned char clk=0;          
  
	for(clk=0;clk<8;clk++) { //8비트 데이터를 1비트씩 시프트 레지스터에 입력        
    	if(out&(0x80>>clk)) {
        	switch(flag_cnt){
        		case 0:digitalWrite(Data_Green, LOW);digitalWrite(Data_Red, HIGH);break;    //Red
        		case 1:digitalWrite(Data_Green, LOW);digitalWrite(Data_Red, HIGH);break;    //Red
        		case 2:digitalWrite(Data_Green, LOW);digitalWrite(Data_Red, HIGH);break;    //Red
    		}   
    	} 
    else {
    	digitalWrite(Data_Green, LOW);digitalWrite(Data_Red, LOW);
    }  
    	digitalWrite(Clk, HIGH);
        digitalWrite(Clk, LOW); 
    }  
}

void ActivePulse() {    
	digitalWrite(LE, HIGH); 
	digitalWrite(LE, LOW); //래치 출력
	digitalWrite(OE, LOW);
	flag_Oe=1;
}

void dot_display_shift(int obj[][16], unsigned char first, unsigned char second,unsigned char third,unsigned char fourth) {
	static unsigned int i_cnt=0;    
	unsigned int buff1[16]={0};       //Dot1
	unsigned int buff2[16]={0};       //Dot2
	unsigned int buff3[16]={0};       //Dot3
	unsigned int buff4[16]={0};       //Dot4
            
	unsigned char high1=0;            //Dot1
	unsigned char low1=0;             //Dot1            
	unsigned char high2=0;            //Dot2
	unsigned char low2=0;             //Dot2
	unsigned char high3=0;            //Dot3
	unsigned char low3=0;             //Dot3
	unsigned char high4=0;            //Dot4
	unsigned char low4=0;             //Dot4

	register unsigned int i=0;
  
	for(i_cnt=0;i_cnt<16;i_cnt++){     //우에서 좌로 시프트          
    	if(move_motion==0) {  
        	buff1[i_cnt]=pgm_read_word(&obj[first][i_cnt]);   // 1st String Dot1
        	buff2[i_cnt]=pgm_read_word(&obj[second][i_cnt]);  // 2st String Dot2
        	buff3[i_cnt]=pgm_read_word(&obj[third][i_cnt]);  // 3st String Dot3
        	buff4[i_cnt]=pgm_read_word(&obj[fourth][i_cnt]);  // 4st String Dot4
    	}          
	}   
                
	for(i=0;i<16;i++) {
    	high1=(buff1[i]>>8);
    	low1=(buff1[i]&0xff);                 
    	high2=(buff2[i]>>8);
    	low2=(buff2[i]&0xff);
    	high3=(buff3[i]>>8);
    	low3=(buff3[i]&0xff);
    	high4=(buff4[i]>>8);
    	low4=(buff4[i]&0xff);

    	shift_Register(low1);   
    	shift_Register(high1);
    	shift_Register(low2);   
    	shift_Register(high2); 
    	shift_Register(low3);   
    	shift_Register(high3);        
    	shift_Register(low4);   
    	shift_Register(high4);      
                                                          
    	row_dynamic();
    	ActivePulse();
    }
}

void printDot_score(int digit, int * print_buf) {
	switch(digit) {
    	case 1:
    		dot_display_shift(numbers, none, none, print_buf[3], constant_S);
    		break;
    	case 2:
    		dot_display_shift(numbers, none, print_buf[2], print_buf[3], constant_S);
    		break;
    	case 3:
    		dot_display_shift(numbers, print_buf[1], print_buf[2], print_buf[3], constant_S);
    		break;
    	case 4:
    		dot_display_shift(numbers, print_buf[0], print_buf[1], print_buf[2], print_buf[3]);
    		break;
    	default:
    		;
  	}
}

void printDot_time(int digit, int * print_buf) {
	switch(digit) {
		case 1:
    		dot_display_shift(numbers, none, none, print_buf[3], constant_T);
    		break;
    	case 2:
    		dot_display_shift(numbers, none, print_buf[2], print_buf[3], constant_T);
    		break;
    	case 3:
    		dot_display_shift(numbers, print_buf[1], print_buf[2], print_buf[3], constant_T);
    		break;
    	case 4:
    		dot_display_shift(numbers, print_buf[0], print_buf[1], print_buf[2], print_buf[3]);
    		break;
    	default:
    		;
	}
}

int decomposeDigits(int n) {
    if(n == 0) {
    	return 1;
    }
    int digit = 4;
    int buf[4] = {n/1000, ((n/100)-(buf[0]*10)), ((n/10)-(buf[0]*100)-(buf[1]*10)), (n-(buf[0]*1000)-(buf[1]*100)-(buf[2]*10))};
   
    for(int i = 0; i < 4; i++) {
        if(buf[i] != 0) {
        	digit = digit - i;
        	break;
        }
    }
    for(int i = 0; i < 4; i++) {
    	print_buf[i] = buf[i];
    }
	return digit;
}

void compare(int score) {
	static int i = 0;
  
	record[i] = score;
	for(int j = 0; j < 50; j++) {
		if(record[j] < record[i]) {
    		newRec();
    		break;
    	}
    }
	i++;
}

void newRec(void) {
	unsigned int c = 0;
  
	for(;;) {
		c++;
		if(c >= (FLOW/2)) {
    		break;
    	}
    	else {
    		dot_display_shift(new_rec, 0, 1, 2, 3);
    		busserOn();
    	}
  	}
}