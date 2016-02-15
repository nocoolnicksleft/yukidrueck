#include <16F877.h>

#fuses HS,NOWDT,NOPROTECT,PUT,NOBROWNOUT,NOLVP

#include <stdlib.h>

#use delay(clock=20000000)

#use fast_io(A)
#use fast_io(B)
#use fast_io(C)
#use fast_io(D)
#use fast_io(E)

// #use rs232(STREAM=DEBUGGER,PARITY=N,BITS=8,BAUD=2400,XMIT=PIN_B3,RCV=PIN_B3)
#use rs232(stream=mp3player, PARITY=N, baud=9600, xmit=PIN_C6, rcv=PIN_C7, ERRORS)

/*

Illustrationen:

KNOPF 5: Schaf
KNOPF 4: Hund
KNOPF 3: Pferd
KNOPF 2: Kuh
KNOPF 1: Esel


PROGRAMM 1 "FANG DEN FURZ"

Die Knöpfe leuchten reihum. Drückt man einen Knopf wenn er gerade leuchtet gibts zur Belohnung den Furz.

PROGRAMM 2 "BEETHOVEN"

Wenn die Knöpfe gedrückt werden leuchten sie und die Töne C,D,E,F,G werden bis zum Loslassen gespielt.
Das reicht leider nicht ganz für "Alle meine Entchen"

PROGRAMM 3 "SENSO" bzw "SIMON"

Es wird Folge von Knöpfen erleuchtet zu denen jeweils ein Ton gehört. Die Folge ist nachzuspielen und verlängert sich jedesmal.
Wird die Ziellänge (5) fehlerfrei nachgespielt gibts fröhliche Musik. Sonst den Furz. Am Ende muss das Spiel mit einem
neuen Druck auf die 3 neu gestartet werden.

PROGRAMM 4 "MUH"

Jeder Knopf gehört einem Tier. Es werden die vorhandenen Geräusche für jedes Tier reihum abgespielt.
1: Kuh 2: Esel 3: Frosch 4: Pferd 5: Schaf

PROGRAMM 5

Lustige Boing/Krach/Peng Geräusche

PROGRAMM 6

Verschiedene Sorten Klingeln

PROGRAMM 7

5 Yuki Top Hits 1

PROGRAMM 8

5 Yuki Top Hits 2

PROGRAMM 9

5 Yuki Top Hits 3


?

KNOPF "X" schaltet in Sleep-Mode

KNOPF "T" sTartet das Programm von vorne

*/

#define MAX_PROGRAM 12

#define KEYLOCK PIN_E0
#define MP3PLAYERBUSY PIN_B0

#define EEPROM_RANDOM_START 1
#define EEPROM_RUNNING_PROGRAM 5



/**********************************************************
/ Application Global
/**********************************************************/
int8 program_running = 0;

/**********************************************************
/ Randomize
/**********************************************************/
void random_update(int delta)
{
	int random_start;
    random_start = read_eeprom(EEPROM_RANDOM_START);
	random_start += delta;
	srand(random_start);
//	write_eeprom(EEPROM_RANDOM_START,random_start);
}

void random_init()
{
	int random_start;
    random_start = read_eeprom(EEPROM_RANDOM_START);
	srand(random_start);
}

/**********************************************************
/ Common Timer
/**********************************************************/
#define TIMER1_50MSEC_OFF_10MHZ 30000
#define TIMER1_50MSEC_10MHZ 34285
#define TIMER1_100MSEC_10MHZ 3035
#define TIMER1_10MSEC_20MHZ 15536
#define TIMER1_1MSEC_20MHZ 60536
#define TIMER1_3MSEC_20MHZ 50536
#define TIMER1_5MSEC_20MHZ 40536

int1 timeout1msec = 0;
int1 timeout10msec = 0;
int1 timeout100msec = 0;
int  countdown10 = 10;
int  countdown100 = 10;

#INT_TIMER1
void timeproc()
{
   set_timer1(get_timer1() + TIMER1_1MSEC_20MHZ);
   timeout1msec = 1;
   if (countdown10 == 0) {
    timeout10msec = 1;
    countdown10 = 10;
   	if (countdown100 == 0) {
    	timeout100msec = 1;
    	countdown100 = 10;
   	} else countdown100--;
   } else countdown10--;
}

//**********************************************************
// QUEUE HANDLING
//**********************************************************
#define QUEUE_LENGTH 10
char queue[QUEUE_LENGTH];
int8 queue_start = 0;
int8 queue_stop = 0;

int8 get_queue_length()
{
  if (queue_start == queue_stop) return 0;
  if (queue_start < queue_stop) return  (queue_stop - queue_start);
  else return  (QUEUE_LENGTH-queue_start) + queue_stop;
}

void push(char c)
{
// fprintf(terminal,"push %c qlen: %u qstart: %u qstop: %u \r\n",c,get_queue_length(),queue_start,queue_stop);
 queue[queue_stop] = c;
 if (queue_stop == (QUEUE_LENGTH-1)) {
   if (queue_start>0) {
    queue_stop = 0;
   }
 } else {
  if (queue_stop == (queue_start - 1)) {
  } else {
   queue_stop++;
  }
 } 
 // output_high(INT_PIN);
}

char pop()
{
 char c = 0;
 if (queue_start != queue_stop) {
  c = queue[queue_start];
//  fprintf(terminal,"pop %c qstart: %u qstop: %u \r\n",c,queue_start,queue_stop);
  if (queue_start == (QUEUE_LENGTH-1)) queue_start = 0;
  else {
    queue_start++;
  }
 }
 return c;
}

/**********************************************************
/ LEDs
/**********************************************************/
#byte PORTA = 0x5

void button_led_on(int ledid)
{
	PORTA |= (1 << ledid);
}

void button_led_off(int ledid)
{
	PORTA &= ~(1 << ledid);
}

void button_led_clearall()
{
	PORTA = 0;
}

void set_kbd_lamp(int8 no)
{
    // PORTB
    // rrrrcccx
	int8 row;
    int8 col;
    no--;
    row = no / 3;
    col = no - (row * 3);
	output_b(  
			  ( (1 << (row))  << 4) 
			| (  1 << (col+1)) 
			);
}

//**********************************************************
// 4x4 MATRIX KEYBOARD HANDLING
//**********************************************************
#define MBIT_MAKE          0b00010000
#define MBIT_BREAK         0b00100000

#define MBIT_LEFT          0b00000100
#define MBIT_RIGHT         0b00001000

#define MBIT_CODE_MASK     0b00001111

#define MBIT_ID_MASK       0b11000000
#define MBIT_ID_KEYBOARD   0b00000000
#define MBIT_ID_BUTTON     0b01000000
#define MBIT_ID_BUTTONPUSH 0b10000000
#define MBIT_ID_LED 	   0b11000000

#byte KBPORT = 0x8

#define KCOL0 (1 << 1)
#define KCOL1 (1 << 2)
#define KCOL2 (1 << 3)

#define KROW0 (1 << 4)
#define KROW1 (1 << 5)
#define KROW2 (1 << 6)
#define KROW3 (1 << 7)

#define KALL_ROWS (KROW0|KROW1|KROW2|KROW3)
#define KALL_PINS (KALL_ROWS|KCOL0|KCOL1|KCOL2)

#define kset_tris(x) set_tris_d(x)

char const keycodes[16] = {1,4,7,10,2,5,8,11,3,6,9,12};

void dokeyboard()
{

	static int16 allkeys1 = 0xFFFF;
	int16 allkeys = 0;
    int16 deltakeys = 0;
    int16 i;
    int16 test = 0;
	
	kset_tris(KALL_PINS&~KCOL0);
    KBPORT=~KCOL0&KALL_PINS;
    delay_cycles(10);
    allkeys = (int16)((KBPORT & KALL_ROWS) >> 4);

	kset_tris(KALL_PINS&~KCOL1);
    KBPORT=~KCOL1&KALL_PINS;
    delay_cycles(10);
    allkeys |= (int16)((KBPORT & KALL_ROWS) );

	kset_tris(KALL_PINS&~KCOL2);
    KBPORT=~KCOL2&KALL_PINS;
    delay_cycles(10);
    test = ((KBPORT & KALL_ROWS) >> 4);
//    if (test != 0b00001111) button_led_on(4);
    allkeys |= (int16)(test << 8);
  
    if (allkeys != allkeys1) {
     deltakeys = (allkeys ^ allkeys1);
     for (i=0;i<12;i++) {
      if (deltakeys & ( 1 << i)) {
        if (allkeys & ( 1 << i)) push(MBIT_ID_KEYBOARD | MBIT_BREAK | keycodes[i]);
        else push(MBIT_ID_KEYBOARD | MBIT_MAKE | keycodes[i]);
      }
     }

    // fprintf(DEBUGGER,"kstat: %X %X\r\n",*(&allkeys+1),allkeys);

     allkeys1 = allkeys;
    }
}

//**********************************************************
// 1x5 STATIC KEYBOARD HANDLING (BIG BUTTONS)
//**********************************************************
void dobuttonpush()
{
  static int8 keystate1 = 0xFF;
  int8 keystate;
  int8 keydelta;
  int8 i;

  keystate = input_c();
  if (keystate != keystate1) 
  {
    keydelta = (keystate ^ keystate1);
    for (i=0;i<6;i++) {
     if (keydelta & ( 1 << i)) {
       if (keystate & (1 << i)) push (MBIT_ID_BUTTONPUSH | MBIT_BREAK | i );
       else push (MBIT_ID_BUTTONPUSH | MBIT_MAKE | i );
     }
    }    
    keystate1 = keystate;
  }
  
}


/**********************************************************
/ Serial Input Buffer
/**********************************************************/
#define MP3_REPORT_BUFFER_LENGTH 32
#define MP3_COLUMN_LENGTH 20
BYTE mp3buffer[MP3_REPORT_BUFFER_LENGTH];
int8 mp3bufferptr = 0;
int8 mp3status = 0;
int8 mp3completed = 0;
int1 mp3received = 0;

   
#INT_RDA
void serial_isr() {
   BYTE c;
   c = fgetc(mp3player);
   if (!mp3received) {
    if (c == '>')  {
     mp3buffer[mp3bufferptr] = 0;
     mp3received = 1;
     mp3bufferptr = 0;
    } else {
     mp3buffer[mp3bufferptr] = c;
     if (mp3bufferptr < MP3_REPORT_BUFFER_LENGTH) mp3bufferptr++;
    }
   }
}
/**********************************************************
/ MP3 Control
/**********************************************************/

int8 cfg_mp3volume = 60;

int1 mp3playing = 0;
int8 mp3timeout = 0;
int8 mp3retries = 0;

char mp3title[10];

#define MP3_STATUS_READY 0
#define MP3_CMD_QUERY_STATUS 1
#define MP3_CMD_SET_VOLUME 2
#define MP3_CMD_PLAY 3
#define MP3_CMD_STOP 4
#define MP3_STATUS_INIT_0 5
#define MP3_STATUS_INIT_1 6
#define MP3_STATUS_INIT_2 7
#define MP3_STATUS_INIT_3 8
#define MP3_CMD_OPEN_TITLEFILE 9
#define MP3_CMD_QUERY_TITLE 10
#define MP3_CMD_QUERY_ARTIST 11
#define MP3_CMD_OPEN_COUNTERFILE 12
#define MP3_CMD_QUERY_COUNTERS 13
#define MP3_CMD_CLOSE_TITLEFILE 14
#define MP3_CMD_CLOSE_COUNTERFILE 15
#define MP3_CMD_QUERY_LENGTH 16
#define MP3_CMD_SET_BASS_ENHANCE 17
#define MP3_CMD_SET_SERIAL_SPEED 18
#define MP3_CMD_LOAD_MACRO 19
#define MP3_CMD_ENABLE_BUSY_IND 20
#define MP3_CMD_LOOP_ON 21
#define MP3_CMD_LOOP_OFF 22

#define MP3_DEFAULT_TIMEOUT 100

#define MP3_VOLUME_MINIMUM 120
#define MP3_VOLUME_MAXIMUM 50
#define MP3_VOLUME_DEFAULT 90
/*
int8 mp3initsequencecount = 8;
int8 mp3initsequence[8] =[MP3_CMD_SET_SERIAL_SPEED,
                      MP3_CMD_SET_BASS_ENHANCE,
                      MP3_CMD_SET_VOLUME,
                      MP3_CMD_CLOSE_TITLEFILE,
                      MP3_CMD_CLOSE_COUNTERFILE,
                      MP3_CMD_OPEN_TITLEFILE,
                      MP3_CMD_OPEN_COUNTERFILE,
                      MP3_CMD_QUERY_COUNTERS];
*/


void mp3sendcommand()
{
 // button_led_off(5);
  if      (mp3status ==  MP3_CMD_QUERY_STATUS) fprintf(mp3player,"PC Z\r");
  else if (mp3status ==  MP3_CMD_PLAY) {
	mp3playing = 1;
	fprintf(mp3player,"PC F /0/%s.mp3\r",mp3title);
  }
  else if (mp3status ==  MP3_CMD_STOP) fprintf(mp3player,"PC S\r");
  else if (mp3status ==  MP3_CMD_SET_VOLUME) fprintf(mp3player,"ST V %u\r",cfg_mp3volume);
  else if (mp3status ==  MP3_CMD_SET_BASS_ENHANCE) fprintf(mp3player,"ST B %u\r",7);
  else if (mp3status ==  MP3_CMD_SET_SERIAL_SPEED) fprintf(mp3player,"ST D 2\r");
  else if (mp3status ==  MP3_CMD_ENABLE_BUSY_IND) fprintf(mp3player,"ST H 1\r");
  else if (mp3status ==  MP3_CMD_LOOP_ON) fprintf(mp3player,"ST O 0\r");
  else if (mp3status ==  MP3_CMD_LOOP_OFF) fprintf(mp3player,"ST O 1\r");
  mp3timeout = MP3_DEFAULT_TIMEOUT;
}


void mp3docommand(int8 commandid)
{
 if (!mp3status) {
  mp3status = commandid;
  mp3retries = 5;
  mp3sendcommand();
 }
}
#define MP3_QUEUE_LENGTH 30

int mp3command_queue[MP3_QUEUE_LENGTH];
int mp3command_queue_start = 0;
int mp3command_queue_stop = 0;
int mp3command_last;

unsigned short int mp3get_queue_length()
{
  if (mp3command_queue_start == mp3command_queue_stop) return 0;
  if (mp3command_queue_start < mp3command_queue_stop) return  (mp3command_queue_stop - mp3command_queue_start);
  else return  (MP3_QUEUE_LENGTH-mp3command_queue_start) + mp3command_queue_stop;
}

void mp3clearqueue()
{
 mp3command_queue_start = 0;
 mp3command_queue_stop = 0;
 mp3retries = 1;
 mp3timeout = 1;
}


void mp3commandpush(char c)
{
#ifdef DEBUG_MAIN
 //debug_printf("push %c qlen: %u qstart: %u qstop: %u \r\n",c,get_queue_length(),mp3command_queue_start,mp3command_queue_stop);
#endif
 
 if (c != mp3command_last) {
 mp3command_queue[mp3command_queue_stop] = c;
 if (mp3command_queue_stop == (MP3_QUEUE_LENGTH-1)) {
   if (mp3command_queue_start>0) {
    mp3command_queue_stop = 0;
   }
 } else {
  if (mp3command_queue_stop == (mp3command_queue_start - 1)) {
  } else {
   mp3command_queue_stop++;
  }
 } 
 mp3command_last = c;
 }
 
}

char mp3commandpop()
{
 char c = 0;
 if (mp3command_queue_start != mp3command_queue_stop) {
  c = mp3command_queue[mp3command_queue_start];
  if (mp3command_queue_start == (MP3_QUEUE_LENGTH-1)) mp3command_queue_start = 0;
  else {
    mp3command_queue_start++;
  }
 }
 if (mp3command_queue_start == mp3command_queue_stop) mp3command_last = 0;
 return c;
}

void mp3execute()
{
 if (mp3get_queue_length() && (!mp3status)  && (!mp3received)) {
  mp3status = mp3commandpop();
  mp3retries = 3;
  mp3sendcommand();
 }
}

int1 mp3playtrack(char *name)
{
  if (mp3playing) {
  	mp3commandpush(MP3_CMD_STOP);
  }
  //if (!mp3playing) {
  	strcpy(mp3title,name);
  	mp3commandpush(MP3_CMD_PLAY);
  	return 1;
  //}
 //return 0;
}


/**********************************************************
/ SAMPLES
/**********************************************************/


/**********************************************************
/ SUBPROGRAMS
/**********************************************************/
char manc1track[] = "manc1";
char fart1track[] = "fart1";

char ton_c_track[] = "ton_c";
char ton_d_track[] = "ton_d";
char ton_e_track[] = "ton_e";
char ton_f_track[] = "ton_f";
char ton_g_track[] = "ton_g";


char mootrack[] = "moo1";
char horsetrack[] = "horse1";
// char frogtrack[] = "frog1";
char donkeytrack[] = "donkey1";
char sheeptrack[] = "sheep1";
char dogtrack[] = "dog1";


char boing1track[] = "boing1";
char boing2track[] = "boing2";
char boing3track[] = "boing3";
char boing4track[] = "boing4";
char boing5track[] = "boing5";

char bell1track[] = "bell1";
char bell2track[] = "bell2";
char bell3track[] = "bell3";
char bell4track[] = "bell4";
char bell5track[] = "bell5";


// shared data storage
int programState;
int programStep;
int1 programSignal1;
int1 programSignal2;
int programWaiting = 0;


// included program definitions
#include "program1.c"
#include "program2.c"
#include "program3.c"
#include "program4.c"
#include "program5.c"
#include "program6.c"
#include "program7.c"

// shared entry methods
void program_keydown(int key)
{
	if (program_running == 1) program1_keydown(key);
	else if (program_running == 2) program2_keydown(key);
	else if (program_running == 3) program3_keydown(key);
	else if (program_running == 4) program4_keydown(key);
	else if (program_running == 5) program5_keydown(key);
	else if (program_running == 6) program6_keydown(key);
	else if (program_running == 7) program7_keydown(key);
	else if (program_running == 8) program7_keydown(key);
	else if (program_running == 9) program7_keydown(key);
	else if (program_running == 10) program7_keydown(key);
	else if (program_running == 11) program7_keydown(key);
	else if (program_running == 12) program7_keydown(key);

}

void program_timer100msec()
{
	if (program_running == 1) program1_timer100msec();
	else if (program_running == 2) program2_timer100msec();
	else if (program_running == 3) program3_timer100msec();
	else if (program_running == 4) program4_timer100msec();
	else if (program_running == 5) program5_timer100msec();
	else if (program_running == 6) program6_timer100msec();
	else if (program_running == 7) program7_timer100msec();
	else if (program_running == 8) program7_timer100msec();
	else if (program_running == 9) program7_timer100msec();
	else if (program_running == 10) program7_timer100msec();
	else if (program_running == 11) program7_timer100msec();
	else if (program_running == 12) program7_timer100msec();
}

void program_keyup(int key)
{
	if (program_running == 1) program1_keyup(key);
	else if (program_running == 2) program2_keyup(key);
	else if (program_running == 3) program3_keyup(key);
	else if (program_running == 4) program4_keyup(key);
	else if (program_running == 5) program5_keyup(key);
	else if (program_running == 6) program6_keyup(key);
	else if (program_running == 7) program7_keyup(key);
	else if (program_running == 8) program7_keyup(key);
	else if (program_running == 9) program7_keyup(key);
	else if (program_running == 10) program7_keyup(key);
	else if (program_running == 11) program7_keyup(key);
	else if (program_running == 12) program7_keyup(key);
}

void program_start(int programId)
{
	random_update(programId);

	if (programId <= MAX_PROGRAM)
	{
		if (program_running == 1) program1_stop();
		else if (program_running == 2) program2_stop();
		else if (program_running == 3) program3_stop();
		else if (program_running == 4) program4_stop();
		else if (program_running == 5) program5_stop();
		else if (program_running == 6) program6_stop();
		else if (program_running == 7) program7_stop();
		else if (program_running == 8) program7_stop();
		else if (program_running == 9) program7_stop();
		else if (program_running == 10) program7_stop();
		else if (program_running == 11) program7_stop();
		else if (program_running == 12) program7_stop();

		if (program_running != programId)
		{
			program_running = programId;

			write_eeprom(EEPROM_RUNNING_PROGRAM,program_running);
		}

		set_kbd_lamp(program_running);

		programState = 0;
		programStep = 0;
		programSignal1 = 0;
		programSignal2 = 0;

		button_led_clearall();

		if (program_running == 1) program1_start();
		else if (program_running == 2) program2_start();
		else if (program_running == 3) program3_start();
		else if (program_running == 4) program4_start();
		else if (program_running == 5) program5_start();
		else if (program_running == 6) program6_start();
		else if (program_running == 7) program7_start();
		else if (program_running == 8) program7_start();
		else if (program_running == 9) program7_start();
		else if (program_running == 10) program7_start();
		else if (program_running == 11) program7_start();
		else if (program_running == 12) program7_start();

 	}
}

void program_sample_started(int sample)
{
	if (program_running == 1) program1_sample_started(sample);
	else if (program_running == 2) program2_sample_started(sample);
	else if (program_running == 3) program3_sample_started(sample);
	else if (program_running == 4) program4_sample_started(sample);
	else if (program_running == 5) program5_sample_started(sample);
	else if (program_running == 6) program6_sample_started(sample);
	else if (program_running == 7) program7_sample_started(sample);
	else if (program_running == 8) program7_sample_started(sample);
	else if (program_running == 9) program7_sample_started(sample);
	else if (program_running == 10) program7_sample_started(sample);
	else if (program_running == 11) program7_sample_started(sample);
	else if (program_running == 12) program7_sample_started(sample);
}

void program_sample_completed(int sample)
{
	if (program_running == 1) program1_sample_completed(sample);
	else if (program_running == 2) program2_sample_completed(sample);
	else if (program_running == 3) program3_sample_completed(sample);
	else if (program_running == 4) program4_sample_completed(sample);
	else if (program_running == 5) program5_sample_completed(sample);
	else if (program_running == 6) program6_sample_completed(sample);
	else if (program_running == 7) program7_sample_completed(sample);
	else if (program_running == 8) program7_sample_completed(sample);
	else if (program_running == 9) program7_sample_completed(sample);
	else if (program_running == 10) program7_sample_completed(sample);
	else if (program_running == 11) program7_sample_completed(sample);
	else if (program_running == 12) program7_sample_completed(sample);
}

//**********************************************************
// MP3-PLAYER SERIAL COMMUNICATIONS
//**********************************************************

void doserial()
{
    if (mp3received) {
  //   fprintf(terminal,"R:%S!",mp3buffer); // echo to terminal
      mp3received = 0;

      if (mp3buffer[0] == 'E') { // fehlermeldung!!
		button_led_on(5);
        mp3completed = 0;

      } else {

       if (mp3status == MP3_CMD_PLAY) {
         //mp3playing = 1;
       }

       if (mp3status == MP3_CMD_STOP) {
		if (mp3playing) {
        // mp3playing = 0;
		// program_sample_completed(0);
	    }
       }

       if (mp3status == MP3_CMD_QUERY_LENGTH) {
       }

       if (mp3status == MP3_CMD_QUERY_STATUS) {
        if (mp3buffer[0] == 'S') {
         mp3playing = 0;
        } // if (mp3buffer[0] == 'S')
       } // if (mp3status == MP3_STATUS_QUERIED)


      } // if (mp3buffer[0] == 'E') / else

      mp3completed = mp3status;  // status für folgeaktionen aufbewahren
      mp3status = MP3_STATUS_READY; // neues kommando ist zulässig

      if (program_running == 1) program1_mp3commandfinished(mp3completed);
      else if (program_running == 2) program2_mp3commandfinished(mp3completed);
      else if (program_running == 3) program3_mp3commandfinished(mp3completed);
      else if (program_running == 4) program4_mp3commandfinished(mp3completed);
      else if (program_running == 5) program5_mp3commandfinished(mp3completed);
      else if (program_running == 6) program6_mp3commandfinished(mp3completed);
     
    } // if (bkbhit) 
 }


/**********************************************************
/ KEYBOARD
/**********************************************************/

void dokeyaction(int key)
{
   if ((key & MBIT_ID_MASK) == MBIT_ID_BUTTONPUSH)
   {
   	   if (key & MBIT_MAKE)
       {
		program_keydown(key & MBIT_CODE_MASK);
       } else {
		program_keyup(key & MBIT_CODE_MASK);
       }
   } 

   else 

   if ((key & MBIT_ID_MASK) == MBIT_ID_KEYBOARD)
   {
   	   if (key & MBIT_MAKE)
       {
        key = (key & MBIT_CODE_MASK);

        if (key <= MAX_PROGRAM)
        {
           if (key != program_running)
           {
              program_start(key);
           }
        } else {
			// if (key == 11) program_start(program_running);
		}
       } else {

       }
   }

}

int1 busy_went_down = 0;
int1 busy_went_up = 0;

#INT_EXT
void isr_intext()
{
	if ((input_b() & 0x1)) {
	 busy_went_up = 1;
	 ext_int_edge(H_TO_L);  
	} else {
	 busy_went_down = 1;
	 ext_int_edge(L_TO_H);  
	}  
}


/**********************************************************
/ MAIN
/**********************************************************/
void main()
{

   //int8 y = 0;
   
   setup_adc(ADC_OFF);
   setup_adc_ports(NO_ANALOGS);

   output_a(0x00);
   output_b(0x00);
   output_c(0x00);
   output_d(0x00);
   output_e(0x00);

   set_tris_a(0b00000000); // button leds
   set_tris_b(0b00000001); // keyboard leds, mp3 busy (B0)
   set_tris_c(0b10111110); // kommunikation und buttons
   set_tris_e(0b00000111); // E0: Keboard Lock

   ext_int_edge(L_TO_H);    

   setup_timer_1(T1_INTERNAL | T1_DIV_BY_1 );    // Start timer 1
   enable_interrupts(INT_TIMER1);
   enable_interrupts(INT_RDA);
   enable_interrupts(INT_EXT);
   enable_interrupts(global);

   button_led_clearall();

   set_kbd_lamp(12);
   delay_ms(300); 
   set_kbd_lamp(11);
   delay_ms(300); 
   set_kbd_lamp(10);
   delay_ms(300); 
   set_kbd_lamp(9);
   delay_ms(300); 
   set_kbd_lamp(8);
   delay_ms(300); 
   set_kbd_lamp(7);
   delay_ms(300); 
   set_kbd_lamp(6);
   delay_ms(300); 
   set_kbd_lamp(5);
   delay_ms(300); 
   set_kbd_lamp(4);
   delay_ms(300); 
   set_kbd_lamp(3);
   delay_ms(300); 
   set_kbd_lamp(2);
   delay_ms(300); 
   set_kbd_lamp(1);
   delay_ms(300); 

  // set_uart_speed(9600,mp3player);

  // mp3commandpush(MP3_CMD_SET_SERIAL_SPEED);
  
   set_uart_speed(38400,mp3player);
  
   mp3commandpush(MP3_CMD_STOP);
   mp3commandpush(MP3_CMD_SET_VOLUME);
   mp3commandpush(MP3_CMD_SET_BASS_ENHANCE);
   mp3commandpush(MP3_CMD_ENABLE_BUSY_IND);
   mp3commandpush(MP3_CMD_LOOP_OFF);
  
   random_init();

   program_running = read_eeprom(EEPROM_RUNNING_PROGRAM);

   delay_ms(100);  // wait for things to calm


   program_start(program_running);
  // program_start(4);

   for (;;) {

    mp3execute();

	if (busy_went_down)
	{
		busy_went_down = 0;
		mp3playing = 0;
		program_sample_completed(0);
	}

	if (busy_went_up)
	{
		busy_went_up = 0;
		program_sample_started(0);
	}



    if (mp3completed)
 	{
    	mp3completed = 0;
	}

   if (queue_start != queue_stop) {
     dokeyaction(queue[queue_start]);
     pop();
   }

   if (timeout1msec) {
     timeout1msec = 0;
     dobuttonpush();

	if (input_state(KEYLOCK) == 0) dokeyboard();
   }

   if (timeout10msec) {
	 timeout10msec = 0;
   }

   if (timeout100msec) {
	 timeout100msec = 0;
	 program_timer100msec();

     if (mp3status) {
      if (--mp3timeout == 0) {
//       fprintf(terminal,"T!");
       button_led_on(5);
       mp3completed = 0;
       mp3bufferptr = 0;
       if (--mp3retries) mp3sendcommand();
       else mp3status = MP3_STATUS_READY;
      }
     }

   }

   doserial();

 }

}
