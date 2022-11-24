/*
  Games usermod by ewowi, september 2022

  Contains:
    - mode_pongGame
    - Depending on USERMOD_MPU6050_IMU
      - mode_IMUTest (shows IMU values only if WLED_DEBUG)
    - class Frame3D and struct Voxel
    - mode_3DIMUCube (uses class Frame3D to show a rotating cube, if USERMOD_MPU6050_IMU then IMU used for rotation)
    - class GamesUsermod (Add the modes/effects and initiates IMU)
*/

#pragma once

#include "wled.h"

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

  char tempString[4] = { '\0' };
  snprintf(tempString, 4, "%1d%1d", ball->scoreRight, ball->scoreLeft);
  SEGMENT.drawCharacter(tempString[0], vW/2-5, -2, 5, 8, BLUE);
  SEGMENT.drawCharacter(tempString[1], vW/2+2, -2, 5, 8, BLUE);

  return FRAMETIME;
}

static const char _data_FX_MODE_PONGGAME[] PROGMEM = "🎮 Pong@!;!;!;2d";

//https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/

#ifdef USERMOD_MPU6050_IMU
MPU6050Driver *IMU = nullptr;
uint16_t mode_IMUTest(void) { 
  SEGMENT.fill(BLACK);

  uint8_t y = 0;

  if (IMU != nullptr) {
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aa.x+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aa.y+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aa.z+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaReal.x+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaReal.y+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaReal.z+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->gy.x+1024)/(2*1024), y+=1, RED);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->gy.y+1024)/(2*1024), y+=1, RED);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->gy.z+1024)/(2*1024), y+=1, RED);

    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaWorld.x+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaWorld.y+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->aaWorld.z+INT16_MAX)/(2*INT16_MAX), y+=1, BLUE);
    
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->ypr[0]* 180/M_PI+180)/(2*180), y+=1, RED);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->ypr[1]* 180/M_PI+180)/(2*180), y+=1, RED);
    SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() * (IMU->ypr[2]* 180/M_PI+180)/(2*180), y+=1, RED);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_IMUTest[] PROGMEM = "🎮 IMUTest@;;;2d";

#endif

//WLEDMM 3D to 2D mapping
struct Voxel {
  float x;
  float y;
  float z;
  uint32_t col;
};

//https://xem.github.io/articles/projection.html
class Frame3D {
  private:
    std::vector<Voxel> points;
    void rotate(float a, float b, float angle, float &x, float &y) {
      x = cosf(angle) * a - sinf(angle) * b;
      y = sinf(angle) * a + cosf(angle) * b;
    }
    float yaw;
    float pitch; 
    float roll;
  public:
    Frame3D(float yaw1, float pitch1, float roll1) {
      points.clear();
      yaw = yaw1;
      pitch = pitch1;
      roll = roll1;
    }
    ~Frame3D() {
      std::sort(points.begin(), points.end(), [](const Voxel &lhs, const Voxel &rhs) {return lhs.z > rhs.z;});
      for(size_t i = 0; i < points.size(); ++i) {
        float w = 0.5;//SEGMENT.virtualWidth()/2;
        float h = 0.5;//SEGMENT.virtualHeight()/2;
        float perspective = SEGMENT.intensity / 64.0;
        if(points[i].z > 0) {
          float projX, projY;
          projX = w+points[i].x/points[i].z*perspective;
          projY = h+points[i].y/points[i].z*perspective;
          SEGMENT.setPixelColorXY(projX, projY, points[i].col, false); //no aa
        }
      }
    }
    void setPixelColorXYZ(Voxel voxel) {
      float camx = 0;
      float camy = 0;
      float camz = -6;
      rotate(voxel.x,voxel.z,yaw, voxel.x, voxel.z); // Camera yaw
      rotate(voxel.y,voxel.z,pitch, voxel.y, voxel.z); // Camera pitch
      rotate(voxel.x,voxel.y,roll, voxel.x, voxel.y); // Camera roll
      voxel.x -= camx;
      voxel.y -= camy;
      voxel.z -= camz;
      points.push_back(voxel);
    }
    void drawLineXYZ(Voxel from, Voxel to, uint32_t col) {
      for (float x=MIN(from.x, to.x); x<=MAX(from.x, to.x); x+=.05)
        for (float y=MIN(from.y, to.y); y<=MAX(from.y, to.y); y+=.05)
          for (float z=MIN(from.z, to.z); z<=MAX(from.z, to.z); z+=.05)
            setPixelColorXYZ({x, y, z, col});
    }
};

uint16_t mode_3DIMUCube(void) { 
  SEGMENT.fill(BLACK);

  float yaw = 0;
  float pitch = 0; 
  float roll = 0;

  #ifdef USERMOD_MPU6050_IMU
    if (IMU != nullptr) {
      yaw = -IMU->ypr[0];
      pitch = IMU->ypr[1];
      roll = IMU->ypr[2];
    }
  #else
    //simulate rotation
    yaw = (fmod(SEGENV.call, 360)-180) / (180/M_PI); //-180 .. 180
    pitch = yaw;
    roll = yaw;
  #endif

  Frame3D frame3D = Frame3D(yaw, pitch, roll);

  Voxel leftbottomback = {-1,-1,-1};
  Voxel rightbottomback = {1,-1,-1};
  Voxel lefttopback = {-1,1,-1};
  Voxel righttopback = {1,1,-1};
  Voxel leftbottomfront = {-1,-1,1};
  Voxel rightbottomfront = {1,-1,1};
  Voxel lefttopfront = {-1,1,1};
  Voxel righttopfront = {1,1,1};
  frame3D.drawLineXYZ(leftbottomback, rightbottomback, SEGMENT.color_from_palette(255/12, false, true, 0));
  frame3D.drawLineXYZ(leftbottomback, lefttopback, SEGMENT.color_from_palette(255/12*2, false, true, 0));
  frame3D.drawLineXYZ(rightbottomback, righttopback, SEGMENT.color_from_palette(255/12*3, false, true, 0));
  frame3D.drawLineXYZ(lefttopback, righttopback, SEGMENT.color_from_palette(255/12*4, false, true, 0));

  frame3D.drawLineXYZ(leftbottomfront, leftbottomback, SEGMENT.color_from_palette(255/12*9, false, true, 0));
  frame3D.drawLineXYZ(rightbottomfront, rightbottomback, SEGMENT.color_from_palette(255/12*10, false, true, 0));
  frame3D.drawLineXYZ(lefttopfront, lefttopback, SEGMENT.color_from_palette(255/12*11, false, true, 0));
  frame3D.drawLineXYZ(righttopfront, righttopback, SEGMENT.color_from_palette(255/12*12, false, true, 0));

  frame3D.drawLineXYZ(leftbottomfront, rightbottomfront, SEGMENT.color_from_palette(255/12*5, false, true, 0));
  frame3D.drawLineXYZ(leftbottomfront, lefttopfront, SEGMENT.color_from_palette(255/12*6, false, true, 0));
  frame3D.drawLineXYZ(rightbottomfront, righttopfront, SEGMENT.color_from_palette(255/12*7, false, true, 0));
  frame3D.drawLineXYZ(lefttopfront, righttopfront, SEGMENT.color_from_palette(255/12*8, false, true, 0));

  return FRAMETIME;
}
static const char _data_FX_MODE_3DIMUCube[] PROGMEM = "🎮 3DIMUCube@,Perspective;!;!;,pal=1,2d"; //random cycle

class GamesUsermod : public Usermod {
  private:

  public:

    void setup() {
      strip.addEffect(255, &mode_pongGame, _data_FX_MODE_PONGGAME);
      #ifdef USERMOD_MPU6050_IMU
        IMU = (MPU6050Driver *)usermods.lookup(USERMOD_ID_IMU);
        #ifdef WLED_DEBUG
          strip.addEffect(255, &mode_IMUTest, _data_FX_MODE_IMUTest);
        #endif
      #endif
      strip.addEffect(255, &mode_3DIMUCube, _data_FX_MODE_3DIMUCube); //works also without IMU
    }

    void connected() {
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