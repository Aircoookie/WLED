/*
 * Realtime Smoothing with Active Interval Control.  J.D. Smith, 2021
 * Smooths playback intervals of single or multi-packet UDP realtime frames
 */
#ifndef RTS_N_FRAME  //Number of buffered frames to store
#define RTS_N_FRAME 5  // (which can be multi-packet)
#endif

#ifdef RTS_MAX_SIZE  // Max packet size, larger will be silently dropped
#define RTS_BUF_SIZE RTS_MAX_SIZE
#else
#define RTS_BUF_SIZE UDP_IN_MAXSIZE
#endif

#ifndef RTS_MAX_PACKETS_PER_UPDATE
#define RTS_MAX_PACKETS_PER_UPDATE 1 // >1 for dnrgb(w) multi-packets
#endif

#ifndef RTS_MAX_GUIDE //Maximum half-range of guide factor <~0.15
#define RTS_MAX_GUIDE 0.1 
#endif

#define RTS_N_TRAIN 45   // # of samples to collect to pre-train average/SD
#define RTS_SIGMA_CUT 8  // # of std. deviations above which to trim samples as deviant

#ifndef RTS_TARGET
#define RTS_TARGET (RTS_N_FRAME/2 + 1) // Target Frame buffer fullness to try to maintain
#endif

#ifndef RTS_EMA_BINS
#define RTS_EMA_BINS 500 // average age (in deltaT intervals) of moving exponential bin
#endif

#define RTS_N_BUF (RTS_N_FRAME * RTS_MAX_PACKETS_PER_UPDATE)

class RealtimeSmooth {
 public:
  typedef struct RTS_Frame {
    uint8_t *packets[RTS_MAX_PACKETS_PER_UPDATE]; // pointing to buffers
    uint16_t len[RTS_MAX_PACKETS_PER_UPDATE];
    uint8_t pnum; // current packet number for this frame
  } rts_frame;

  uint16_t min_index;   // minimum starting index seen (dnrgb(w))
  rts_frame ring[RTS_N_FRAME];
  int head;		// ring buffer head position
  uint8_t frame_cnt; 	// number of frames currently in ring 

  uint32_t last_add, last_remove; // millis() of last added/removed frame 
  uint32_t start_time;		// time of first add
  uint32_t sample_cnt;		// sample count 

  float guide_fac; // Correction factor (~1.0) to steer deltaT with
  float bin_error; // Smoothed recent error e=(frame_cnt - RTS_TARGET)

  uint8_t* get_free_buffer(void);
  bool add(uint8_t *, uint16_t, uint32_t, bool=false);
  rts_frame* remove(uint32_t);
  void guide(float);
  void reset(void);
  void update_stats(uint32_t);
  void reset_stats(float);
  float mean_deltaT(void);
  float sigma_deltaT(void);
#ifdef DEBUG_RTS
  void log(uint32_t);
  uint16_t waiting, missed;
#endif

  RealtimeSmooth() {
    alpha = 1./RTS_EMA_BINS;
    beta = 1 - alpha;
    reset();
  }
    
 private:
  uint8_t  buffers[RTS_N_BUF][RTS_BUF_SIZE];
  uint16_t cur_buf;
  
  float alpha, beta;
  float old_dT, var;
  float deltaT;	// Estimated raw milliseconds between packet arrivals
#ifdef DEBUG_RTS
  uint32_t dry_cnt;		// times ring ran dry
  uint32_t overflow_cnt;	// times ring overflowed
  uint32_t outliers;		// number of std. outlier packets
  bool recovering; // Waiting for buffer to empty/refill after it overflowed/ran dry
  uint32_t last_log;
  float bin_avg;
  uint16_t bin_avg_cnt;
#endif
};

void RealtimeSmooth::reset(void) {
  head = 0;
  frame_cnt = 0;
  cur_buf = 0;
  last_add = last_remove = start_time = 0;
  sample_cnt = 0;
  min_index = 0;
  deltaT = 0.0;
  for(uint16_t i=0; i<RTS_N_FRAME; i++) ring[i].pnum = 0;

#ifdef DEBUG_RTS
  DEBUG_PRINTLN(">> Realtime Smoothing Reset...");
  dry_cnt = overflow_cnt = outliers = 0;
  recovering = false;
  bin_avg = 0.0;
  bin_avg_cnt=0;
  last_log = 0;
  waiting = missed = 0;
#endif

  guide_fac = 1.0;
  bin_error = 0.0;
}

float RealtimeSmooth::sigma_deltaT(void) {
  return sqrt(sample_cnt >= RTS_N_TRAIN?var:var/(sample_cnt - 1));
}

float RealtimeSmooth::mean_deltaT(void) {
  return guide_fac*deltaT;
}

void RealtimeSmooth::update_stats(uint32_t now) {
  uint32_t dT = now - last_add;
  sample_cnt++;

  if (sample_cnt == RTS_N_TRAIN) // switch average flavors
    var /= sample_cnt-2;
  else if (sample_cnt == 1) {
    start_time = millis();
#ifdef DEBUG_RTS
    DEBUG_PRINTLN(">> Realtime Smoothing Enabled...");
#endif
    return;			// 1 sample isn't enough for deltaT!
  }

#ifdef DEBUG_RTS
  bin_avg += frame_cnt;
  bin_avg_cnt++;
#endif
  
  if (sample_cnt >= RTS_N_TRAIN) { // Main Phase: Exponential average
    if(dT <= deltaT + RTS_SIGMA_CUT * sigma_deltaT()) { // Trim long-duration outliers
      var = beta * (var + alpha * (dT - deltaT)*(dT - deltaT));
      deltaT = beta * deltaT + alpha * dT;
    }
#ifdef DEBUG_RTS    
    else {
      outliers++;
    }
#endif
    // Bin offset guiding: try to make (recent average) frame_cnt == RTS_TARGET
    bin_error = 0.5 * bin_error + 0.5 * (frame_cnt - RTS_TARGET); 
    guide_fac = 1. - 2 * RTS_MAX_GUIDE * (bin_error/RTS_N_FRAME); // negative error: guide>1
  } else { // Training Phase: Full cumulative average, keep all samples 
    if (sample_cnt == 2) old_dT = deltaT = dT;
    deltaT = old_dT + (dT - old_dT)/sample_cnt;
    var += (dT - old_dT)*(dT - deltaT); // See Knuth TAOCP vol 2, 3rd edition, page 232
    old_dT = deltaT;
  }
}

uint8_t* RealtimeSmooth::get_free_buffer(void) {
  return buffers[cur_buf];
}

bool RealtimeSmooth::add(uint8_t *udpIn, uint16_t len, uint32_t now, bool re_adding) {
  uint16_t start_index=0;
  
  if (udpIn[0] == 4 || udpIn[0] == 5) {  // DN flavors
    start_index = ((udpIn[3] << 0) & 0xFF) + ((udpIn[2] << 8) & 0xFF00);
    if (start_index < min_index) min_index = start_index; // watch for minimum index
  }
  
  if(!re_adding) {
    cur_buf++; // prepare for next (valid) packet set
    if (cur_buf>=RTS_N_BUF) cur_buf=0; 

    if(start_index == min_index) { // min index marks start of new frame
      update_stats(now);
      last_add = now;
    }
  }

  if (start_index == min_index) {  // New packet set starting    
    if (frame_cnt == RTS_N_FRAME) {
#ifdef DEBUG_RTS
      overflow_cnt++;
#endif
      return false;
    }
    head--;
    if (head<0) head = RTS_N_FRAME - 1;
    frame_cnt++;
    ring[head].pnum = 0;	// 1st packet!
#if RTS_MAX_PACKETS_PER_UPDATE > 1
    for (uint16_t i = 1; i < RTS_MAX_PACKETS_PER_UPDATE; i++)
      ring[head].len[i] = 0;	// for safety
#endif
#ifdef DEBUG_RTS
    recovering = false;
#endif
  } else { // just more packets in a series..
    ring[head].pnum++;
    if (ring[head].pnum >= RTS_MAX_PACKETS_PER_UPDATE)
      ring[head].pnum = 0; // shouldn't happen
  }
  
  ring[head].len[ring[head].pnum]     = len;  // Slot the new packet into the frame!
  ring[head].packets[ring[head].pnum] = udpIn;
  return true;
}

RealtimeSmooth::rts_frame* RealtimeSmooth::remove(uint32_t now) {
  if (frame_cnt == 0) {
#ifdef DEBUG_RTS
    if (!recovering) {
      recovering = true;      
      dry_cnt++;
    }
#endif
    return NULL;
  } else {
    last_remove = now;
#ifdef DEBUG_RTS
    recovering = false;
#endif
  }
  
  uint16_t tail = head + frame_cnt - 1;
  if (tail >= RTS_N_FRAME) tail -= RTS_N_FRAME;
  frame_cnt--;
  return ring + tail;
}

#ifdef DEBUG_RTS
void RealtimeSmooth::log(uint32_t now) {
  if (sample_cnt <= 1 || now - last_log < 5000 || now - last_add < deltaT/2)
    return; // log during the quiet times
  last_log = now;

  float binav=0; 
  int bin=0,part_width=0;
  if (bin_avg_cnt>0) {
    binav=4*bin_avg/bin_avg_cnt;
    binav = binav>(4*RTS_N_FRAME)?4*RTS_N_FRAME:binav;
    binav = binav<0?0:binav;
    bin = floor(binav);
    part_width = floor((binav-bin)*8);
  }

  DEBUG_PRINTF(">>Realtime Smooth: %u total samples ",sample_cnt);
  DEBUG_PRINT("|\033[1;34m");
  for(int i = 0; i < bin; i++) DEBUG_PRINT("\u2588");
  switch (part_width) {
  case 0: DEBUG_PRINT(" "); break;
  case 1: DEBUG_PRINT("\u258F"); break;
  case 2: DEBUG_PRINT("\u258E"); break;
  case 3: DEBUG_PRINT("\u258D"); break;
  case 4: DEBUG_PRINT("\u258C"); break;
  case 5: DEBUG_PRINT("\u258B"); break;
  case 6: DEBUG_PRINT("\u258A"); break;
  case 7: DEBUG_PRINT("\u2589"); break;
  default: break;
  }
  for(int i = bin+1; i < 4*RTS_N_FRAME; i++) DEBUG_PRINT(" ");
  DEBUG_PRINTF("\033[0m| (avg: %0.3f/%d)\n",bin_avg/bin_avg_cnt,RTS_N_FRAME);
  
  if(sample_cnt < RTS_N_TRAIN) DEBUG_PRINTLN("\tTRAINING deltaT");
  DEBUG_PRINTF("\tGuide Offset: \033[1;%sm%+0.2f%%\033[0m, Bin error %0.3f\n",
	       guide_fac<1.?"31":"32",100*(guide_fac-1), bin_error);
  DEBUG_PRINTF("\tDry \033[1;31m%u\033[0m (%0.3f%%), "
	       "Overflowed \033[1;31m%u\033[0m (%0.3f%%), "
	       "Outliers: \033[1;31m%u\033[0m (%0.3f%%)\n"
	       "\tPackets – waiting: \033[1;31m%u\033[0m, missed: \033[1;31m%u\033[0m\n",
	       dry_cnt, (float)dry_cnt/sample_cnt*100.,
	       overflow_cnt, (float)overflow_cnt/sample_cnt*100.,
	       outliers,(float)outliers/(outliers + sample_cnt) * 100,
	       waiting, missed);

  DEBUG_PRINTF("\tdeltaT: \033[1m%0.3f±%0.3f ms\033[0m (min_index: %u)\n",mean_deltaT(),sigma_deltaT(),min_index);
  bin_avg = 0.0; bin_avg_cnt = 0;
}
#endif

RealtimeSmooth rts;
