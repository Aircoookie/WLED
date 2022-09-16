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


//effect function
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


//class name. Use something descriptive and leave the ": public Usermod" part :)
class GamesUsermod : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
        strip.addEffect(255, &mode_pongGame, _data_FX_MODE_PONGGAME);
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
    }

    void loop() {
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
      JsonObject top = root.createNestedObject("gamesUsermod");
    }

    bool readFromConfig(JsonObject& root)
    {

      JsonObject top = root["exampleUsermod"];

      bool configComplete = !top.isNull();

      return configComplete;
    }

    void handleOverlayDraw()
    {
    }

    uint16_t getId()
    {
      return USERMOD_ID_EXAMPLE;
    }
};