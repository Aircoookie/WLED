static uint16_t mode_static(void)
{
  SEGMENT.fill(SEGCOLOR(0));
  return strip.isOffRefreshRequired() ? FRAMETIME : 350;
}

static uint16_t mode_diffusionfire(void)
{
  static uint32_t call = 0;

  if (!strip.isMatrix || !SEGMENT.is2D())
    return mode_static(); // not a 2D set-up

  const int cols = SEG_W;
  const int rows = SEG_H;

  const uint8_t refresh_hz = map(SEGMENT.speed, 0, 255, 20, 80);
  const unsigned refresh_ms = 1000 / refresh_hz;
  const int16_t diffusion = map(SEGMENT.custom1, 0, 255, 0, 100);
  const uint8_t spark_rate = SEGMENT.intensity;
  const uint8_t turbulence = SEGMENT.custom2;

  unsigned dataSize = SEGMENT.length() + (cols * sizeof(uint16_t));
  if (!SEGENV.allocateData(dataSize))
    return mode_static(); // allocation failed

  if (SEGENV.call == 0)
  {
    SEGMENT.fill(BLACK);
    SEGENV.step = 0;
    call = 0;
  }

  if ((strip.now - SEGENV.step) >= refresh_ms)
  {
    uint16_t *tmp_row = (uint16_t *)(SEGMENT.data + SEGMENT.length());
    SEGENV.step = strip.now;
    call++;

    // scroll up
    for (unsigned y = 1; y < rows; y++)
      for (unsigned x = 0; x < cols; x++)
      {
        unsigned src = SEGMENT.XY(x, y);
        unsigned dst = SEGMENT.XY(x, y - 1);
        SEGMENT.data[dst] = SEGMENT.data[src];
      }

    if (random8() > turbulence)
    {
      // create new sparks at bottom row
      for (unsigned x = 0; x < cols; x++)
      {
        uint8_t p = random8();
        if (p < spark_rate)
        {
          unsigned dst = SEGMENT.XY(x, rows - 1);
          SEGMENT.data[dst] = 255;
        }
      }
    }

    // diffuse
    for (unsigned y = 0; y < rows; y++)
    {
      for (unsigned x = 0; x < cols; x++)
      {
        unsigned v = SEGMENT.data[SEGMENT.XY(x, y)];
        if (x > 0)
        {
          v += SEGMENT.data[SEGMENT.XY(x - 1, y)];
        }
        if (x < (cols - 1))
        {
          v += SEGMENT.data[SEGMENT.XY(x + 1, y)];
        }
        tmp_row[x] = min(255, (int)(v / (3.0 + (float)diffusion / 100.0)));
      }

      for (unsigned x = 0; x < cols; x++)
      {
        SEGMENT.data[SEGMENT.XY(x, y)] = tmp_row[x];
        if (SEGMENT.check1)
        {
          CRGB color = ColorFromPalette(SEGPALETTE, tmp_row[x], 255, NOBLEND);
          SEGMENT.setPixelColorXY(x, y, color);
        }
        else
        {
          uint32_t color = SEGCOLOR(0);
          SEGMENT.setPixelColorXY(x, y, color_fade(color, tmp_row[x]));
        }
      }
    }
  }
  return FRAMETIME;
}
static const char _data_FX_MODE_DIFFUSIONFIRE[] PROGMEM = "Diffusion Fire@!,Spark rate,Diffusion Speed,Turbulence,,Use palette;;Color;;2;pal=35";

class DiffusionFireUsermod : public Usermod
{

private:
public:
  void setup()
  {
    strip.addEffect(255, &mode_diffusionfire, _data_FX_MODE_DIFFUSIONFIRE);
  }

  void loop()
  {
  }

  uint16_t getId()
  {
    return USERMOD_ID_DIFFUSIONFIRE;
  }
};
