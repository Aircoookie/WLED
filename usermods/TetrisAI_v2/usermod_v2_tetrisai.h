#pragma once

#include "wled.h"
#include "FX.h"
#include "fcn_declare.h"

#include "tetrisaigame.h"
// By: muebau

typedef struct TetrisAI_data
{
  uint8_t   intelligence;
  uint8_t   rotate;
  bool      showNext;
  bool      showBorder;
  uint8_t   colorOffset;
  uint8_t   colorInc;
  uint8_t   mistaceCountdown;
} tetrisai_data;

void drawGrid(TetrisAIGame* tetris, TetrisAI_data* tetrisai_data)
{
  SEGMENT.fill(SEGCOLOR(1));

  //GRID
  for (auto index_y = 4; index_y < tetris->grid.height; index_y++)
  {
    for (auto index_x = 0; index_x < tetris->grid.width; index_x++)
    {
      CRGB color;
      if (*tetris->grid.getPixel(index_x, index_y) == 0)
      {
        //BG color
        color = SEGCOLOR(1);
      }
      //game over animation
      else if(*tetris->grid.getPixel(index_x, index_y) == 254)
      {
        //use fg
        color = SEGCOLOR(0);
      }
      else
      {
        //spread the color over the whole palette
        uint8_t colorIndex = *tetris->grid.getPixel(index_x, index_y) * 32;
        colorIndex += tetrisai_data->colorOffset;
        color = ColorFromPalette(SEGPALETTE, colorIndex, 255, NOBLEND);
      }

      SEGMENT.setPixelColorXY(index_x, index_y - 4, color);
    }
  }
  tetrisai_data->colorOffset += tetrisai_data->colorInc;

  //NEXT PIECE AREA
  if (tetrisai_data->showNext)
  {
    //BORDER
    if (tetrisai_data->showBorder)
    {
      //draw a line 6 pixels from right with the border color
      for (auto index_y = 0; index_y < SEGMENT.virtualHeight(); index_y++)
      {
        SEGMENT.setPixelColorXY(SEGMENT.virtualWidth() - 6, index_y, SEGCOLOR(2));
      }
    }

    //NEXT PIECE
    int piecesOffsetX = SEGMENT.virtualWidth() - 4;
    int piecesOffsetY = 1;
    for (uint8_t nextPieceIdx = 1; nextPieceIdx < tetris->nLookAhead; nextPieceIdx++)
    {
      uint8_t pieceNbrOffsetY = (nextPieceIdx - 1) * 5;

      Piece piece(tetris->bag.piecesQueue[nextPieceIdx]);

      for (uint8_t pieceY = 0; pieceY < piece.getRotation().height; pieceY++)
      {
        for (uint8_t pieceX = 0; pieceX < piece.getRotation().width; pieceX++)
        {
          if (piece.getPixel(pieceX, pieceY))
          {
            uint8_t colIdx = ((piece.pieceData->colorIndex * 32) + tetrisai_data->colorOffset);
            SEGMENT.setPixelColorXY(piecesOffsetX + pieceX, piecesOffsetY + pieceNbrOffsetY + pieceY, ColorFromPalette(SEGPALETTE, colIdx, 255, NOBLEND));
          }
        }
      }
    }
  }
}

////////////////////////////
//     2D Tetris AI       //
////////////////////////////
uint16_t mode_2DTetrisAI()
{
  static unsigned long lastTime = 0;

  if (!strip.isMatrix || !SEGENV.allocateData(sizeof(tetrisai_data)))
  {
    // not a 2D set-up
    SEGMENT.fill(SEGCOLOR(0));
    return 350;
  }
  TetrisAI_data* tetrisai_data = reinterpret_cast<TetrisAI_data*>(SEGENV.data);

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  //range 0 - 1024ms => 1024/255 ~ 4
  uint16_t msDelayMove = 1024 - (4 * SEGMENT.speed);
  int16_t msDelayGameOver = msDelayMove / 4;

  //range 0 - 2 (not including current)
  uint8_t nLookAhead = SEGMENT.intensity ? (SEGMENT.intensity >> 7) + 2 : 1;
  //range 0 - 16
  tetrisai_data->colorInc = SEGMENT.custom2 >> 4;

  static TetrisAIGame tetris(cols < 32 ? cols : 32, rows, 1, piecesData, numPieces);

  if (tetris.nLookAhead != nLookAhead
    || tetrisai_data->showNext != SEGMENT.check1
    || tetrisai_data->showBorder != SEGMENT.check2
    )
  {
    tetrisai_data->showNext = SEGMENT.check1;
    tetrisai_data->showBorder = SEGMENT.check2;
    tetris.nLookAhead = nLookAhead;

    //not more than 32 as this is the limit of this implementation
    uint8_t gridWidth = cols < 32 ? cols : 32;
    uint8_t gridHeight = rows;

    // do we need space for the 'next' section?
    if (tetrisai_data->showNext)
    {
      // make space for the piece and one pixel of space
      gridWidth = gridWidth - 5;

      // do we need space for a border?
      if (tetrisai_data->showBorder)
      {
        gridWidth = gridWidth - 1;
      }
    }

    tetris = TetrisAIGame(gridWidth, gridHeight, nLookAhead, piecesData, numPieces);
    SEGMENT.fill(SEGCOLOR(1));
  }

  if (tetrisai_data->intelligence != SEGMENT.custom1)
  {
    tetrisai_data->intelligence = SEGMENT.custom1;
    double dui = 0.2 - (0.2 * (tetrisai_data->intelligence / 255.0));

    tetris.ai.aHeight = -0.510066 + dui;
    tetris.ai.fullLines = 0.760666 - dui;
    tetris.ai.holes = -0.35663 + dui;
    tetris.ai.bumpiness = -0.184483 + dui;
  }

  if (tetris.state == TetrisAIGame::ANIMATE_MOVE)
  {
    if (millis() - lastTime > msDelayMove)
    {
      drawGrid(&tetris, tetrisai_data);
      lastTime = millis();
      tetris.poll();
    }
  }
  else if (tetris.state == TetrisAIGame::ANIMATE_GAME_OVER)
  {
    if (millis() - lastTime > msDelayGameOver)
    {
      drawGrid(&tetris, tetrisai_data);
      lastTime = millis();
      tetris.poll();
    }
  }
  else if (tetris.state == TetrisAIGame::FIND_BEST_MOVE)
  {
    if (SEGMENT.check3)
    {
      if(tetrisai_data->mistaceCountdown == 0)
      {
        tetris.ai.findWorstMove = true;
        tetris.poll();
        tetris.ai.findWorstMove = false;
        tetrisai_data->mistaceCountdown = SEGMENT.custom3;
      }
      tetrisai_data->mistaceCountdown--;      
    }
    tetris.poll();
  }
  else
  {
    tetris.poll();
  }

  return FRAMETIME;
} // mode_2DTetrisAI()
static const char _data_FX_MODE_2DTETRISAI[] PROGMEM = "Tetris AI@!,Look ahead,Intelligence,Rotate color,Mistake free,Show next,Border,Mistakes;Game Over,!,Border;!;2;sx=127,ix=64,c1=255,c2=0,c3=31,o1=1,o2=1,o3=0,pal=11";

class TetrisAIUsermod : public Usermod
{

private:

public:
  void setup()
  {
    strip.addEffect(255, &mode_2DTetrisAI, _data_FX_MODE_2DTETRISAI);
  }

  void loop()
  {

  }

  uint16_t getId()
  {
    return USERMOD_ID_TETRISAI;
  }
};
