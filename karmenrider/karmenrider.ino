/*-------------------------- 基本定義 --------------------------*/
#define SW_CENTER_PIN 6
#define SW_DRIVER_PIN 8
#define IR_RX_PIN 7

#define LOOP_INTERVAL_MS 20

#define SW_ON  LOW
#define SW_OFF HIGH
#define IR_ON  LOW
#define IR_OFF HIGH

#define STATE_SINGLE_A                0
#define STATE_SINGLE_B                1
#define STATE_CHANGE_READY            2
#define STATE_ARMOR_TIME_READY        3
#define STATE_ARMOR_TIME              4
#define STATE_FINISH_TIME_READY       5
#define STATE_FINISH_TIME             6
#define STATE_FINAL_FORM_TIME         7
#define STATE_WEAPON                  8
#define STATE_FINAL_ATTACK_TIME_BREAK 9

#define FORM_GROWING 0
#define FORM_MIGHTY  1
#define FORM_DRAGON  2
#define FORM_PEGASUS 3
#define FORM_TITAN   4
#define FORM_RISING_MIGHTY  5
#define FORM_RISING_DRAGON  6
#define FORM_RISING_PEGASUS 7
#define FORM_RISING_TITAN   8
#define FORM_AMAZING_MIGHTY 9

#define LIMIT_MS_RISING  60000  // 本家は30000msだが、これだと必殺技音が終わる前に解除されてしまうので、意図的に60000msに変更
#define LIMIT_MS_PEGASUS 80000  // 本家は50000msだが、これだと必殺技音が終わる前に解除されてしまう可能性が高いので、意図的に80000msに変更

#define LIMIT_MS_DRIVER_SW 1000
#define LIMIT_MS_INVALID_IR 500
#define WAIT_MS_LONG_PRESS 1000
const uint16_t WAIT_COUNT_LONG_PRESS = WAIT_MS_LONG_PRESS / LOOP_INTERVAL_MS;

uint8_t state = STATE_SINGLE_A;
uint8_t prev_state = STATE_SINGLE_A;
uint8_t before_state = STATE_SINGLE_A;
uint8_t form = FORM_GROWING;
uint8_t prev_form = FORM_GROWING;
uint8_t before_form = FORM_GROWING;
uint8_t driver_counter = 0;

uint8_t sw_center = SW_OFF;
uint8_t sw_driver = SW_OFF;
uint8_t prev_sw_center = SW_OFF;
uint8_t prev_sw_driver = SW_OFF;

uint8_t ir_state      = IR_OFF;
uint8_t prev_ir_state = IR_OFF;

uint16_t sw_center_press_counter = 0;
boolean  is_valid_sw_center_long_press = false;
uint16_t sw_driver_press_counter = 0;
boolean  is_valid_sw_driver_long_press = false;

unsigned long driver_timer  = 0;
unsigned long ir_timer      = 0;
unsigned long pegasus_timer = 0;
unsigned long rising_timer  = 0;
boolean is_counting_driver_timer  = false;
boolean is_counting_ir_timer      = false;
boolean is_counting_pegasus_timer = false;
boolean is_counting_rising_timer  = false;
boolean is_rising_limit_over = false;
boolean is_keeping_rising = false;

/*-------------------------- 発光関係 --------------------------*/
#include <Adafruit_NeoPixel.h>
#define LED_COLOR_PIN  4
#define N_COLOR_LED   10
#define N_RING_LED     7

#define LUMINANCE_BASE 127
#define LUMINANCE_MAX  255
#define LED_COUNT_MAX 1500 // LOOP_INTERVAL_MSが20msで、30sまでの発光定義を想定

struct color_rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct color_rgb COLOR_WHITE  = {127,127,127};
struct color_rgb COLOR_RED    = {127,  0,  0};
struct color_rgb COLOR_BLUE   = {  0,  0,127};
struct color_rgb COLOR_GREEN  = {  0,127,  0};
struct color_rgb COLOR_PURPLE = {127,  0,127};
struct color_rgb COLOR_GOLD   = {127,107,  0};

struct color_rgb COLOR_WHITE_2X  = {255,255,255};
struct color_rgb COLOR_RED_2X    = {255,  0,  0};
struct color_rgb COLOR_BLUE_2X   = {  0,  0,255};
struct color_rgb COLOR_GREEN_2X  = {  0,255,  0};
struct color_rgb COLOR_PURPLE_2X = {255,  0,255};
struct color_rgb COLOR_GOLD_2X   = {255,215,  0};

uint16_t led_counter = LED_COUNT_MAX;
uint8_t  led_index = 0;
unsigned long prev_face_blink_time = 0;
unsigned long prev_ring_blink_time = 0;
unsigned long prev_circle_time = 0;
unsigned long ring_inc_dim_start_time = 0;
unsigned long face_inc_dim_start_time = 0;
boolean is_face_lighting = false;
boolean is_ring_lighting = false;
boolean is_ring_inc = false;
boolean is_face_inc = false;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(N_COLOR_LED, LED_COLOR_PIN, NEO_GRB);

void led_base_pattern_ring_on(struct color_rgb *color){
  ring_inc_dim_start_time = 0;
  led_index = 0;
  for(uint8_t i=0;i<N_RING_LED;i++){
    pixels.setPixelColor(i, pixels.Color(color->r,color->g,color->b));
  }
}

void led_base_pattern_face_on(struct color_rgb *color){
  face_inc_dim_start_time = 0;
  pixels.setPixelColor(7, pixels.Color(color->r,color->g,color->b));
  pixels.setPixelColor(8, pixels.Color(COLOR_WHITE.r,COLOR_WHITE.g,COLOR_WHITE.b));
  pixels.setPixelColor(9, pixels.Color(color->r,color->g,color->b));
}

void led_base_pattern_ring_off(){
  ring_inc_dim_start_time = 0;
  led_index = 0;
  for(uint8_t i=0;i<N_RING_LED;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
}

void led_base_pattern_face_off(){
  face_inc_dim_start_time = 0;
  pixels.setPixelColor(7, pixels.Color(0,0,0));
  pixels.setPixelColor(8, pixels.Color(0,0,0));
  pixels.setPixelColor(9, pixels.Color(0,0,0));
}

void led_base_pattern_ring_blink(struct color_rgb *color, int interval_ms){
  ring_inc_dim_start_time = 0;
  unsigned long now = millis();
  if(now - prev_ring_blink_time >= interval_ms){
    if(is_ring_lighting){
      for(uint8_t i=0;i<N_RING_LED;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
      }
    }else{
      for(uint8_t i=0;i<N_RING_LED;i++){
    pixels.setPixelColor(i, pixels.Color(color->r, color->g, color->b));
  }
    }
    is_ring_lighting = !is_ring_lighting;
    prev_ring_blink_time = now;
  }
}

void led_base_pattern_face_blink(struct color_rgb *color, int interval_ms){
  face_inc_dim_start_time = 0;
  unsigned long now = millis();
  if(now - prev_face_blink_time >= interval_ms){
    if(is_face_lighting){
      pixels.setPixelColor(7, pixels.Color(0,0,0));
      pixels.setPixelColor(8, pixels.Color(0,0,0));
      pixels.setPixelColor(9, pixels.Color(0,0,0));
    }else{
      pixels.setPixelColor(7, pixels.Color(color->r,color->g,color->b));
      pixels.setPixelColor(8, pixels.Color(COLOR_WHITE.r,COLOR_WHITE.g,COLOR_WHITE.b));
      pixels.setPixelColor(9, pixels.Color(color->r,color->g,color->b));
    }
    is_face_lighting = !is_face_lighting;
    prev_face_blink_time = now;
  }
}

void led_base_pattern_ring_inc(struct color_rgb *color, int interval_ms, uint8_t steps){
  led_index = 0;
  unsigned long now = millis();
  if(ring_inc_dim_start_time == 0){
    ring_inc_dim_start_time = now;
  }
  int ms_per_step = interval_ms / steps;
  int current_step = (now - ring_inc_dim_start_time) / ms_per_step;
  if(current_step > steps){
    current_step = steps;
  }
  uint8_t r_step = color->r/steps;
  uint8_t g_step = color->g/steps;
  uint8_t b_step = color->b/steps;
  for(uint8_t i=0;i<N_RING_LED;i++){
    pixels.setPixelColor(i, pixels.Color(r_step*current_step, g_step*current_step, b_step*current_step));
  }
  /*
  if(now - ring_inc_dim_start_time > interval_ms){
    ring_inc_dim_start_time = 0;
  }
  */
}

void led_base_pattern_face_inc(struct color_rgb *color, int interval_ms, uint8_t steps){
  unsigned long now = millis();
  if(face_inc_dim_start_time == 0){
    face_inc_dim_start_time = now;
  }
  int ms_per_step = interval_ms / steps;
  int current_step = (now - face_inc_dim_start_time) / ms_per_step;
  if(current_step > steps){
    current_step = steps;
  }
  uint8_t r_step = color->r/steps;
  uint8_t g_step = color->g/steps;
  uint8_t b_step = color->b/steps;
  uint8_t w_step = COLOR_WHITE.r/steps;
  pixels.setPixelColor(7, pixels.Color(r_step*current_step, g_step*current_step, b_step*current_step));
  pixels.setPixelColor(8, pixels.Color(w_step*current_step, w_step*current_step, w_step*current_step));
  pixels.setPixelColor(9, pixels.Color(r_step*current_step, g_step*current_step, b_step*current_step));
  /*
  if(now - face_inc_dim_start_time > interval_ms){
    face_inc_dim_start_time = 0;
  }
  */
}

void led_base_pattern_ring_dim(struct color_rgb *color, int interval_ms, uint8_t steps){
  led_index = 0;
  unsigned long now = millis();
  if(ring_inc_dim_start_time == 0){
    ring_inc_dim_start_time = now;
  }
  int ms_per_step = interval_ms / steps;
  int current_step = (now - ring_inc_dim_start_time) / ms_per_step;
  if(current_step > steps){
    current_step = steps;
  }
  uint8_t r_step = color->r/steps;
  uint8_t g_step = color->g/steps;
  uint8_t b_step = color->b/steps;
  for(uint8_t i=0;i<N_RING_LED;i++){
    pixels.setPixelColor(i, pixels.Color(r_step*(steps-current_step), g_step*(steps-current_step), b_step*(steps-current_step)));
  }
  /*
  if(now - ring_inc_dim_start_time >= interval_ms){
    ring_inc_dim_start_time = 0;
  }
  */
}

void led_base_pattern_face_dim(struct color_rgb *color, int interval_ms, uint8_t steps){
  unsigned long now = millis();
  if(face_inc_dim_start_time == 0){
    face_inc_dim_start_time = now;
  }
  int ms_per_step = interval_ms / steps;
  int current_step = (now - face_inc_dim_start_time) / ms_per_step;
  if(current_step > steps){
    current_step = steps;
  }
  uint8_t r_step = color->r/steps;
  uint8_t g_step = color->g/steps;
  uint8_t b_step = color->b/steps;
  uint8_t w_step = COLOR_WHITE.r/steps;
  pixels.setPixelColor(7, pixels.Color(r_step*(steps-current_step), g_step*(steps-current_step), b_step*(steps-current_step)));
  pixels.setPixelColor(8, pixels.Color(w_step*(steps-current_step), w_step*(steps-current_step), w_step*(steps-current_step)));
  pixels.setPixelColor(9, pixels.Color(r_step*(steps-current_step), g_step*(steps-current_step), b_step*(steps-current_step)));
  /*
  if(now - ring_inc_dim_start_time >= interval_ms){
    ring_inc_dim_start_time = 0;
  }
  */
}

void led_base_pattern_ring_blink_slowly(struct color_rgb *color, int interval_ms, uint8_t steps){
  led_index = 0;
  unsigned long now = millis();
  if(ring_inc_dim_start_time == 0){
    ring_inc_dim_start_time = now;
  }
  int ms_per_step = interval_ms / steps;
  int current_step = (now - ring_inc_dim_start_time) / ms_per_step;
  if(current_step > steps){
    current_step = steps;
  }
  uint8_t r_step = color->r/steps;
  uint8_t g_step = color->g/steps;
  uint8_t b_step = color->b/steps;
  if(is_ring_inc){
    for(uint8_t i=0;i<N_RING_LED;i++){
      pixels.setPixelColor(i, pixels.Color(r_step*current_step, g_step*current_step, b_step*current_step));
    }
  }else{
    for(uint8_t i=0;i<N_RING_LED;i++){
      pixels.setPixelColor(i, pixels.Color(r_step*(steps-current_step), g_step*(steps-current_step), b_step*(steps-current_step)));
    }
  }
  if(now - ring_inc_dim_start_time >= interval_ms){
    is_ring_inc = !is_ring_inc;
    ring_inc_dim_start_time = 0;
  }
}

void led_base_pattern_ring_circle(struct color_rgb *base_color,struct color_rgb *point_color, int interval_ms){
  ring_inc_dim_start_time = 0;
  unsigned long now = millis();
  if(now - prev_circle_time >= interval_ms){
    for(uint8_t i=0;i<led_index;i++){
      pixels.setPixelColor(i, pixels.Color(base_color->r, base_color->g, base_color->b));
    }
    pixels.setPixelColor(led_index, pixels.Color(point_color->r, point_color->g, point_color->b));
    for(uint8_t i=led_index+1;i<N_RING_LED;i++){
      pixels.setPixelColor(i, pixels.Color(base_color->r, base_color->g, base_color->b));
    }
    led_index++;
    if(led_index == N_RING_LED){
      led_index = 0;
    }
    prev_circle_time = now;
  }
}

// 設定したLEDパターンを実行する。この間、スイッチの操作などは受け付けられないので注意
void execute_led_pattern(void (*func)(uint16_t), uint16_t duration_ms){
  for(uint16_t i=0;i<duration_ms;i+=LOOP_INTERVAL_MS){
    func(i);
    pixels.show();
    delay(LOOP_INTERVAL_MS);
  }
}

//----------------------//

void led_pattern_power_on(uint16_t led_counter_ms){ // 起動音＋起動音＋『クウガ』
  if(led_counter_ms <= 6800){
    led_base_pattern_ring_inc(&COLOR_WHITE, 6800, 40);
    led_base_pattern_face_inc(&COLOR_WHITE, 6800, 40);
  }else{
    led_base_pattern_ring_on(&COLOR_WHITE);
    led_base_pattern_face_on(&COLOR_WHITE);
  }
}

void led_pattern_single_a(uint16_t led_counter_ms){ // 起動音＋起動音＋『クウガ』
  if(led_counter_ms <= 440){ led_base_pattern_ring_circle(&COLOR_WHITE, &COLOR_WHITE_2X, 40);
                             led_base_pattern_face_on(&COLOR_WHITE);}
  else if( 440 < led_counter_ms && led_counter_ms <= 500){  led_base_pattern_ring_off();}
  else if( 500 < led_counter_ms && led_counter_ms <= 560){  led_base_pattern_ring_on(&COLOR_WHITE_2X);}
  else if( 560 < led_counter_ms && led_counter_ms <= 1460){ led_base_pattern_ring_dim(&COLOR_WHITE_2X, 1800, 10);} // 半分まで暗くする
  else if(1460 < led_counter_ms && led_counter_ms <= 2960){ led_base_pattern_ring_on(&COLOR_WHITE);
                                                            led_base_pattern_face_blink(&COLOR_WHITE,40);}
  else{                                                     led_base_pattern_face_on(&COLOR_WHITE);}
}

void led_pattern_single_b(uint16_t led_counter_ms){ // 起動音＋説明＋『クウガ』
  if(led_counter_ms <= 440){ led_base_pattern_ring_circle(&COLOR_WHITE, &COLOR_WHITE_2X, 40);
                             led_base_pattern_face_on(&COLOR_WHITE);}
  else if( 440 < led_counter_ms && led_counter_ms <= 500){  led_base_pattern_ring_off();}
  else if( 500 < led_counter_ms && led_counter_ms <= 560){  led_base_pattern_ring_on(&COLOR_WHITE_2X);}
  else if( 560 < led_counter_ms && led_counter_ms <= 1460){ led_base_pattern_ring_dim(&COLOR_WHITE_2X, 1800, 10);} // 半分まで暗くする
  else if(1460 < led_counter_ms && led_counter_ms <= 1560){ led_base_pattern_ring_on(&COLOR_WHITE_2X);}
  else if(1560 < led_counter_ms && led_counter_ms <= 2560){ led_base_pattern_ring_dim(&COLOR_WHITE_2X, 2000, 20);} // 半分まで暗くする
  else if(2560 < led_counter_ms && led_counter_ms <= 5660){ led_base_pattern_ring_on(&COLOR_WHITE);}
  else if(5660 < led_counter_ms && led_counter_ms <= 8040){ led_base_pattern_face_blink(&COLOR_WHITE,90);}
  else if(8040 < led_counter_ms && led_counter_ms <= 8560){ led_base_pattern_face_off();}
  else{                                                     led_base_pattern_face_on(&COLOR_WHITE);}
}
//-------------------------------------------------------
void led_pattern_single_base(struct color_rgb *color_ring, struct color_rgb *color_face){
  led_base_pattern_ring_on(color_ring);
  led_base_pattern_face_on(color_face);
}

void led_pattern_single_growing(){
  led_pattern_single_base(&COLOR_WHITE, &COLOR_WHITE);
}

void led_pattern_single_mighty(){
  led_pattern_single_base(&COLOR_RED, &COLOR_RED);
}

void led_pattern_single_dragon(){
  led_pattern_single_base(&COLOR_BLUE, &COLOR_BLUE);
}

void led_pattern_single_pegasus(){
  led_pattern_single_base(&COLOR_GREEN, &COLOR_GREEN);
}

void led_pattern_single_titan(){
  led_pattern_single_base(&COLOR_PURPLE, &COLOR_PURPLE);
}

void led_pattern_single_rising_mighty(){
  led_pattern_single_base(&COLOR_GOLD, &COLOR_RED);
}
//-------------------------------------------------------
void led_pattern_change_ready(uint16_t led_counter_ms){ // 変身待機音
  if(led_counter_ms <= 1000){ led_base_pattern_ring_inc(&COLOR_RED, 1000, 20);
                              led_base_pattern_face_on(&COLOR_WHITE);}
  else if(1000 < led_counter_ms && led_counter_ms <= 1200){ led_base_pattern_ring_on(&COLOR_RED_2X);}
  else{                                                     led_base_pattern_ring_blink_slowly(&COLOR_RED, 400, 10);}
}
//-------------------------------------------------------
void led_pattern_change_mighty_init(uint16_t led_counter_ms){ // 変身
  if(led_counter_ms <= 500){ led_base_pattern_ring_on(&COLOR_RED);
                             led_base_pattern_face_on(&COLOR_WHITE);}
  else if( 500 < led_counter_ms && led_counter_ms <= 2500){ led_base_pattern_ring_circle(&COLOR_RED, &COLOR_RED_2X, 60);}
  else if(2500 < led_counter_ms && led_counter_ms <= 4500){ led_base_pattern_ring_circle(&COLOR_RED, &COLOR_RED_2X, 40);}
  else if(4500 < led_counter_ms && led_counter_ms <= 5500){ led_pattern_single_base(&COLOR_RED_2X, &COLOR_RED_2X);}
  else{                                                     led_pattern_single_base(&COLOR_RED, &COLOR_RED);}
}
//-------------------------------------------------------
void led_pattern_change_long_base(struct color_rgb *prev_color, struct color_rgb *next_color, struct color_rgb *next_color_2x, uint16_t led_counter_ms){
  if(led_counter_ms <= 1000){ led_base_pattern_ring_inc(next_color, 1000, 10);
                              led_base_pattern_face_on(prev_color);}
  else if(1000 < led_counter_ms && led_counter_ms <= 2500){ led_base_pattern_ring_circle(next_color, next_color_2x, 60);}
  else if(2500 < led_counter_ms && led_counter_ms <= 4500){ led_base_pattern_ring_circle(next_color, next_color_2x, 40);}
  else if(4500 < led_counter_ms && led_counter_ms <= 5800){ led_pattern_single_base(next_color_2x,next_color_2x);}
  else{                                                     led_pattern_single_base(next_color,next_color);}
}

void led_pattern_change_mighty_long(uint16_t led_counter_ms){
  led_pattern_change_long_base(&COLOR_PURPLE, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}

void led_pattern_change_dragon_long(uint16_t led_counter_ms){
  led_pattern_change_long_base(&COLOR_RED, &COLOR_BLUE, &COLOR_BLUE_2X, led_counter_ms);
}

void led_pattern_change_pegasus_long(uint16_t led_counter_ms){
  led_pattern_change_long_base(&COLOR_BLUE, &COLOR_GREEN, &COLOR_GREEN_2X, led_counter_ms);
}

void led_pattern_change_titan_long(uint16_t led_counter_ms){
  led_pattern_change_long_base(&COLOR_GREEN, &COLOR_PURPLE, &COLOR_PURPLE_2X, led_counter_ms);
}
//-------------------------------------------------------

void led_pattern_change_short_base(struct color_rgb *prev_color, struct color_rgb *next_color, struct color_rgb *next_color_2x, uint16_t led_counter_ms){
  if(led_counter_ms <= 1000){ led_base_pattern_ring_circle(next_color, next_color_2x, 60);
                              led_base_pattern_face_on(prev_color);}
  else if(1000 < led_counter_ms && led_counter_ms <= 2400){ led_base_pattern_ring_circle(next_color, next_color_2x, 40);}
  else if(2400 < led_counter_ms && led_counter_ms <= 3400){ led_pattern_single_base(next_color_2x,next_color_2x);}
  else{                                                     led_pattern_single_base(next_color,next_color);}
}

void led_pattern_change_mighty_short(uint16_t led_counter_ms){
  led_pattern_change_short_base(&COLOR_PURPLE, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}

void led_pattern_change_dragon_short(uint16_t led_counter_ms){
  led_pattern_change_short_base(&COLOR_RED, &COLOR_BLUE, &COLOR_BLUE_2X, led_counter_ms);
}

void led_pattern_change_pegasus_short(uint16_t led_counter_ms){
  led_pattern_change_short_base(&COLOR_BLUE, &COLOR_GREEN, &COLOR_GREEN_2X, led_counter_ms);
}

void led_pattern_change_titan_short(uint16_t led_counter_ms){
  led_pattern_change_short_base(&COLOR_GREEN, &COLOR_PURPLE, &COLOR_PURPLE_2X, led_counter_ms);
}
//-------------------------------------------------------
void led_pattern_limit_pegasus(uint16_t led_counter_ms){
  if(led_counter_ms <= 1000){ led_pattern_single_base(&COLOR_GREEN_2X,&COLOR_GREEN_2X);}
  else if(1000 < led_counter_ms && led_counter_ms <= 3000){ led_base_pattern_ring_dim(&COLOR_GREEN_2X, 4000, 20);
                                                            led_base_pattern_face_dim(&COLOR_GREEN_2X, 4000, 20);}
  else{                                                     led_pattern_single_base(&COLOR_WHITE,&COLOR_WHITE);}
}

void led_pattern_limit_rising_base(struct color_rgb *color_ring, struct color_rgb *color_face, uint16_t led_counter_ms){
  if(led_counter_ms <= 4000){ led_base_pattern_ring_blink_slowly(&COLOR_GOLD, 800, 10);}
  else{                       led_pattern_single_base(color_ring, color_face);}
}

void led_pattern_limit_rising_mighty(uint16_t led_counter_ms){
  led_pattern_limit_rising_base(&COLOR_RED, &COLOR_RED, led_counter_ms);
}

void led_pattern_limit_rising_dragon(uint16_t led_counter_ms){
  led_pattern_limit_rising_base(&COLOR_BLUE, &COLOR_BLUE, led_counter_ms);
}

void led_pattern_limit_rising_pegasus(uint16_t led_counter_ms){ // グローイングに戻る
  led_pattern_limit_rising_base(&COLOR_WHITE, &COLOR_WHITE, led_counter_ms);
}

void led_pattern_limit_rising_titan(uint16_t led_counter_ms){
  led_pattern_limit_rising_base(&COLOR_PURPLE, &COLOR_PURPLE, led_counter_ms);
}
//-------------------------------------------------------
void led_pattern_deactivate_rising_base(struct color_rgb *color_ring, struct color_rgb *color_face, uint16_t led_counter_ms){
  if(led_counter_ms <= 1000){ led_pattern_single_base(&COLOR_GOLD, color_face);}
  else{                       led_pattern_single_base(color_ring, color_face);}
}

void led_pattern_deactivate_rising_mighty(uint16_t led_counter_ms){
  led_pattern_deactivate_rising_base(&COLOR_RED, &COLOR_RED, led_counter_ms);
}

void led_pattern_deactivate_rising_dragon(uint16_t led_counter_ms){
  led_pattern_deactivate_rising_base(&COLOR_BLUE, &COLOR_BLUE, led_counter_ms);
}

void led_pattern_deactivate_rising_pegasus(uint16_t led_counter_ms){
  led_pattern_deactivate_rising_base(&COLOR_GREEN, &COLOR_GREEN, led_counter_ms);
}

void led_pattern_deactivate_rising_titan(uint16_t led_counter_ms){
  led_pattern_deactivate_rising_base(&COLOR_PURPLE, &COLOR_PURPLE, led_counter_ms);
}

void led_pattern_deactivate_amazing_mighty(uint16_t led_counter_ms){
  if(led_counter_ms <= 1000){ led_pattern_single_base(&COLOR_GOLD_2X, &COLOR_RED);}
  else{                       led_pattern_single_base(&COLOR_GOLD, &COLOR_RED);}
}
//-------------------------------------------------------
void led_pattern_armor_time_ready_base(struct color_rgb *color_ring, struct color_rgb *color_face){
  led_base_pattern_ring_blink(color_ring, 800);
  led_base_pattern_face_on(color_face);
}

void led_pattern_armor_time_ready_growing(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_WHITE, &COLOR_WHITE);
}

void led_pattern_armor_time_ready_mighty(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_RED, &COLOR_RED);
}

void led_pattern_armor_time_ready_dragon(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_BLUE, &COLOR_BLUE);
}

void led_pattern_armor_time_ready_pegasus(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_GREEN, &COLOR_GREEN);
}

void led_pattern_armor_time_ready_titan(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_PURPLE, &COLOR_PURPLE);
}

void led_pattern_armor_time_ready_rising_mighty(uint16_t led_counter_ms){
  led_pattern_armor_time_ready_base(&COLOR_GOLD, &COLOR_RED);
}
//-------------------------------------------------------
void led_pattern_armor_time_base(struct color_rgb *color_ring, struct color_rgb *color_ring_2x,
                                 struct color_rgb *color_face, struct color_rgb *color_face_2x, uint16_t led_counter_ms){
  if(led_counter_ms <= 6600){                                 led_pattern_single_base(color_ring, color_face);}
  else if( 6600 < led_counter_ms && led_counter_ms <=  7300){ led_base_pattern_face_dim(color_face_2x, 700, 10);}
  else if( 7300 < led_counter_ms && led_counter_ms <=  7400){ led_base_pattern_face_on(color_face_2x);}
  else if( 7400 < led_counter_ms && led_counter_ms <=  8100){ led_base_pattern_face_dim(color_face_2x, 700, 10);}
  else if( 8100 < led_counter_ms && led_counter_ms <= 10600){ led_base_pattern_face_on(color_face);}
  else if(10600 < led_counter_ms && led_counter_ms <= 14500){ led_base_pattern_ring_circle(color_ring, color_ring_2x, 40);}
  else if(14500 < led_counter_ms && led_counter_ms <= 16200){ led_pattern_single_base(color_ring_2x, color_face_2x);}
  else{                                                       led_pattern_single_base(color_ring, color_face);}
}

void led_pattern_armor_time_growing(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_WHITE, &COLOR_WHITE_2X, &COLOR_WHITE, &COLOR_WHITE_2X, led_counter_ms);
}

void led_pattern_armor_time_mighty(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_RED, &COLOR_RED_2X, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}

void led_pattern_armor_time_dragon(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_BLUE, &COLOR_BLUE_2X, &COLOR_BLUE, &COLOR_BLUE_2X, led_counter_ms);
}

void led_pattern_armor_time_pegasus(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_GREEN, &COLOR_GREEN_2X, &COLOR_GREEN, &COLOR_GREEN_2X, led_counter_ms);
}

void led_pattern_armor_time_titan(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_PURPLE, &COLOR_PURPLE_2X, &COLOR_PURPLE, &COLOR_PURPLE_2X, led_counter_ms);
}

void led_pattern_armor_time_rising_mighty(uint16_t led_counter_ms){
  led_pattern_armor_time_base(&COLOR_GOLD, &COLOR_GOLD_2X, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}
//-------------------------------------------------------
void led_pattern_finish_time_ready_base(struct color_rgb *color_ring, struct color_rgb *color_ring_2x,
                                        struct color_rgb *color_face, struct color_rgb *color_face_2x, uint16_t led_counter_ms){
  if(led_counter_ms <= 440){ led_base_pattern_ring_circle(color_ring, color_ring_2x, 40);
                             led_base_pattern_face_on(color_face);}
  else if( 440 < led_counter_ms && led_counter_ms <= 500){  led_base_pattern_ring_off();}
  else if( 500 < led_counter_ms && led_counter_ms <= 560){  led_base_pattern_ring_on(color_ring_2x);}
  else if( 560 < led_counter_ms && led_counter_ms <= 1460){ led_base_pattern_ring_dim(color_ring_2x, 1200, 10);} // 3/4まで暗くする
  else if(1460 < led_counter_ms && led_counter_ms <= 4460){ led_pattern_single_base(color_ring, color_face);}
  else{                                                     led_base_pattern_ring_blink(color_ring, 800);
                                                            led_base_pattern_face_on(color_face);}
}

void led_pattern_finish_time_ready_growing(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_WHITE, &COLOR_WHITE_2X, &COLOR_WHITE, &COLOR_WHITE_2X, led_counter_ms);
}

void led_pattern_finish_time_ready_mighty(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_RED, &COLOR_RED_2X, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}

void led_pattern_finish_time_ready_dragon(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_BLUE, &COLOR_BLUE_2X, &COLOR_BLUE, &COLOR_BLUE_2X, led_counter_ms);
}

void led_pattern_finish_time_ready_pegasus(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_GREEN, &COLOR_GREEN_2X, &COLOR_GREEN, &COLOR_GREEN_2X, led_counter_ms);
}

void led_pattern_finish_time_ready_titan(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_PURPLE, &COLOR_PURPLE_2X, &COLOR_PURPLE, &COLOR_PURPLE_2X, led_counter_ms);
}

void led_pattern_finish_time_ready_rising_mighty(uint16_t led_counter_ms){
  led_pattern_finish_time_ready_base(&COLOR_GOLD, &COLOR_GOLD_2X, &COLOR_RED, &COLOR_RED_2X, led_counter_ms);
}
//-------------------------------------------------------
void led_pattern_finish_time_base(struct color_rgb *color_ring, struct color_rgb *color_face, uint16_t led_counter_ms){
  if(led_counter_ms <= 1100){                               led_pattern_single_base(color_ring, color_face);}
  else if(1100 < led_counter_ms && led_counter_ms <= 1600){ led_base_pattern_ring_blink(color_ring, 40);}
  else{                                                     led_pattern_single_base(color_ring, color_face);}
}

void led_pattern_finish_time_growing(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_WHITE, &COLOR_WHITE, led_counter_ms);
}

void led_pattern_finish_time_mighty(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_RED, &COLOR_RED, led_counter_ms);
}

void led_pattern_finish_time_dragon(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_BLUE, &COLOR_BLUE, led_counter_ms);
}

void led_pattern_finish_time_pegasus(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_GREEN, &COLOR_GREEN, led_counter_ms);
}

void led_pattern_finish_time_titan(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_PURPLE, &COLOR_PURPLE, led_counter_ms);
}

void led_pattern_finish_time_rising_mighty(uint16_t led_counter_ms){
  led_pattern_finish_time_base(&COLOR_GOLD, &COLOR_RED, led_counter_ms);
}
//-------------------------------------------------------

void led_pattern_final_form_time_mighty(uint16_t led_counter_ms){
  if(led_counter_ms <= 4000){                               led_pattern_single_base(&COLOR_WHITE, &COLOR_WHITE);}
  else if(4000 < led_counter_ms && led_counter_ms <= 5000){ led_base_pattern_ring_circle(&COLOR_RED, &COLOR_RED_2X, 60);}
  else if(5000 < led_counter_ms && led_counter_ms <= 6400){ led_base_pattern_ring_circle(&COLOR_RED, &COLOR_RED_2X, 40);}
  else if(6400 < led_counter_ms && led_counter_ms <= 7400){ led_pattern_single_base(&COLOR_RED_2X, &COLOR_RED_2X);}
  else{                                                     led_pattern_single_base(&COLOR_RED,&COLOR_RED);}
}

void led_pattern_final_form_time_rising_base(struct color_rgb *color_ring, struct color_rgb *color_face, uint16_t led_counter_ms){
  if(led_counter_ms <= 4000){                                led_pattern_single_base(color_ring, color_face);}
  else if(4000 < led_counter_ms && led_counter_ms <= 11000){ led_base_pattern_ring_blink_slowly(&COLOR_GOLD, 600, 10);}
  else{                                                      led_pattern_single_base(&COLOR_GOLD, color_face);}
}

void led_pattern_final_form_time_rising_mighty(uint16_t led_counter_ms){
  led_pattern_final_form_time_rising_base(&COLOR_RED, &COLOR_RED, led_counter_ms);
}

void led_pattern_final_form_time_rising_dragon(uint16_t led_counter_ms){
  led_pattern_final_form_time_rising_base(&COLOR_BLUE, &COLOR_BLUE, led_counter_ms);
}

void led_pattern_final_form_time_rising_pegasus(uint16_t led_counter_ms){
  led_pattern_final_form_time_rising_base(&COLOR_GREEN, &COLOR_GREEN, led_counter_ms);
}

void led_pattern_final_form_time_rising_titan(uint16_t led_counter_ms){
  led_pattern_final_form_time_rising_base(&COLOR_PURPLE, &COLOR_PURPLE, led_counter_ms);
}

void led_pattern_final_form_time_amazing_mighty(uint16_t led_counter_ms){
  if(led_counter_ms <= 5500){                                led_pattern_single_base(&COLOR_GOLD, &COLOR_RED);}
  else if(5500 < led_counter_ms && led_counter_ms <= 14000){ led_base_pattern_ring_blink_slowly(&COLOR_GOLD_2X, 600, 10);}
  else{                                                      led_pattern_single_base(&COLOR_GOLD_2X, &COLOR_RED);}
}
//-------------------------------------------------------
void led_pattern_final_attack_time_break_base(struct color_rgb *color_ring, struct color_rgb *color_face){
  led_pattern_single_base(color_ring, color_face);
}

void led_pattern_final_attack_time_break_growing(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_WHITE, &COLOR_WHITE);
}

void led_pattern_final_attack_time_break_mighty(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_RED, &COLOR_RED);
}

void led_pattern_final_attack_time_break_dragon(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_BLUE, &COLOR_BLUE);
}

void led_pattern_final_attack_time_break_pegasus(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GREEN, &COLOR_GREEN);
}

void led_pattern_final_attack_time_break_titan(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_PURPLE, &COLOR_PURPLE);
}

void led_pattern_final_attack_time_break_rising_mighty(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GOLD, &COLOR_RED);
}

void led_pattern_final_attack_time_break_rising_dragon(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GOLD, &COLOR_BLUE);
}

void led_pattern_final_attack_time_break_rising_pegasus(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GOLD, &COLOR_GREEN);
}

void led_pattern_final_attack_time_break_rising_titan(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GOLD, &COLOR_PURPLE);
}

void led_pattern_final_attack_time_break_amazing_mighty(uint16_t led_counter_ms){
  led_pattern_final_attack_time_break_base(&COLOR_GOLD_2X, &COLOR_RED);
}
//-------------------------------------------------------

void control_led(){
  // 基本的に状態変化が発生するとカウントを0にしてリセットするが、いくつかのケースではカウントをリセットしない

  if(prev_state != state){
    switch(state){
    case STATE_FINAL_FORM_TIME:
      switch(prev_state){
      case STATE_WEAPON:
      case STATE_FINAL_ATTACK_TIME_BREAK:
        ;
        break;
      default:
        led_counter = 0;
      }
      break;
    default:
      led_counter = 0;
    }
  }else{
    if(prev_form != form){ // 状態は変わらずフォームだけが変わるときはここで対応する。この間はスイッチの操作を受け付けない
      // ・STATE_SINGLE_Aの状態から超変身するとき
      // ・アーマータイム中/フィニッシュタイム直後に超変身するとき
      // ・ライジングから時間切れで元のフォームに戻るどき
      // ・ペガサス/ライジングペガサスから時間切れでグローイングに変わるとき
      switch(form){
      case FORM_GROWING:
        switch(before_form){
        case FORM_PEGASUS:
          execute_led_pattern(led_pattern_limit_pegasus, 3550);
          break;
        case FORM_RISING_PEGASUS:
          execute_led_pattern(led_pattern_limit_rising_pegasus, 6500);
          break;
        default: ;
        }
        break;
      case FORM_MIGHTY:
        switch(before_form){
        case FORM_TITAN:
          switch(state){
          case STATE_SINGLE_A:
            execute_led_pattern(led_pattern_change_mighty_long, 5850);
            break;
          case STATE_ARMOR_TIME:
          case STATE_FINISH_TIME:
            execute_led_pattern(led_pattern_change_mighty_short, 3550);
            break;
          default:
            ;
          }
          break;
        case FORM_RISING_MIGHTY:
          execute_led_pattern(led_pattern_limit_rising_mighty, 6500);
          break;
        default: ;
        }
        break;
      case FORM_DRAGON:
        switch(before_form){
        case FORM_MIGHTY:
          switch(state){
          case STATE_SINGLE_A:
            execute_led_pattern(led_pattern_change_dragon_long, 5850);
            break;
          case STATE_ARMOR_TIME:
          case STATE_FINISH_TIME:
            execute_led_pattern(led_pattern_change_dragon_short, 3550);
            break;
          default:
            ;
          }
          break;
        case FORM_RISING_DRAGON:
          execute_led_pattern(led_pattern_limit_rising_dragon, 6500);
          break;
        default: ;
        }
        break;
      case FORM_PEGASUS:
        switch(before_form){
        case FORM_DRAGON:
          switch(state){
          case STATE_SINGLE_A:
            execute_led_pattern(led_pattern_change_pegasus_long, 5850);
            break;
          case STATE_ARMOR_TIME:
          case STATE_FINISH_TIME:
            execute_led_pattern(led_pattern_change_pegasus_short, 3550);
            break;
          default:
            ;
          }
          break;
        case FORM_RISING_PEGASUS:
          execute_led_pattern(led_pattern_limit_rising_pegasus, 6500);
          break;
        default: ;
        }
        break;
      case FORM_TITAN:
        switch(before_form){
        case FORM_PEGASUS:
          switch(state){
          case STATE_SINGLE_A:
            execute_led_pattern(led_pattern_change_titan_long, 5850);
            break;
          case STATE_ARMOR_TIME:
          case STATE_FINISH_TIME:
            execute_led_pattern(led_pattern_change_titan_short, 3550);
            break;
          default:
            ;
          }
          break;
        case FORM_RISING_TITAN:
          execute_led_pattern(led_pattern_limit_rising_titan, 6500);
          break;
        default: ;
        }
        break;
      default:
        ;
      }
    }
  }

  uint16_t led_counter_ms = led_counter * LOOP_INTERVAL_MS; // LEDはms単位で制御

  // ---------- 状態ごとの発光処理 ----------
  switch(state){
  case STATE_SINGLE_A:
    // この状態には複数の状態から遷移し得る
    // ・STATE_SINGLE_Bで竜頭ボタンを押された時（グローイング以外は発光させない）
    // ・STATE_SINGLE_Bから竜頭ボタンを長押しされたとき（超変身）
    // ・STATE_CHANGE_READYで竜頭ボタンを押された時（マイティフォームへの変身）
    // ・ドライバー装填状態（STATE_ARMOR_TIME_READY, ARMOR_TIME, FINISH_TIME_READY）から解除されたとき（ただの発光状態）
    // ・ネオディケイドライドウォッチ装填状態（STATE_FINAL_FORM_TIME, WEAPON, FINAL_ATTACK_TIME_BREAK）から解除されたとき（強化解除）
    switch(before_state){
    case STATE_SINGLE_B:
      switch(form){
      case FORM_GROWING:
        led_pattern_single_b(led_counter_ms);
        break;
      case FORM_MIGHTY:
        if(before_form != form){
          led_pattern_change_mighty_long(led_counter_ms);
        }
        break;
      case FORM_DRAGON:
        if(before_form != form){
          led_pattern_change_dragon_long(led_counter_ms);
        }
        break;
      case FORM_PEGASUS:
        if(before_form != form){
          led_pattern_change_pegasus_long(led_counter_ms);
        }
        break;
      case FORM_TITAN:
        if(before_form != form){
          led_pattern_change_titan_long(led_counter_ms);
        }
        break;
      default:
        ;
      }
      break;
    case STATE_CHANGE_READY:
      led_pattern_change_mighty_init(led_counter_ms);
      break;
    case STATE_ARMOR_TIME_READY:
    case STATE_ARMOR_TIME:
    case STATE_FINISH_TIME_READY:
      switch(form){
      case FORM_GROWING: led_pattern_single_growing(); break;
      case FORM_MIGHTY:  led_pattern_single_mighty();  break;
      case FORM_DRAGON:  led_pattern_single_dragon();  break;
      case FORM_PEGASUS: led_pattern_single_pegasus(); break;
      case FORM_TITAN:   led_pattern_single_titan();   break;
      case FORM_RISING_MIGHTY: led_pattern_single_rising_mighty(); break;
      default: ;
      }
      break;
    case STATE_FINAL_FORM_TIME:
    case STATE_WEAPON:
    case STATE_FINAL_ATTACK_TIME_BREAK:
      switch(form){
      case FORM_MIGHTY:
        if(!is_rising_limit_over && before_form == FORM_RISING_MIGHTY){
          led_pattern_deactivate_rising_mighty(led_counter_ms);
        }
        break;
      case FORM_DRAGON:
        if(!is_rising_limit_over){
          led_pattern_deactivate_rising_dragon(led_counter_ms);
        }
        break;
      case FORM_PEGASUS:
        if(!is_rising_limit_over){
          led_pattern_deactivate_rising_pegasus(led_counter_ms);
        }
        break;
      case FORM_TITAN:
        if(!is_rising_limit_over){
          led_pattern_deactivate_rising_titan(led_counter_ms);
        }
        break;
      case FORM_AMAZING_MIGHTY:
        led_pattern_deactivate_amazing_mighty(led_counter_ms);
        break;
      default: ;
      }
      break;
    default:
      ;
    }
    break;
  case STATE_SINGLE_B:
    switch(form){
    case FORM_GROWING: led_pattern_single_a(led_counter_ms); break;
    case FORM_MIGHTY:  led_pattern_single_mighty();  break;
    case FORM_DRAGON:  led_pattern_single_dragon();  break;
    case FORM_PEGASUS: led_pattern_single_pegasus(); break;
    case FORM_TITAN:   led_pattern_single_titan();   break;
    default: ;
    }
    break;
  case STATE_CHANGE_READY:
    led_pattern_change_ready(led_counter_ms);
    break;
  case STATE_ARMOR_TIME_READY:
    switch(form){
    case FORM_GROWING: led_pattern_armor_time_ready_growing(led_counter_ms); break;
    case FORM_MIGHTY:  led_pattern_armor_time_ready_mighty(led_counter_ms);  break;
    case FORM_DRAGON:  led_pattern_armor_time_ready_dragon(led_counter_ms);  break;
    case FORM_PEGASUS: led_pattern_armor_time_ready_pegasus(led_counter_ms); break;
    case FORM_TITAN:   led_pattern_armor_time_ready_titan(led_counter_ms);   break;
    case FORM_RISING_MIGHTY: led_pattern_armor_time_ready_rising_mighty(led_counter_ms); break;
    default: ;
    }
    break;
    break;
  case STATE_ARMOR_TIME:
    switch(form){
    case FORM_GROWING: led_pattern_armor_time_growing(led_counter_ms); break;
    case FORM_MIGHTY:  led_pattern_armor_time_mighty(led_counter_ms); break;
    case FORM_DRAGON:  led_pattern_armor_time_dragon(led_counter_ms); break;
    case FORM_PEGASUS: led_pattern_armor_time_pegasus(led_counter_ms); break;
    case FORM_TITAN:   led_pattern_armor_time_titan(led_counter_ms); break;
    case FORM_RISING_MIGHTY: led_pattern_armor_time_rising_mighty(led_counter_ms); break;
    default: ;
    }
    break;
  case STATE_FINISH_TIME_READY:
    switch(form){
    case FORM_GROWING: led_pattern_finish_time_ready_growing(led_counter_ms); break;
    case FORM_MIGHTY:  led_pattern_finish_time_ready_mighty(led_counter_ms);  break;
    case FORM_DRAGON:  led_pattern_finish_time_ready_dragon(led_counter_ms);  break;
    case FORM_PEGASUS: led_pattern_finish_time_ready_pegasus(led_counter_ms); break;
    case FORM_TITAN:   led_pattern_finish_time_ready_titan(led_counter_ms);   break;
    case FORM_RISING_MIGHTY: led_pattern_finish_time_ready_rising_mighty(led_counter_ms); break;
    default: ;
    }
    break;
  case STATE_FINISH_TIME:
    switch(form){
    case FORM_GROWING: led_pattern_finish_time_growing(led_counter_ms); break;
    case FORM_MIGHTY:  led_pattern_finish_time_mighty(led_counter_ms); break;
    case FORM_DRAGON:  led_pattern_finish_time_dragon(led_counter_ms); break;
    case FORM_PEGASUS: led_pattern_finish_time_pegasus(led_counter_ms); break;
    case FORM_TITAN:   led_pattern_finish_time_titan(led_counter_ms); break;
    case FORM_RISING_MIGHTY: led_pattern_finish_time_rising_mighty(led_counter_ms); break;
    default: ;
    }
    break;
  case STATE_FINAL_FORM_TIME:
    if(before_state != STATE_WEAPON && before_state != STATE_FINAL_ATTACK_TIME_BREAK){
      switch(form){
      case FORM_MIGHTY: led_pattern_final_form_time_mighty(led_counter_ms); break;
      case FORM_RISING_MIGHTY:  led_pattern_final_form_time_rising_mighty(led_counter_ms); break;
      case FORM_RISING_DRAGON:  led_pattern_final_form_time_rising_dragon(led_counter_ms); break;
      case FORM_RISING_PEGASUS: led_pattern_final_form_time_rising_pegasus(led_counter_ms); break;
      case FORM_RISING_TITAN:   led_pattern_final_form_time_rising_titan(led_counter_ms); break;
      case FORM_AMAZING_MIGHTY: led_pattern_final_form_time_amazing_mighty(led_counter_ms); break;
      default: ;
      }
    }
    break;
  case STATE_FINAL_ATTACK_TIME_BREAK:
    switch(form){
    case FORM_GROWING:  led_pattern_final_attack_time_break_growing(led_counter_ms); break;
    case FORM_MIGHTY:   led_pattern_final_attack_time_break_mighty(led_counter_ms);  break;
    case FORM_DRAGON:   led_pattern_final_attack_time_break_dragon(led_counter_ms);  break;
    case FORM_PEGASUS:  led_pattern_final_attack_time_break_pegasus(led_counter_ms); break;
    case FORM_TITAN:    led_pattern_final_attack_time_break_titan(led_counter_ms);   break;
    case FORM_RISING_MIGHTY:  led_pattern_final_attack_time_break_rising_mighty(led_counter_ms);  break;
    case FORM_RISING_DRAGON:  led_pattern_final_attack_time_break_rising_dragon(led_counter_ms);  break;
    case FORM_RISING_PEGASUS: led_pattern_final_attack_time_break_rising_pegasus(led_counter_ms); break;
    case FORM_RISING_TITAN:   led_pattern_final_attack_time_break_rising_titan(led_counter_ms);   break;
    case FORM_AMAZING_MIGHTY: led_pattern_final_attack_time_break_amazing_mighty(led_counter_ms); break;
    default: ;
    }
    break;
  default: ;
  }  

  if(led_counter < LED_COUNT_MAX){
    led_counter++;
  }

  pixels.show();
}

/*-------------------------- 音声関係 --------------------------*/
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define SOUND_VOLUME_DEFAULT 23
#define SOUND_VOLUME_LARGE   28

#define SOUND_ON                         1
#define SOUND_SINGLE_A                   2
#define SOUND_SINGLE_B                   3
#define SOUND_ARMOR_TIME_MIGHTY          4
#define SOUND_ARMOR_TIME_DRAGON          5
#define SOUND_ARMOR_TIME_PEGASUS         6
#define SOUND_ARMOR_TIME_TITAN           7
#define SOUND_ARMOR_TIME_RISING_MIGHTY   8
#define SOUND_FINISH_TIME_READY_MIGHTY   9
#define SOUND_FINISH_TIME_READY_DRAGON  10
#define SOUND_FINISH_TIME_READY_PEGASUS 11
#define SOUND_FINISH_TIME_READY_TITAN   12
#define SOUND_FINISH_TIME_MIGHTY        13
#define SOUND_FINISH_TIME_DRAGON        14
#define SOUND_FINISH_TIME_PEGASUS       15
#define SOUND_FINISH_TIME_TITAN         16
#define SOUND_CHANGE_READY              17
#define SOUND_CHANGE_MIGHTY_INIT        18
#define SOUND_CHANGE_MIGHTY_LONG        19
#define SOUND_CHANGE_DRAGON_LONG        20
#define SOUND_CHANGE_PEGASUS_LONG       21
#define SOUND_CHANGE_TITAN_LONG         22
#define SOUND_CHANGE_MIGHTY_SHORT       23
#define SOUND_CHANGE_DRAGON_SHORT       24
#define SOUND_CHANGE_PEGASUS_SHORT      25
#define SOUND_CHANGE_TITAN_SHORT        26
#define SOUND_CHANGE_RISING             27
#define SOUND_CHANGE_AMAZING            28
#define SOUND_LIMIT_GROWING             29
#define SOUND_LIMIT_RISING              30
#define SOUND_DEACTIVATE_RISING         31
#define SOUND_KEEP_RISING               32
#define SOUND_BGM_MIGHTY                33
#define SOUND_BGM_DRAGON                34
#define SOUND_BGM_PEGASUS               35
#define SOUND_BGM_TITAN                 36
#define SOUND_WEAPON_GROWING            37
#define SOUND_WEAPON_MIGHTY             38
#define SOUND_WEAPON_DRAGON             39
#define SOUND_WEAPON_PEGASUS            40
#define SOUND_WEAPON_TITAN              41
#define SOUND_WEAPON_RISING_MIGHTY      42
#define SOUND_WEAPON_RISING_DRAGON      43
#define SOUND_WEAPON_RISING_PEGASUS     44
#define SOUND_WEAPON_RISING_TITAN       45
#define SOUND_WEAPON_AMAZING_MIGHTY     46
#define SOUND_FATB_GROWING              47
#define SOUND_FATB_MIGHTY               48
#define SOUND_FATB_DRAGON               49
#define SOUND_FATB_PEGASUS              50
#define SOUND_FATB_TITAN                51
#define SOUND_FATB_RISING_MIGHTY        52
#define SOUND_FATB_RISING_DRAGON        53
#define SOUND_FATB_RISING_PEGASUS       54
#define SOUND_FATB_RISING_TITAN         55
#define SOUND_FATB_AMAZING_MIGHTY       56

#define WAIT_MS_SOUND_ARMOR_TIME  6500
#define WAIT_MS_SOUND_FINISH_TIME 1000
#define WAIT_MS_SOUND_FINAL_FORM_TIME 4000
#define WAIT_MS_SOUND_FINAL_ATTACK_TIME_BREAK 4500

unsigned long sound_wait_start_time = 0;
boolean is_armor_time_waiting = false;
boolean is_finish_time_waiting = false;
boolean is_final_form_time_waiting = false;
boolean is_final_attack_time_break_waiting = false;

uint8_t volume = SOUND_VOLUME_DEFAULT;

SoftwareSerial ss_mp3_player(2, 3); // RX, TX
DFRobotDFPlayerMini mp3_player;

void play_sound(uint8_t sound_num){
  //mp3_player.pause();
  mp3_player.playMp3Folder(sound_num);
}

void pause_sound(){
  mp3_player.pause();
}

void update_volume(uint8_t new_volume){
  mp3_player.volume(new_volume);
  volume = new_volume;
}

void control_sound(){
  // ---------- 共通処理 ----------
  unsigned long now = millis();

  // ---------- 状態遷移による音声再生 ----------
  if(prev_state != state){ // 基本的に状態遷移したときのみ音声処理
    switch(state){
    case STATE_SINGLE_A:
      switch(prev_state){
      case STATE_SINGLE_B:
        switch(form){
        case FORM_GROWING: play_sound(SOUND_SINGLE_B); break;
        case FORM_MIGHTY:
        case FORM_DRAGON:
        case FORM_PEGASUS:
        case FORM_TITAN: pause_sound(); break;
        default: ;
        }
        break;
      case STATE_CHANGE_READY:
        play_sound(SOUND_CHANGE_MIGHTY_INIT);
        break;
      default:
        pause_sound();
      }
      break;
    case STATE_SINGLE_B:
      switch(prev_state){
      case STATE_SINGLE_A:
        switch(form){
        case FORM_GROWING: play_sound(SOUND_SINGLE_A);    break;
        case FORM_MIGHTY:  play_sound(SOUND_BGM_MIGHTY);  break;
        case FORM_DRAGON:  play_sound(SOUND_BGM_DRAGON);  break;
        case FORM_PEGASUS: play_sound(SOUND_BGM_PEGASUS); break;
        case FORM_TITAN:   play_sound(SOUND_BGM_TITAN);   break;
        default: ;
        }
        break;
      default: ;
      }
      break;
    case STATE_CHANGE_READY:
      play_sound(SOUND_CHANGE_READY);
      break;
    case STATE_ARMOR_TIME_READY:
      pause_sound();
      break;
    case STATE_ARMOR_TIME:
      sound_wait_start_time = millis();
      is_armor_time_waiting = true;
      break;
    case STATE_FINISH_TIME_READY:
      switch(form){
      case FORM_GROWING:
      case FORM_MIGHTY:
      case FORM_RISING_MIGHTY: play_sound(SOUND_FINISH_TIME_READY_MIGHTY);  break;
      case FORM_DRAGON:  play_sound(SOUND_FINISH_TIME_READY_DRAGON);  break;
      case FORM_PEGASUS: play_sound(SOUND_FINISH_TIME_READY_PEGASUS); break;
      case FORM_TITAN:   play_sound(SOUND_FINISH_TIME_READY_TITAN);   break;
      default: ;
      }
      break;
    case STATE_FINISH_TIME:
      sound_wait_start_time = millis();
      is_finish_time_waiting = true;
      switch(form){
      case FORM_DRAGON:
      case FORM_PEGASUS:
      case FORM_TITAN:
        update_volume(SOUND_VOLUME_LARGE);  // ドラゴン、ペガサス、タイタンの音声が小さいので、一時的に音量を上げる
        break;
      default:
        ;
      }
      break;
    case STATE_FINAL_FORM_TIME:
      if(prev_state != STATE_WEAPON && prev_state != STATE_FINAL_ATTACK_TIME_BREAK){
        sound_wait_start_time = millis();
        is_final_form_time_waiting = true;
      }
      break;
    case STATE_WEAPON:
      switch(form){
      case FORM_GROWING: play_sound(SOUND_WEAPON_GROWING); break;
      case FORM_MIGHTY:  play_sound(SOUND_WEAPON_MIGHTY);  break;
      case FORM_DRAGON:  play_sound(SOUND_WEAPON_DRAGON);  break;
      case FORM_PEGASUS: play_sound(SOUND_WEAPON_PEGASUS); break;
      case FORM_TITAN:   play_sound(SOUND_WEAPON_TITAN);   break;
      case FORM_RISING_MIGHTY:  play_sound(SOUND_WEAPON_RISING_MIGHTY);  break;
      case FORM_RISING_DRAGON:  play_sound(SOUND_WEAPON_RISING_DRAGON);  break;
      case FORM_RISING_PEGASUS: play_sound(SOUND_WEAPON_RISING_PEGASUS); break;
      case FORM_RISING_TITAN:   play_sound(SOUND_WEAPON_RISING_TITAN);   break;
      case FORM_AMAZING_MIGHTY: play_sound(SOUND_WEAPON_AMAZING_MIGHTY); break;
      default: ;
      }
      break;
    case STATE_FINAL_ATTACK_TIME_BREAK:
      sound_wait_start_time = millis();
      is_final_attack_time_break_waiting = true;
      break;
    default: ;
    }
  }else{ // 音声再生をディレイさせるときの処理。アーマータイム、フィニッシュタイム、ファイナルフォームタイム、ファイナルアタックタイムブレイクで必要
    unsigned long now = millis();
    if(is_armor_time_waiting){
      if(now - sound_wait_start_time >= WAIT_MS_SOUND_ARMOR_TIME){
        switch(form){
        case FORM_GROWING:
        case FORM_MIGHTY:  play_sound(SOUND_ARMOR_TIME_MIGHTY);  break;
        case FORM_DRAGON:  play_sound(SOUND_ARMOR_TIME_DRAGON);  break;
        case FORM_PEGASUS: play_sound(SOUND_ARMOR_TIME_PEGASUS); break;
        case FORM_TITAN:   play_sound(SOUND_ARMOR_TIME_TITAN);   break;
        case FORM_RISING_MIGHTY: play_sound(SOUND_ARMOR_TIME_RISING_MIGHTY); break;
        default: ;
        }
        is_armor_time_waiting = false;
      }
    }else if(is_finish_time_waiting){
      if(now - sound_wait_start_time >= WAIT_MS_SOUND_FINISH_TIME){
        switch(form){
        case FORM_GROWING:
        case FORM_MIGHTY:
        case FORM_RISING_MIGHTY: play_sound(SOUND_FINISH_TIME_MIGHTY);  break;
        case FORM_DRAGON:  play_sound(SOUND_FINISH_TIME_DRAGON);  break;
        case FORM_PEGASUS: play_sound(SOUND_FINISH_TIME_PEGASUS); break;
        case FORM_TITAN:   play_sound(SOUND_FINISH_TIME_TITAN);   break;
        default: ;
        }
        is_finish_time_waiting = false;
      }
    }else if(is_final_form_time_waiting){
      if(now - sound_wait_start_time >= WAIT_MS_SOUND_FINAL_FORM_TIME){
        switch(form){
        case FORM_MIGHTY: play_sound(SOUND_CHANGE_MIGHTY_SHORT);  break;
        case FORM_RISING_MIGHTY:
        case FORM_RISING_DRAGON:
        case FORM_RISING_PEGASUS:
        case FORM_RISING_TITAN:   play_sound(SOUND_CHANGE_RISING);   break;
        case FORM_AMAZING_MIGHTY: play_sound(SOUND_CHANGE_AMAZING); break;
        default: ;
        }
        is_final_form_time_waiting = false;
      }
    }else if(is_final_attack_time_break_waiting){
      if(now - sound_wait_start_time >= WAIT_MS_SOUND_FINAL_ATTACK_TIME_BREAK){
        switch(form){
        case FORM_GROWING: play_sound(SOUND_FATB_GROWING); break;
        case FORM_MIGHTY:  play_sound(SOUND_FATB_MIGHTY);  break;
        case FORM_DRAGON:  play_sound(SOUND_FATB_DRAGON);  break;
        case FORM_PEGASUS: play_sound(SOUND_FATB_PEGASUS); break;
        case FORM_TITAN:   play_sound(SOUND_FATB_TITAN);   break;
        case FORM_RISING_MIGHTY:  play_sound(SOUND_FATB_RISING_MIGHTY);  break;
        case FORM_RISING_DRAGON:  play_sound(SOUND_FATB_RISING_DRAGON);  break;
        case FORM_RISING_PEGASUS: play_sound(SOUND_FATB_RISING_PEGASUS); break;
        case FORM_RISING_TITAN:   play_sound(SOUND_FATB_RISING_TITAN);   break;
        case FORM_AMAZING_MIGHTY: play_sound(SOUND_FATB_AMAZING_MIGHTY); break;
        default: ;
        }
        is_final_attack_time_break_waiting = false;
      }
    }
  }

  // ---------- フォームチェンジによる音声再生 ----------
  // 基本的に状態遷移を伴わない変化のための記述（ファイナルフォームタイム終了時の音声処理も兼ねる）

  if(prev_form != form){ // 基本的にフォームチェンジしたときのみ音声処理
    switch(form){
    case(FORM_GROWING):
      switch(prev_form){
      case FORM_PEGASUS: play_sound(SOUND_LIMIT_GROWING); break;
      case FORM_RISING_PEGASUS: play_sound(SOUND_LIMIT_RISING); break;
      default: ;
      }
    case(FORM_MIGHTY):
      switch(prev_form){
      case FORM_RISING_MIGHTY:
        if(state == STATE_SINGLE_A){
          play_sound(SOUND_DEACTIVATE_RISING);
        }else{
          play_sound(SOUND_LIMIT_RISING);
        }
        break;
      case FORM_TITAN:
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          play_sound(SOUND_CHANGE_MIGHTY_LONG);
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          play_sound(SOUND_CHANGE_MIGHTY_SHORT);
          break;
        }
        break;
      default:
        ;
      }
      break;
    case(FORM_DRAGON):
      switch(prev_form){
      case FORM_RISING_DRAGON:
        if(state == STATE_SINGLE_A){
          play_sound(SOUND_DEACTIVATE_RISING);
        }else{
          play_sound(SOUND_LIMIT_RISING);
        }
        break;
      case FORM_MIGHTY:
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          play_sound(SOUND_CHANGE_DRAGON_LONG);
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          play_sound(SOUND_CHANGE_DRAGON_SHORT);
          break;
        }
        break;
      default:
        ;
      }
      break;
    case(FORM_PEGASUS):
      switch(prev_form){
      case FORM_RISING_PEGASUS:
        if(state == STATE_SINGLE_A){
          play_sound(SOUND_DEACTIVATE_RISING);
        }else{
          play_sound(SOUND_LIMIT_RISING);
        }
        break;
      case FORM_DRAGON:
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          play_sound(SOUND_CHANGE_PEGASUS_LONG);
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          play_sound(SOUND_CHANGE_PEGASUS_SHORT);
          break;
        }
        break;
      default:
        ;
      }
      break;
    case(FORM_TITAN):
      switch(prev_form){
      case FORM_RISING_TITAN:
        if(state == STATE_SINGLE_A){
          play_sound(SOUND_DEACTIVATE_RISING);
        }else{
          play_sound(SOUND_LIMIT_RISING);
        }
        break;
      case FORM_PEGASUS:
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          play_sound(SOUND_CHANGE_TITAN_LONG);
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          play_sound(SOUND_CHANGE_TITAN_SHORT);
          break;
        }
        break;
      default:
        ;
      }
      break;
    default:
      ;
    }
  }
}

/*-------------------------- ユーティリティ関数 --------------------------*/

void update_state(uint8_t new_state){
  before_state = state;
  state = new_state;
}

void update_form(uint8_t new_form){
  before_form = form;
  form = new_form;
  switch(form){
  case FORM_GROWING:
  case FORM_MIGHTY:
  case FORM_DRAGON:
  case FORM_TITAN:
    is_counting_pegasus_timer = false;
    is_counting_rising_timer  = false;
    break;
  case FORM_PEGASUS:
    pegasus_timer = millis();
    is_counting_pegasus_timer = true;
    is_counting_rising_timer  = false;
    break;
  case FORM_RISING_MIGHTY:
  case FORM_RISING_DRAGON:
  case FORM_RISING_PEGASUS:
  case FORM_RISING_TITAN:
    rising_timer = millis();
    is_counting_pegasus_timer = false;
    is_counting_rising_timer = true;
    is_rising_limit_over = false;
    is_keeping_rising = false;
    break;
  default:
    ;
  }
}

/*-------------------------- セットアップ処理 --------------------------*/

void setup() {
  Serial.begin(115200);
  pinMode(SW_CENTER_PIN, INPUT_PULLUP);
  pinMode(SW_DRIVER_PIN, INPUT_PULLUP);
  pinMode(IR_RX_PIN, INPUT_PULLUP);
  pinMode(LED_COLOR_PIN, OUTPUT);

  //---------- MP3プレイヤー ----------
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
  play_sound(SOUND_ON);
  execute_led_pattern(led_pattern_power_on, 7000);
  // 起動直後は発光させないように、カウントの上限値にする
  led_counter = LED_COUNT_MAX;
}

/*-------------------------- メイン処理 --------------------------*/

void loop() {
  // ---------- 共通処理 ----------
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

  // ---------- 龍頭スイッチ処理 ----------
  sw_center = digitalRead(SW_CENTER_PIN);

  if(prev_sw_center == SW_ON){
    if(sw_center == SW_ON){ // スイッチが継続して押されているとき
      if(!is_valid_sw_center_long_press){
          sw_center_press_counter++;
      }
      if(!is_valid_sw_center_long_press && (sw_center_press_counter > WAIT_COUNT_LONG_PRESS)){
        is_valid_sw_center_long_press = true;
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          switch(form){
          case FORM_GROWING: update_state(STATE_CHANGE_READY); break;
          case FORM_MIGHTY:  update_form(FORM_DRAGON);  update_state(STATE_SINGLE_A); break;
          case FORM_DRAGON:  update_form(FORM_PEGASUS); update_state(STATE_SINGLE_A); break;
          case FORM_PEGASUS: update_form(FORM_TITAN);   update_state(STATE_SINGLE_A); break;
          case FORM_TITAN:   update_form(FORM_MIGHTY);  update_state(STATE_SINGLE_A); break;
          default:
            ;
          }
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          switch(form){
          case FORM_MIGHTY:  update_form(FORM_DRAGON);  break;
          case FORM_DRAGON:  update_form(FORM_PEGASUS); break;
          case FORM_PEGASUS: update_form(FORM_TITAN);   break;
          case FORM_TITAN:   update_form(FORM_MIGHTY);  break;
          default:
            ;
          }
          break;
        case STATE_WEAPON:
          if(form == FORM_RISING_MIGHTY){
            // 隠し要素として例外処理
            play_sound(SOUND_KEEP_RISING);
            is_counting_rising_timer = false;
            is_keeping_rising = true;
          }
          break;
        default:
          ;
        }
      }
    }else{ // スイッチが離されたとき
      if(!is_valid_sw_center_long_press){ // 短押しから離されたとき
        switch(state){
        case STATE_SINGLE_A:
          switch(form){
          case FORM_GROWING:
          case FORM_MIGHTY:
          case FORM_DRAGON:
          case FORM_PEGASUS:
          case FORM_TITAN: update_state(STATE_SINGLE_B); break;
          default:
            ;
          }
          break;
        case STATE_SINGLE_B:
          switch(form){ // フォームが変わっていないことを記録するために、同じフォームで情報をupdateする
          case FORM_GROWING: update_form(FORM_GROWING); update_state(STATE_SINGLE_A); break;
          case FORM_MIGHTY:  update_form(FORM_MIGHTY);  update_state(STATE_SINGLE_A); break;
          case FORM_DRAGON:  update_form(FORM_DRAGON);  update_state(STATE_SINGLE_A); break;
          case FORM_PEGASUS: update_form(FORM_PEGASUS); update_state(STATE_SINGLE_A); break;
          case FORM_TITAN:   update_form(FORM_TITAN);   update_state(STATE_SINGLE_A); break;
          default:
            ;
          }
          break;
        case STATE_CHANGE_READY:
          update_form(FORM_MIGHTY);
          update_state(STATE_SINGLE_A);
          break;
        case STATE_ARMOR_TIME:
        case STATE_FINISH_TIME:
          update_state(STATE_FINISH_TIME_READY);
          break;
        default:
          ;
        }
      }else{  // 長押しから離されたとき
        ;
      }

       // 短押し/長押し共通の、離されたときの処理
      if(state == STATE_WEAPON){
        update_state(STATE_FINAL_FORM_TIME);
      }

      sw_center_press_counter = 0;
      is_valid_sw_center_long_press = false;
    }
  }else{ // prev_sw_center == SW_OFF
    if(sw_center == SW_ON){ // スイッチが押されたとき
      if(state == STATE_FINAL_FORM_TIME){
          update_state(STATE_WEAPON);
      }

      if(volume != SOUND_VOLUME_DEFAULT){
        update_volume(SOUND_VOLUME_DEFAULT);
      }
    }
  }

  prev_sw_center = sw_center;

  // ---------- ドライバー連動スイッチ処理 ----------
  sw_driver = digitalRead(SW_DRIVER_PIN);

  // スイッチが押された時の処理
  if(prev_sw_driver == SW_ON){
    if(sw_driver == SW_ON){ // スイッチが継続して押されているとき
      if(!is_valid_sw_driver_long_press){
          sw_driver_press_counter++;
      }
      if(!is_valid_sw_driver_long_press && (sw_driver_press_counter > WAIT_COUNT_LONG_PRESS)){
        is_valid_sw_driver_long_press = true;
        switch(state){
        case STATE_SINGLE_A:
        case STATE_SINGLE_B:
          update_state(STATE_ARMOR_TIME_READY);
          break;
        default:
          ;
        }
      }
    }else{ // スイッチが離されたとき
      if(!is_valid_sw_driver_long_press){ // 短押しから離されたとき
        ;
      }else{  // 長押しから離されたとき
        ;
      }
      // 短押し/長押し共通の、離されたときの処理
      switch(state){
      case STATE_ARMOR_TIME_READY:
        // タイマー処理。一定時間以内にスイッチが2回押されないと、ドライバーから外されたものとみなす
        driver_timer = now;
        is_counting_driver_timer = true;
        break;
      case STATE_ARMOR_TIME:
        update_state(STATE_ARMOR_TIME_READY);
        // タイマー処理。一定時間以内にスイッチが2回押されないと、ドライバーから外されたものとみなす
        driver_timer = now;
        is_counting_driver_timer = true;
        break;
      case STATE_FINISH_TIME_READY:
        // タイマー処理。一定時間以内にスイッチが2回押されないと、ドライバーから外されたものとみなす
        driver_timer = now;
        is_counting_driver_timer = true;
        break;
      case STATE_FINISH_TIME:
        update_state(STATE_ARMOR_TIME_READY);
        // タイマー処理。一定時間以内にスイッチが2回押されないと、ドライバーから外されたものとみなす
        driver_timer = now;
        is_counting_driver_timer = true;
        break;
      case STATE_FINAL_FORM_TIME:
      case STATE_WEAPON:
      case STATE_FINAL_ATTACK_TIME_BREAK:
        switch(form){
        case FORM_RISING_MIGHTY:
          if(!is_keeping_rising){
            update_form(FORM_MIGHTY);
          }
          break;
        case FORM_RISING_DRAGON:  update_form(FORM_DRAGON);  break;
        case FORM_RISING_PEGASUS: update_form(FORM_PEGASUS); break;
        case FORM_RISING_TITAN:   update_form(FORM_TITAN);   break;
        default:
          ;  // 基本フォーム（グローイングからマイティになったときも含む）とアメイジングマイティはそのまま
        }
        update_state(STATE_SINGLE_A);
        break;
      default:
        ;
      }

      sw_driver_press_counter = 0;
      is_valid_sw_driver_long_press = false;

      if(volume != SOUND_VOLUME_DEFAULT){
        update_volume(SOUND_VOLUME_DEFAULT);
      }
    }
  }else{ // prev_sw_driver == SW_OFF
    if(sw_driver == SW_ON){ // スイッチが押されたとき
      switch(state){
      case STATE_ARMOR_TIME_READY:
        driver_counter++;
        if(driver_counter >= 2){
          is_counting_driver_timer = false;
          update_state(STATE_ARMOR_TIME);
          driver_counter = 0;
        }
        break;
      case STATE_FINISH_TIME_READY:
        driver_counter++;
        if(driver_counter >= 2){
          is_counting_driver_timer = false;
          update_state(STATE_FINISH_TIME);
          driver_counter = 0;
        }
        break;
      default:
        ;
      }
    }
  }

  prev_sw_driver = sw_driver;

  // ---------- IR信号受信処理 ----------
  ir_state = digitalRead(IR_RX_PIN);
  if(!is_counting_ir_timer && prev_ir_state == IR_OFF && ir_state == IR_ON){
    // 最初にIR信号を受信したときのみだけ処理する（チャタリング防止）
    Serial.println(F("IR Receive"));
    ir_timer = now;
    is_counting_ir_timer = true;

    switch(state){
    case STATE_FINAL_FORM_TIME:
    case STATE_FINAL_ATTACK_TIME_BREAK: // これは発生しない想定
      update_state(STATE_FINAL_ATTACK_TIME_BREAK);
      break;
    default:
      update_state(STATE_FINAL_FORM_TIME);
      switch(form){
      case FORM_GROWING: update_form(FORM_MIGHTY); break;
      case FORM_MIGHTY:  update_form(FORM_RISING_MIGHTY);  break;
      case FORM_DRAGON:  update_form(FORM_RISING_DRAGON);  break;
      case FORM_PEGASUS: update_form(FORM_RISING_PEGASUS); break;
      case FORM_TITAN:   update_form(FORM_RISING_TITAN);   break;
      case FORM_RISING_MIGHTY: update_form(FORM_AMAZING_MIGHTY); break;
      defautl: ;
      }
    }
  }
  prev_ir_state = ir_state;

  // ---------- 各種タイマー処理 ----------
  if(is_counting_driver_timer && (now - driver_timer >= LIMIT_MS_DRIVER_SW)){
    update_state(STATE_SINGLE_A);
    is_counting_driver_timer = false;
    driver_counter = 0;
  }

  if(is_counting_ir_timer && (now - ir_timer >= LIMIT_MS_INVALID_IR)){
    if(state == STATE_FINAL_ATTACK_TIME_BREAK){
      update_state(STATE_FINAL_FORM_TIME);
    }
    is_counting_ir_timer = false;
  }

  if(is_counting_pegasus_timer && (now - pegasus_timer >= LIMIT_MS_PEGASUS)){
    update_form(FORM_GROWING);
    is_counting_pegasus_timer = false;
  }

  if(is_counting_rising_timer && (now - rising_timer >= LIMIT_MS_RISING)){
    switch(form){
    case FORM_RISING_MIGHTY:  update_form(FORM_MIGHTY);  break;
    case FORM_RISING_DRAGON:  update_form(FORM_DRAGON);  break;
    case FORM_RISING_PEGASUS: update_form(FORM_GROWING); break;
    case FORM_RISING_TITAN:   update_form(FORM_TITAN);   break;
    default: ;
    }
    is_rising_limit_over = true;
    is_counting_rising_timer = false;
  }

  // ---------- 音声処理 ----------
  control_sound();

  // ---------- 発光処理 ----------
  control_led();

  delay(LOOP_INTERVAL_MS);
}
