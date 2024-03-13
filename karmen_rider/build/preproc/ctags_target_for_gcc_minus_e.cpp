# 1 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
# 33 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
const uint16_t WAIT_COUNT_LONG_PRESS = 1000 / 20;

uint8_t state = 10;
uint8_t prev_state = 10;
uint8_t before_state = 10;
uint8_t form = 10;
uint8_t prev_form = 10;
uint8_t before_form = 10;

uint8_t sw_center = 0x1;
uint8_t prev_sw_center = 0x1;

uint16_t sw_center_press_counter = 0;
boolean is_valid_sw_center_long_press = false;

void update_state(uint8_t new_state){
  before_state = state;
  state = new_state;
}

void update_form(uint8_t new_form){
  before_form = form;
  form = new_form;

}

# 60 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 2
# 61 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 2
# 91 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
uint8_t volume = 20;

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
    case 1:
     play_sound(1);
     break;

    case 2:
     play_sound(2);
     break;

    case 3:
     play_sound(3);
     break;

    case 4:
     play_sound(4);
     break;

    case 5:
     play_sound(5);
     break;

    case 6:
     play_sound(6);
     break;

    case 7:
     play_sound(7);
     break;

    case 8:
     play_sound(8);
     break;

    case 9:
     play_sound(9);
     break;

    case 10:
     play_sound(10);
     break;

    }
    }
    if(prev_form!=form){ //
    switch(form){
      case 1:
      loop_sound(11);
      break;

      case 2:
      loop_sound(12);
      break;

      case 3:
      loop_sound(13);
      break;

      case 4:
      loop_sound(14);
      break;

      case 5:
      loop_sound(15);
      break;

      case 6:
      loop_sound(16);
      break;

      case 7:
      loop_sound(17);
      break;

      case 8:
      loop_sound(18);
      break;

      case 9:
      loop_sound(19);
      break;

      case 10:
      loop_sound(20);
      break;
    }
    }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(14, 0x02);
  ss_mp3_player.begin(9600);
  if(!mp3_player.begin(ss_mp3_player)) { //Use softwareSerial to communicate with mp3.
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 212 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "212" "." "40" "\", \"aSM\", @progbits, 1 #"))) = (
# 212 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  "Unable to begin music_player:"
# 212 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  ); &__pstr__[0];}))
# 212 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  ))));
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 213 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "213" "." "41" "\", \"aSM\", @progbits, 1 #"))) = (
# 213 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  "1.Please recheck the connection!"
# 213 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  ); &__pstr__[0];}))
# 213 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  ))));
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 214 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "214" "." "42" "\", \"aSM\", @progbits, 1 #"))) = (
# 214 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  "2.Please insert the SD card!"
# 214 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                  ); &__pstr__[0];}))
# 214 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                  ))));
    while(true);
  }
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 217 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "217" "." "43" "\", \"aSM\", @progbits, 1 #"))) = (
# 217 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                "mp3_player online."
# 217 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                ); &__pstr__[0];}))
# 217 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ))));
  mp3_player.setTimeOut(500); //Set serial communictaion time out 500ms
  update_volume(20);

  //---------- 起動処理 ----------
  play_sound(23 /*system starting*/);


}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long now = millis();

  if(prev_state != state){
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 232 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "232" "." "44" "\", \"aSM\", @progbits, 1 #"))) = (
# 232 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                "Prev State:"
# 232 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                ); &__pstr__[0];}))
# 232 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ))));
    Serial.print(prev_state);
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 234 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "234" "." "45" "\", \"aSM\", @progbits, 1 #"))) = (
# 234 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ", State:"
# 234 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                ); &__pstr__[0];}))
# 234 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ))));
    Serial.println(state);
  }
  if(prev_form != form){
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 238 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "238" "." "46" "\", \"aSM\", @progbits, 1 #"))) = (
# 238 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                "Prev Form:"
# 238 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                ); &__pstr__[0];}))
# 238 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ))));
    Serial.print(prev_form);
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 240 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "karmen_rider.ino" "." "240" "." "47" "\", \"aSM\", @progbits, 1 #"))) = (
# 240 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ", Form:"
# 240 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino" 3
                ); &__pstr__[0];}))
# 240 "D:\\ARDUINOCODE\\ArduinoProjects\\karmen_rider\\karmen_rider.ino"
                ))));
    Serial.println(form);
  }

  prev_state = state;
  prev_form = form;

  // kiem tra nut nhan
  sw_center = digitalRead(14);
  if(prev_sw_center == 0x0){
    if(sw_center == 0x0){ // nut nhan duoc giu
      if(!is_valid_sw_center_long_press){
          sw_center_press_counter++;
      }
      if(!is_valid_sw_center_long_press && (sw_center_press_counter > WAIT_COUNT_LONG_PRESS)){
        is_valid_sw_center_long_press = true;
        switch(state){//chuyen sang state tiep theo
        case 10:
          update_state(1);
          break;
        case 1:
          update_state(2);
          break;
        case 2:
        update_state(3);
        break;
        case 3:
          update_state(4);
          break;
        case 4:
          update_state(5);
          break;
          case 5:
          update_state(6);
          break;
          case 6:
          update_state(7);
          break;
          case 7:
          update_state(8);
          break;
          case 8:
          update_state(9);
          break;
          case 9:
          update_state(10);
          break;
        default:
          ;
        }
      }
    }else{ // スイッチが離されたとき
      if(!is_valid_sw_center_long_press){ // 短押しから離されたとき
          switch(state){
            case 10:
            break;
          case 1:
              switch(form){
                case 1:
                update_form(10);
                break;
                case 10:
                update_form(1);
                break;
                default:
                update_form(1);
                break;
              }
              break;
          case 2:
              switch(form){
                case 2:
                update_form(10);
                break;
                case 10:
                update_form(2);
                break;
                default:
                update_form(2);
                break;
              }
              break;
          case 3:
              switch(form){
                case 3:
                update_form(10);
                break;
                case 10:
                update_form(3);
                break;
                default:
                update_form(3);
                break;
              }
              break;
          case 4:
              switch(form){
                case 4:
                update_form(10);
                break;
                case 10:
                update_form(4);
                break;
                default:
                update_form(4);
                break;
              }
              break;
              case 5:
              switch(form){
                case 5:
                update_form(10);
                break;
                case 10:
                update_form(5);
                break;
                default:
                update_form(5);
                break;
              }
              break;
              case 6:
              switch(form){
                case 6:
                update_form(10);
                break;
                case 10:
                update_form(6);
                break;
                default:
                update_form(6);
                break;
              }
              break;
              case 7:
              switch(form){
                case 7:
                update_form(10);
                break;
                case 10:
                update_form(7);
                break;
                default:
                update_form(7);
                break;
              }
              break;
              case 8:
              switch(form){
                case 8:
                update_form(10);
                break;
                case 10:
                update_form(8);
                break;
                default:
                update_form(8);
                break;
              }
              break;
              case 9:
              switch(form){
                case 9:
                update_form(10);
                break;
                case 10:
                update_form(9);
                break;
                default:
                update_form(9);
                break;
              }
              break;
          default:
            ;
          }

      }else{ // 長押しから離されたとき
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
      delay(20);
}
