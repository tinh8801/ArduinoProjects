#define SW_CENTER_PIN 14
#define LOOP_INTERVAL_MS 20

#define SW_ON  LOW
#define SW_OFF HIGH


#define STATE_1   1
#define STATE_2   2
#define STATE_3   3
#define STATE_4   4
#define STATE_5   5
#define STATE_6   6
#define STATE_7   7
#define STATE_8   8
#define STATE_9   9
#define STATE_DEFAULT   10

#define FORM_1  1
#define FORM_2  2
#define FORM_3 3
#define FORM_4   4
#define FORM_5  5
#define FORM_6  6
#define FORM_7  7
#define FORM_8  8
#define FORM_9  9
#define FORM_DEFAULT 10



#define WAIT_MS_LONG_PRESS 1000
const uint16_t WAIT_COUNT_LONG_PRESS = WAIT_MS_LONG_PRESS / LOOP_INTERVAL_MS;

uint8_t state = STATE_DEFAULT;
uint8_t prev_state = STATE_DEFAULT;
uint8_t before_state = STATE_DEFAULT;
uint8_t form = FORM_DEFAULT;
uint8_t prev_form = FORM_DEFAULT;
uint8_t before_form = FORM_DEFAULT;

uint8_t sw_center = SW_OFF;
uint8_t prev_sw_center = SW_OFF;

uint16_t sw_center_press_counter = 0;
boolean  is_valid_sw_center_long_press = false;

void update_state(uint8_t new_state){
  before_state = state;
  state = new_state;
}

void update_form(uint8_t new_form){
  before_form = form;
  form = new_form;
  
}

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define SOUND_VOLUME_DEFAULT 20
#define SOUND_VOLUME_LARGE   30


#define SOUND_MODE_1                   1
#define SOUND_MODE_2                 2
#define SOUND_MODE_3       3
#define SOUND_MODE_4        4
#define SOUND_MODE_5        5
#define SOUND_MODE_6           6
#define SOUND_MODE_7 7
#define SOUND_MODE_8 8
#define SOUND_MODE_9 9
#define SOUND_MODE_10 10
#define SOUND_THEME_1 11
#define SOUND_THEME_2 12
#define SOUND_THEME_3 13
#define SOUND_THEME_4 14
#define SOUND_THEME_5 15
#define SOUND_THEME_6 16
#define SOUND_THEME_7 17
#define SOUND_THEME_8 18
#define SOUND_THEME_9 19
#define SOUND_THEME_10 20
#define SOUND_DEFAULT 23 //system starting




uint8_t volume = SOUND_VOLUME_DEFAULT;

SoftwareSerial ss_mp3_player(4, 5); // RX, TX
DFRobotDFPlayerMini mp3_player;

void play_sound(uint8_t sound_num){
  //mp3_player.pause();
  mp3_player.playMp3Folder(sound_num);
}

void loop_sound(uint8_t sound_num){
  //mp3_player.pause();
  mp3_player.loop(sound_num);
}

void pause_sound(){
  mp3_player.pause();
}

void update_volume(uint8_t new_volume){
  mp3_player.volume(new_volume);
  volume = new_volume;
}

void control_sound(){
  if(prev_state != state){ // 基本的に状態遷移したときのみ音声処理
    switch(state){
    case STATE_1:
     play_sound(SOUND_MODE_1);
     break;

    case STATE_2:
     play_sound(SOUND_MODE_2);
     break;

    case STATE_3:
     play_sound(SOUND_MODE_3);
     break;

    case STATE_4:
     play_sound(SOUND_MODE_4);
     break;

    case STATE_5:
     play_sound(SOUND_MODE_5);
     break;

    case STATE_6:
     play_sound(SOUND_MODE_6);
     break;

    case STATE_7:
     play_sound(SOUND_MODE_7);
     break;

    case STATE_8:
     play_sound(SOUND_MODE_8);
     break;

    case STATE_9:
     play_sound(SOUND_MODE_9);
     break;

    case STATE_DEFAULT:
     play_sound(SOUND_MODE_10);
     break;

    }
    }
    if(prev_form!=form){ //
    switch(form){
      case FORM_1:
      loop_sound(SOUND_THEME_1);
      break;

      case FORM_2:
      loop_sound(SOUND_THEME_2);
      break;

      case FORM_3:
      loop_sound(SOUND_THEME_3);
      break;

      case FORM_4:
      loop_sound(SOUND_THEME_4);
      break;

      case FORM_5:
      loop_sound(SOUND_THEME_5);
      break;

      case FORM_6:
      loop_sound(SOUND_THEME_6);
      break;

      case FORM_7:
      loop_sound(SOUND_THEME_7);
      break;

      case FORM_8:
      loop_sound(SOUND_THEME_8);
      break;

      case FORM_9:
      loop_sound(SOUND_THEME_9);
      break;

      case FORM_DEFAULT:
      loop_sound(SOUND_THEME_10);
      break;
    }
    }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SW_CENTER_PIN, INPUT_PULLUP);
  ss_mp3_player.begin(9600);
  if(!mp3_player.begin(ss_mp3_player)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin music_player:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("mp3_player online."));
  mp3_player.setTimeOut(500); //Set serial communictaion time out 500ms
  update_volume(SOUND_VOLUME_DEFAULT);

  //---------- 起動処理 ----------
  play_sound(SOUND_DEFAULT);


}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long now = millis();

  if(prev_state != state){
    Serial.print(F("Prev State:"));
    Serial.print(prev_state);
    Serial.print(F(", State:"));
    Serial.println(state);
  }
  if(prev_form != form){
    Serial.print(F("Prev Form:"));
    Serial.print(prev_form);
    Serial.print(F(", Form:"));
    Serial.println(form);
  }

  prev_state = state;
  prev_form  = form;

  // kiem tra nut nhan
  sw_center = digitalRead(SW_CENTER_PIN);
  if(prev_sw_center == SW_ON){
    if(sw_center == SW_ON){ // nut nhan duoc giu
      if(!is_valid_sw_center_long_press){
          sw_center_press_counter++;
      }
      if(!is_valid_sw_center_long_press && (sw_center_press_counter > WAIT_COUNT_LONG_PRESS)){
        is_valid_sw_center_long_press = true;
        switch(state){//chuyen sang state tiep theo
        case STATE_DEFAULT:
          update_state(STATE_1);
          break;
        case STATE_1:
          update_state(STATE_2);
          break;
        case STATE_2:
        update_state(STATE_3);
        break;
        case STATE_3:
          update_state(STATE_4);
          break;
        case STATE_4:
          update_state(STATE_5);
          break;
          case STATE_5:
          update_state(STATE_6);
          break;
          case STATE_6:
          update_state(STATE_7);
          break;
          case STATE_7:
          update_state(STATE_8);
          break;
          case STATE_8:
          update_state(STATE_9);
          break;
          case STATE_9:
          update_state(STATE_DEFAULT);
          break;
        default:
          ;
        }
      }
    }else{ // スイッチが離されたとき
      if(!is_valid_sw_center_long_press){ // 短押しから離されたとき
          switch(state){
            case STATE_DEFAULT:
            break;
          case STATE_1:
              switch(form){
                case FORM_1:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_1);
                break;
                default:
                update_form(FORM_1);
                break;
              }
              break;
          case STATE_2:
              switch(form){
                case FORM_2:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_2);
                break;
                default:
                update_form(FORM_2);
                break;
              }
              break;
          case STATE_3:
              switch(form){
                case FORM_3:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_3);
                break;
                default:
                update_form(FORM_3);
                break;
              }
              break;
          case STATE_4:
              switch(form){
                case FORM_4:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_4);
                break;
                default:
                update_form(FORM_4);
                break;
              }
              break;
              case STATE_5:
              switch(form){
                case FORM_5:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_5);
                break;
                default:
                update_form(FORM_5);
                break;
              }
              break;
              case STATE_6:
              switch(form){
                case FORM_6:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_6);
                break;
                default:
                update_form(FORM_6);
                break;
              }
              break;
              case STATE_7:
              switch(form){
                case FORM_7:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_7);
                break;
                default:
                update_form(FORM_7);
                break;
              }
              break;
              case STATE_8:
              switch(form){
                case FORM_8:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_8);
                break;
                default:
                update_form(FORM_8);
                break;
              }
              break;
              case STATE_9:
              switch(form){
                case FORM_9:
                update_form(FORM_DEFAULT);
                break;
                case FORM_DEFAULT:
                update_form(FORM_9);
                break;
                default:
                update_form(FORM_9);
                break;
              }
              break;
          default:
            ;
          }
        
      }else{  // 長押しから離されたとき
        ;
      }
      sw_center_press_counter = 0;
      is_valid_sw_center_long_press = false;
  }
      
}else{
  ;
}
prev_sw_center = sw_center;
control_sound();
      delay(LOOP_INTERVAL_MS);
}