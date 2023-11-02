#include "wled.h"
#include <FastLED.h>
#include <const.h>


#define BRAKE_PEDAL 19
#define DOOR_OPEN 5
#define LEFT_INDICATOR 23
#define RIGHT_INDICATOR 18
#define BRAKE_SEGMENT 0
#define LEFT_SEGMENT 1
#define FRONT_SEGMENT 2
#define RIGHT_SEGMENT 3

#define RIGHT_SEGMENT_START 0
#define RIGHT_SEGMENT_END 200
#define FRONT_SEGMENT_START 200
#define FRONT_SEGMENT_END 240
#define LEFT_SEGMENT_START 240
#define LEFT_SEGMENT_END 440
#define BRAKE_SEGMENT_START 440
#define BRAKE_SEGMENT_END 480

class Subaru : public Usermod {
  private:
    bool paused = false;
    bool flashingRed = false;
    bool welcomeLights = false;
    bool left_indicator_on = false;
    bool right_indicator_on = false;
    bool brake_pedal_pressed = false;
    bool left_indicator_previous_state = false;
    bool right_indicator_previous_state = false;
    bool brake_pedal_previous_state = false;
    bool override = false;
    bool door_is_open = false;



    unsigned long lastTime = 0;
    unsigned long currentTime = 0;
    unsigned long period = 10000;

    uint8_t previousBrightness;
    uint8_t previousEffect;
    uint8_t previousPalette;
    
    //Declare Segments
    Segment brake_segment_cache;
 
    Segment left_segment_cache;

    Segment right_segment_cache;

    Segment front_segment_cache;

    Segment brake_segment;
 
    Segment left_segment;

    Segment right_segment;

    Segment front_segment;
    
    struct PreviousState {
      uint8_t mode;
      uint8_t palette;
      uint8_t intensity;
      uint32_t * colors;
      uint8_t speed;
      uint8_t opacity;
      uint8_t cct;
    } previousLeftState, previousRightState, previousBrakeState, previousFrontState;
    /**
     * Loop through previousLeftState, previousRightState, previousBrakeState, previousFrontState and initialize them with empty values
    */


    /**
     * Create a new LED strip with 240 LEDs and a output pin of 3
    */


    void updateSegment(int seg, int mode, uint32_t color1, uint32_t color2, uint32_t color3, int speed, uint8_t palette = 0) {
      /** Return if seg is not defined */

      strip.setMode(seg, mode);
      strip.getSegments()[seg].setColor(0, color1);
      strip.getSegments()[seg].setColor(1, color2);
      strip.getSegments()[seg].setColor(2, color3);
      strip.getSegments()[seg].setPalette(palette);

      strip.getSegments()[seg].speed = speed;

   
      strip.trigger();
    
    }

    void restorePreviousState(int segment, const Segment& prevState) {
      updateSegment(segment, prevState.mode, prevState.colors[0], prevState.colors[1], prevState.colors[2], prevState.speed, prevState.palette);
    }
    void select(int segment){
      right_segment.setOption(SEG_OPTION_SELECTED, 0);
      left_segment.setOption(SEG_OPTION_SELECTED, 0);
      brake_segment.setOption(SEG_OPTION_SELECTED, 0);
      front_segment.setOption(SEG_OPTION_SELECTED, 0);
      strip.getSegment(segment).setOption(SEG_OPTION_SELECTED, 1);
    }

    void selectAll(){
      right_segment.setOption(SEG_OPTION_SELECTED, 1);
      left_segment.setOption(SEG_OPTION_SELECTED, 1);
      brake_segment.setOption(SEG_OPTION_SELECTED, 1);
      front_segment.setOption(SEG_OPTION_SELECTED, 1);
    }

     void setupSegments(){
    
      strip.setSegment(BRAKE_SEGMENT, BRAKE_SEGMENT_START, BRAKE_SEGMENT_END, 1, 0, 0);
      strip.setSegment(LEFT_SEGMENT, LEFT_SEGMENT_START, LEFT_SEGMENT_END, 1, 0, 0);
      strip.setSegment(FRONT_SEGMENT, FRONT_SEGMENT_START, FRONT_SEGMENT_END, 1, 0, 0);
      strip.setSegment(RIGHT_SEGMENT, RIGHT_SEGMENT_START, RIGHT_SEGMENT_END, 1, 0, 0);
      
      right_segment = right_segment_cache = strip.getSegment(RIGHT_SEGMENT);
      left_segment = left_segment_cache = strip.getSegment(LEFT_SEGMENT);
      brake_segment = brake_segment_cache = strip.getSegment(BRAKE_SEGMENT);
      front_segment = front_segment_cache = strip.getSegment(FRONT_SEGMENT);


      right_segment.setOption(SEG_OPTION_ON, 1);
      right_segment.setOption(SEG_OPTION_SELECTED, 1);
      left_segment.setOption(SEG_OPTION_ON, 1);
      left_segment.setOption(SEG_OPTION_SELECTED, 1);
      brake_segment.setOption(SEG_OPTION_ON, 1);
      brake_segment.setOption(SEG_OPTION_SELECTED, 1);
      front_segment.setOption(SEG_OPTION_ON, 1);
      front_segment.setOption(SEG_OPTION_SELECTED, 1);

      /** Set the name/title of segments */
      right_segment.name = (char*)"Right Strip";
      left_segment.name = (char*)"Left Strip";
      brake_segment.name = (char*)"Brake Strip";
      front_segment.name = (char*)"Front Strip";

    }

    unsigned long lastLeftIndicatorTime = 0;
    uint16_t currentLeftIndicatorIndex = 0;
    bool fadeLeftIndicatorDirection = true; // true for fading in, false for fading out
    byte currentLeftIndicatorBrightness = 0;
    uint16_t waitLeftIndicatorTime = 50; // time to wait between updates in ms
    uint16_t fadeLeftIndicatorAmount = 5; // amount to change brightness each update
    uint8_t indicator_palette[16] = {0, 0, 255, 0, 255, 165, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0};
    
    bool brake_effect_override;
    unsigned long last_brake_press_time = 0;
    unsigned long brake_effect_duration = 1000;

    bool left_effect_override;
    unsigned long last_left_on_time = 0;
    unsigned long left_effect_duration = 5000;

    bool right_effect_override;
    unsigned long last_right_on_time = 0;
    unsigned long right_effect_duration = 5000;

 

  public:
    void setup() {
      pinMode(LEFT_INDICATOR, INPUT_PULLDOWN);
      pinMode(RIGHT_INDICATOR, INPUT_PULLDOWN);
      pinMode(BRAKE_PEDAL, INPUT_PULLDOWN);
      pinMode(DOOR_OPEN, INPUT_PULLDOWN);
      setupSegments();
      
    }
    /**
     * A method that checks if each segment is assigned to the correct LED start and end. If a change/discrepancy is detected, run setup()
    */
    void checkSegmentIntegrity() {
        static bool previousIntegrityCheckResult = true;

        bool integrityCheckResult = true;

        // Check if each segment is assigned to the correct LED start and end
        integrityCheckResult &= strip.getSegment(BRAKE_SEGMENT).start == BRAKE_SEGMENT_START;
        integrityCheckResult &= strip.getSegment(BRAKE_SEGMENT).stop == BRAKE_SEGMENT_END;
        integrityCheckResult &= strip.getSegment(LEFT_SEGMENT).start == LEFT_SEGMENT_START;
        integrityCheckResult &= strip.getSegment(LEFT_SEGMENT).stop == LEFT_SEGMENT_END;
        integrityCheckResult &= strip.getSegment(FRONT_SEGMENT).start == FRONT_SEGMENT_START;
        integrityCheckResult &= strip.getSegment(FRONT_SEGMENT).stop == FRONT_SEGMENT_END;
        integrityCheckResult &= strip.getSegment(RIGHT_SEGMENT).start == RIGHT_SEGMENT_START;
        integrityCheckResult &= strip.getSegment(RIGHT_SEGMENT).stop == RIGHT_SEGMENT_END;

        if (!integrityCheckResult && previousIntegrityCheckResult) {
            Serial.println("Detected a change in segment integrity. Running setup() again.");
            setupSegments();
        }

        previousIntegrityCheckResult = integrityCheckResult;
    }
    void printSegmentDetails(){

      /** Assign strip.getSegment(LEFT_SEGMENT) to a constant **/
      const Segment& left_segment = strip.getSegment(LEFT_SEGMENT);
      // const Segment& right_segment = strip.getSegment(RIGHT_SEGMENT);
      // const Segment& brake_segment = strip.getSegment(BRAKE_SEGMENT);

      /**
       * Print the current palette and mode of each segment every 5 seconds
       * */
      
      // Serial.println("------");
      // Serial.print("Brake Segment Mode: ");
      // Serial.println(brake_segment.mode);
      // Serial.print("Brake Segment Palette: ");
      // Serial.println(brake_segment.palette);
      // Serial.print("Brake Segment Intensity: ");
      // Serial.println(brake_segment.intensity);
      // Serial.print("Brake Segment Speed: ");
      // Serial.println(brake_segment.speed);
      // Serial.print("Brake Segment Opacity: ");
      // Serial.println(brake_segment.opacity);
      // Serial.print("Brake Segment CCT: ");
      // Serial.println(brake_segment.cct);
      // Serial.print("Brake Segment Color 1: ");
      // Serial.println(brake_segment.colors[0]);
      // Serial.print("Brake Segment Color 2: ");
      // Serial.println(brake_segment.colors[1]);
      // Serial.print("Brake Segment Color 3: ");
      // Serial.println(brake_segment.colors[2]);

      /**
       * Check if the previousLeftState is set.
       * If it is, print the previous state of the segment
       * */

      
      Serial.println("++++++ LEFT SEGMENT PREVIOUS COLORS ++++++");
      Serial.print("LSEG Prev Palette: ");
      Serial.println(left_segment_cache.palette);
      Serial.print("LSEG Prev Color 1: ");
      Serial.println(left_segment_cache.colors[0]);
      Serial.print("LSEG Prev Color 2: ");
      Serial.println(left_segment_cache.colors[1]);
      Serial.print("LSEG Prev Color 3: ");
      Serial.println(left_segment_cache.colors[2]);
    

 

      Serial.println("++++++ LEFT SEGMENT CURRENT COLORS ++++++");
      Serial.print("Left Segment Mode: ");
      Serial.println(left_segment.mode);
      Serial.print("Left Segment Palette: ");
      Serial.println(left_segment.palette);
      Serial.print("Left Segment Color 1: ");
      Serial.println(left_segment.colors[0]);
      Serial.print("Left Segment Color 2: ");
      Serial.println(left_segment.colors[1]);
      Serial.print("Left Segment Color 3: ");
      Serial.println(left_segment.colors[2]);
      // Serial.println("------");
      // Serial.print("Right Segment Mode: ");
      // Serial.println(right_segment.mode);
      // Serial.print("Right Segment Palette: ");
      // Serial.println(right_segment.palette);
      // Serial.print("Right Segment Intensity: ");
      // Serial.println(right_segment.intensity);
      // Serial.print("Right Segment Speed: ");
      // Serial.println(right_segment.speed);
      // Serial.print("Right Segment Opacity: ");
      // Serial.println(right_segment.opacity);
      // Serial.print("Right Segment CCT: ");
      // Serial.println(right_segment.cct);
      // Serial.print("Right Segment Color 1: ");
      // Serial.println(right_segment.colors[0]);
      // Serial.print("Right Segment Color 2: ");
      // Serial.println(right_segment.colors[1]);
      // Serial.print("Right Segment Color 3: ");
      // Serial.println(right_segment.colors[2]);
      // Serial.println("------");

    }
    void printDetailsPeriodically() {
        static unsigned long lastPrintTime = 0;
        //static bool previousStates[60] = {false}; // initialize all to LOW
        const unsigned long printInterval = 5000; // 5 seconds in milliseconds
        if (millis() - lastPrintTime >= printInterval) {
            printSegmentDetails();
            lastPrintTime = millis();
        }
    }



    void loop() {

      printDetailsPeriodically();


      //Get all relevant segments

      //Set the state of PINS

      right_indicator_on = (digitalRead(RIGHT_INDICATOR) == HIGH);
      left_indicator_on = (digitalRead(LEFT_INDICATOR) == HIGH);
      brake_pedal_pressed = (digitalRead(BRAKE_PEDAL) == HIGH);
      door_is_open = (digitalRead(DOOR_OPEN) == HIGH);

      const bool brake_state_change = brake_pedal_previous_state != brake_pedal_pressed;

      brake_effect_override = last_brake_press_time && (millis() - last_brake_press_time < brake_effect_duration);
      if(brake_state_change || brake_effect_override){
        strip.setTransition(0);
        if(brake_pedal_pressed || brake_effect_override){
          updateSegment(BRAKE_SEGMENT, FX_MODE_STATIC, 0xFF0000, 0xFF0000, 0xFF0000, 255, 0);
          Serial.println("++++++ BRAKE PRESSED ++++++");
          Segment& seg = strip.getSegment(BRAKE_SEGMENT);
          seg.mode = FX_MODE_STATIC;
          seg.colors[0] = uint32_t(0xFF0000);
          seg.colors[1] = uint32_t(0xFF0000);
          seg.colors[2] = uint32_t(0xFF0000);
          seg.speed = 255;
          strip.trigger();
          brake_pedal_previous_state = true;\
          if(!last_brake_press_time) {
            last_brake_press_time = millis();
          }
        }else{
          //restorePreviousState(BRAKE_SEGMENT, previousBrakeState);
          Serial.println("------ BRAKE RELEASED ------");
          strip.setMode(BRAKE_SEGMENT, previousBrakeState.mode);
          strip.getSegments()[BRAKE_SEGMENT].setColor(0, previousBrakeState.colors[0]);
          strip.trigger();
          brake_pedal_previous_state = false;
          last_brake_press_time = 0;        
        }
      }else{
          strip.setTransition(1000);
          previousBrakeState = {
            brake_segment.mode, 
            brake_segment.palette, 
            brake_segment.intensity, 
            brake_segment.colors, 
            brake_segment.speed,
            brake_segment.opacity,
            brake_segment.cct,
          };
      }

      const bool left_state_change = left_indicator_previous_state != left_indicator_on;
      left_effect_override = last_left_on_time && (millis() - last_left_on_time < left_effect_duration);
      
      if(left_state_change){
        strip.setTransition(0);
        if(left_indicator_on){

          /** Print the current color array of the segment to console **/

          updateSegment(LEFT_SEGMENT, FX_MODE_RUNNING_COLOR, 0xFFAA00, 0x000000, 0x000000, 255, 0);
          Serial.println("++++++ LEFT CLICKER ON ++++++");

          // Segment& seg = strip.getSegment(LEFT_SEGMENT);
          // seg.mode = FX_MODE_RUNNING_COLOR;
          // seg.colors[0] = uint32_t(0xFFAA00);
          // seg.colors[1] = uint32_t(0x000000);
          // seg.colors[2] = uint32_t(0x000000);
          // seg.setOption(SEG_OPTION_REVERSED, true);
          // seg.speed = 255;
          // strip.trigger();
          left_indicator_previous_state = true;
          if(!last_left_on_time) {
            last_left_on_time = millis();
          }
        }else{
          restorePreviousState(LEFT_SEGMENT, left_segment_cache);
          Serial.println("------ LEFT CLICKER OFF ------");
          // strip.setMode(LEFT_SEGMENT, previousLeftState.mode);
          // strip.getSegments()[LEFT_SEGMENT].setColor(0, previousLeftState.colors[0]);
          strip.trigger();
          left_indicator_previous_state = false;
          last_left_on_time = 0;
        } 
      }else if(!left_indicator_on && !left_effect_override){
          strip.setTransition(1000);
          left_segment_cache = strip.getSegment(LEFT_SEGMENT);
      }

      const bool right_state_change = right_indicator_previous_state != right_indicator_on;
      right_effect_override = last_right_on_time && (millis() - last_right_on_time < right_effect_duration);
      if(right_state_change || right_effect_override){
        strip.setTransition(0);
        if(right_indicator_on || right_effect_override){
          updateSegment(RIGHT_SEGMENT, FX_MODE_RUNNING_COLOR, 0xFFAA00, 0x000000, 0x000000, 255, 0);

          Serial.println("++++++ RIGHT CLICKER ON ++++++");

          Segment& seg = strip.getSegment(RIGHT_SEGMENT);
          seg.mode = FX_MODE_RUNNING_COLOR;
          seg.colors[0] = uint32_t(0xFFAA00);
          seg.colors[1] = uint32_t(0x000000);
          seg.colors[2] = uint32_t(0x000000);
          seg.speed = 255;
          strip.trigger();
          right_indicator_previous_state = true;
          if(!last_right_on_time) {
            last_right_on_time = millis();
          }
        }else{
          //restorePreviousState(RIGHT_SEGMENT, previousRightState);
          Serial.println("------ RIGHT CLICKER OFF ------");
          strip.setMode(RIGHT_SEGMENT, previousRightState.mode);
          strip.getSegments()[RIGHT_SEGMENT].setColor(0, previousRightState.colors[0]);
          strip.trigger();
          right_indicator_previous_state = false;  
          last_right_on_time = 0;      
        }
      }else{
          strip.setTransition(1000);
          previousRightState = {
            right_segment.mode, 
            right_segment.palette, 
            right_segment.intensity, 
            right_segment.colors, 
            right_segment.speed,
            right_segment.opacity,
            right_segment.cct,
          };
      }
    }
};