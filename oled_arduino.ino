#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

unsigned long newtime;
unsigned long time10;
static const unsigned char PROGMEM logo_bmp[] =
{ B11111111, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};


int close_pin = 4;
int open_pin = 5;
int ligth_pin = 7;

int close_sunblind_time = 22;//s
int close_sunblind_time_run = close_sunblind_time;//s

int ligth_num = 350; // 当亮度由黑到亮的阈值
int dark_num = 1000; // 当亮度由亮到黑的阈值
int ligth_new = 0;
int ligth_staus = 2; // 之前亮度阶段  1 亮  2 中等  3 暗

int auto_status = 1; // 是否处于自动状态  1处于   0不处于

int t = 0;
int lianzi_status = 1;  // 表示帘子状态 1 为打开  0为关闭
int close_run_status = 0; //  电机关闭方向是否通电  0未通电
int open_run_status = 0;  //  电机打开方向是否通电  0未通电
int dpin2_status = 0; // 末尾限位开关状态  0 表示未碰触  1表示碰触
int fanxiang = 0;
int wait_ti = 5;


int inter_num = 0;

void time1_callback() {
   display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("g:"));
  ligth_new = analogRead(ligth_pin);
  display.print(ligth_new);
  display.print(F(" s:"));
  display.print(ligth_staus);

  display.print(F("  :"));
  display.print(ligth_num);
  display.print(F(":"));
  display.print(dark_num);
  
  display.setCursor(0, 10);
  display.print(F("onoff:"));
  display.print(dpin2_status);
  display.print(F(" lian:"));
  display.print(lianzi_status);
  display.print(F(" "));
  display.print(close_sunblind_time_run);
  display.print(F(":"));
  display.print(close_sunblind_time);

  display.setCursor(0, 20);
  display.print(F("clos:"));
  display.print(close_run_status);
  display.print(F(" open:"));
  display.print(open_run_status); 
  display.print(F(" int:"));
  display.print(inter_num); 

  display.display();      // Show initial text
  Serial.println(F("ligth_new"));
  Serial.println(ligth_new);

}


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  display.clearDisplay();

  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(2000);

  pinMode(2, INPUT_PULLUP);
  pinMode(close_pin, OUTPUT);
  pinMode(open_pin, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(close_pin, LOW);
  digitalWrite(open_pin, LOW);
  digitalWrite(13, LOW);
  close_run_status = 0;
  open_run_status = 0;

display.setTextSize(1);
 
display.setTextColor(WHITE);


// 设置外部中断
attachInterrupt(0, interrupt0, CHANGE);
dpin2_status = digitalRead(2);

time1_callback();

if (dpin2_status != 0) {
  close_sunblind_time_run = 0;
  lianzi_status = 0;
} else {
  close_sunblind_operate();
}

  newtime = 0;
  time10 = 0;
}

void interrupt0() {
  inter_num++;
  dpin2_status = digitalRead(2);
  if (dpin2_status != 0 && close_run_status == 1) {
    digitalWrite(close_pin, LOW);
    close_run_status = 0;
    lianzi_status = 0;
    digitalWrite(13, HIGH);
    close_sunblind_time_run = 0;
  }
  if (dpin2_status == 0 && lianzi_status == 0) {
    lianzi_status = 1;
    digitalWrite(13, LOW);
  }
}

void close_sunblind_operate() {
  // 当 帘子状态为打开 1    关闭驱动未通电   末尾碰触开关未碰触时   进行关闭操作
    if (lianzi_status == 1 && close_run_status == 0 && dpin2_status == 0) {
        digitalWrite(open_pin, LOW);
        digitalWrite(close_pin, HIGH);
        close_run_status = 1;
        open_run_status = 0;
    }
}

void open_sunblind_operate() {
     // 打开驱动未通电    打开时间未超时
    if ( open_run_status == 0 && close_sunblind_time_run < close_sunblind_time) {
        digitalWrite(close_pin, LOW);
        digitalWrite(open_pin, HIGH);
        close_run_status = 0;
        open_run_status = 1;
    }
}

void stop_sunblind_operate() {
     digitalWrite(close_pin, LOW);
     digitalWrite(open_pin, LOW);
     close_run_status = 0;
     open_run_status = 0;
}

void loop() {

  newtime = millis();

  if ((newtime - time10) > 1000) {
    time10 = newtime;
    time1_callback();
    dpin2_status = digitalRead(2);
    if (open_run_status) {
      close_sunblind_time_run = close_sunblind_time_run + 1;
    }
    if (close_sunblind_time_run > close_sunblind_time && open_run_status == 1) { // 当超过打开窗帘时间时，关闭驱动
      stop_sunblind_operate();
    }

    if (auto_status) {
       int status_ligth = 0;
       if (ligth_new < ligth_num) {
         status_ligth = 1;
       } else if (ligth_new < dark_num) {
          status_ligth = 2;
       } else {
          status_ligth = 3;
       }
       if (status_ligth != ligth_staus && ligth_staus != 0) {
          if (status_ligth == 3 && ligth_staus == 2) { // 
            close_sunblind_operate(); // 关闭窗帘
          }
          if (status_ligth == 1 && ligth_staus == 2) {
            open_sunblind_operate(); // 打开窗帘
          } 
       }
       if (status_ligth == 1 && dpin2_status == 1 && close_sunblind_time_run == 0) {
          open_sunblind_operate(); // 打开窗帘
       }
       if (status_ligth == 3 && dpin2_status == 0 && close_sunblind_time_run >= close_sunblind_time) {
          close_sunblind_operate(); // 关闭窗帘
       }
       if ((status_ligth == 3 && ligth_staus == 1) || (status_ligth == 1 && ligth_staus == 3)) {
          ligth_staus = 2;
       } else {
         ligth_staus = status_ligth;
       }
       
    }
  }

}

