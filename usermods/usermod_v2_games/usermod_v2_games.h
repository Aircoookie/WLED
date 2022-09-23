#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//inspired by https://noobtuts.com/cpp/2d-pong-game
typedef struct PongBall {
  float x;// = SEGMENT.virtualWidth() / 2;
  float y;// = SEGMENT.virtualHeight() / 2;
  float dir_x;// = -1;
  float dir_y;// = 0;
  uint8_t width;// = 8;
  uint8_t height;// = 8;
  float speed;// = 1;
  uint8_t scoreLeft, scoreRight;
  uint32_t color;
  void move() {
    x += dir_x * speed;
    y += dir_y * speed;
  }
  void vec2_norm() {
    // sets a vectors length to 1 (which means that x + y == 1)
    float length = sqrt((dir_x * dir_x) + (dir_y * dir_y));
    if (length != 0.0f) {
        length = 1.0f / length;
        dir_x *= length;
        dir_x *= length;
    }
  }
  void hit() {
    // hit left wall?
    if (x <= 0) {
        dir_x = fabs(dir_x); // force it to be positive
        // scoreLeft++;
        // if (scoreLeft>9) scoreLeft = 0;
    }
    // hit right wall?
    if (x + width-1 >= SEGMENT.virtualWidth()-1) {
        dir_x = -fabs(dir_x); // force it to be negative
        // scoreRight++;
        // if (scoreRight>9) scoreRight = 0;
    }
    // hit top wall?
    if (y <= 0) {
        dir_y = fabs(dir_y); // force it to be positive
    }
    // hit bottom wall? 
    if (y + height-1 >= SEGMENT.virtualHeight()-1) {
        dir_y = -fabs(dir_y); // force it to be negative
    }
  }
  bool hit(PongBall *other) {
    if (x < other->x + other->width && 
      x >= other->x &&
      y < other->y + other->height &&
      y >= other->y) {
      // set fly direction depending on where it hit the racket
      // (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
      float t = ((y - other->y) / other->height) - 0.5f;
      dir_x = fabs(dir_x); // force it to be positive
      dir_y = t;
      return true;
    }
    else
      return false;
  }
} pongBall;


//effect functions
uint16_t mode_pongGame(void) { 

  uint16_t dataSize = 3 * sizeof(pongBall);
  if (!SEGENV.allocateData(dataSize)) {SEGMENT.fill(SEGCOLOR(0)); return 350;} //mode_static(); //allocation failed

  PongBall* ball = reinterpret_cast<PongBall*>(SEGENV.data);
  PongBall* racket_left = reinterpret_cast<PongBall*>(SEGENV.data + sizeof(pongBall));
  PongBall* racket_right = reinterpret_cast<PongBall*>(SEGENV.data + 2* sizeof(pongBall));

  // static uint16_t previousX, previousY;

  uint16_t vW = SEGMENT.virtualWidth();
  uint16_t vH = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    ball->width = 1;
    ball->height = 1;
    ball->x = vW/2;
    ball->y = vH/2;
    ball->dir_x = -0.1;
    ball->dir_y = 0.18;
    ball->color = BLUE;
    // ball->speed = 1;//SEGMENT.speed/30.0;

    racket_left->width = 1;
    racket_left->height = vH/4;
    racket_left->x = 0;
    racket_left->y = vH/2 - racket_left->height/2;
    racket_left->dir_y = 0.18;
    racket_left->speed = 1;
    racket_left->color = BLUE;

    racket_right->width = 1;
    racket_right->height = vH/4;
    racket_right->x = vW - 1;
    racket_right->y = vH/2 - racket_right->height/2;
    racket_right->dir_y = -0.18;
    racket_right->speed = 1;
    racket_right->color = BLUE;
  }

  ball->speed = SEGMENT.speed/30.0;

  SEGMENT.fill(BLACK);

  ball->move();
  racket_left->move();
  racket_right->move();


  if (ball->hit(racket_left)) {
    ball->scoreLeft++;
    if (ball->scoreLeft>9) ball->scoreLeft = 0;

    racket_left->color = RED;
  } else {
    racket_left->color = BLUE;
  }

  if (ball->hit(racket_right)) {
    ball->scoreRight++;
    if (ball->scoreRight>9) ball->scoreRight = 0;

    racket_right->color = RED;
  } else {
    racket_right->color = BLUE;
  }

  ball->hit();
  racket_left->hit();
  racket_right->hit();

  ball->vec2_norm();
  racket_left->vec2_norm();
  racket_right->vec2_norm();

  SEGMENT.setPixelColorXY((uint16_t)ball->x, (uint16_t)ball->y, ball->color);

  SEGMENT.drawLine(0, racket_left->y, 0, racket_left->y + racket_left->height-1, racket_left->color);
  SEGMENT.drawLine(vW-1, racket_right->y, vW-1, racket_right->y + racket_right->height-1, racket_right->color);

  for (int i=0; i<vH; i+=2) {
    SEGMENT.setPixelColorXY(vW/2, i, BLUE);
  }

  char tempString[2] = "";
  sprintf(tempString, "%1d%1d", ball->scoreRight, ball->scoreLeft);
  SEGMENT.drawCharacter(tempString[0], vW/2-5, -2, 5, 8, BLUE);
  SEGMENT.drawCharacter(tempString[1], vW/2+2, -2, 5, 8, BLUE);

  return FRAMETIME;
}

static const char _data_FX_MODE_PONGGAME[] PROGMEM = "🎮 Pong@!;!;!;2d";

//https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
#define MPU_ADDR 0x68 // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

uint16_t mode_gyro(void) { 
  SEGMENT.fill(BLACK);

  uint8_t y = 0;

  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (accelerometer_x+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);
  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (accelerometer_y+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);
  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (accelerometer_z+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);
  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (gyro_x+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);
  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (gyro_y+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);
  SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (gyro_z+INT16_MAX)/(2*INT16_MAX), y+=2, BLUE);

  return FRAMETIME;
}
static const char _data_FX_MODE_GYRO[] PROGMEM = "🎮 Gyro@!;!;!;2d";

#ifndef FLD_PIN_SCL
  #define FLD_PIN_SCL i2c_scl
#endif
#ifndef FLD_PIN_SDA
  #define FLD_PIN_SDA i2c_sda
#endif


class GamesUsermod : public Usermod {
  private:
    bool enabled = true;
    int8_t ioPin[5] = {FLD_PIN_SCL, FLD_PIN_SDA, -1, -1, -1};        // I2C pins: SCL, SDA
    unsigned long lastUMRun = millis();

  public:
    //Functions called by WLED

    void setup() {
      bool isHW;
      PinOwner po = PinOwner::UM_Unspecified;
      uint8_t hw_scl = i2c_scl<0 ? HW_PIN_SCL : i2c_scl;
      uint8_t hw_sda = i2c_sda<0 ? HW_PIN_SDA : i2c_sda;
      if (ioPin[0] < 0 || ioPin[1] < 0) {
        ioPin[0] = hw_scl;
        ioPin[1] = hw_sda;
      }
      isHW = (ioPin[0]==hw_scl && ioPin[1]==hw_sda);
      if (isHW) po = PinOwner::HW_I2C;  // allow multiple allocations of HW I2C bus pins
      PinManagerPinType pins[2] = { {ioPin[0], true }, { ioPin[1], true } };
      if (!pinManager.allocateMultiplePins(pins, 2, po)) { enabled = false; return; }
      // PinManagerPinType pins[2] = { { i2c_scl, true }, { i2c_sda, true } };
      // if (!pinManager.allocateMultiplePins(pins, 2, PinOwner::HW_I2C)) { enabled = false; return; }
      Wire.begin();
      Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
      Wire.write(0x6B); // PWR_MGMT_1 register
      Wire.write(0); // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);

      strip.addEffect(255, &mode_pongGame, _data_FX_MODE_PONGGAME);
      strip.addEffect(255, &mode_gyro, _data_FX_MODE_GYRO);
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
    }

    void loop() {
      if (!enabled || (strip.isUpdating() && (millis() - lastUMRun < 2))) return;   // be nice, but not too nice
      lastUMRun = millis();                    // update time keeping

      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
      Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
      Wire.requestFrom(MPU_ADDR, 7*2); // request a total of 7*2=14 registers
      
      // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
      accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
      accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
      accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
      temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
      gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
      gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
      gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
    }

    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
    }

    void readFromJsonState(JsonObject& root)
    {
      userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
    }

    void addToConfig(JsonObject& root)
    {
      // JsonObject top = root.createNestedObject("gamesUsermod");
    }

    bool readFromConfig(JsonObject& root)
    {

      JsonObject top = root["gamesUsermod"];

      bool configComplete = !top.isNull();

      return configComplete;
    }

    void handleOverlayDraw()
    {
    }

    uint16_t getId()
    {
      return USERMOD_ID_GAMES;
    }
};