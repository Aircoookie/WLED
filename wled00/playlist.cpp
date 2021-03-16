#include "wled.h"

/*
 * Handles playlists, timed sequences of presets
 */

typedef struct PlaylistEntry {
  uint8_t preset;
  uint16_t dur;
  uint16_t tr;
} ple;

int8_t   playlistRepeat = 1;
byte     playlistEndPreset = 0;
byte    *playlistEntries = nullptr;
byte     playlistLen;
int8_t   playlistIndex = -1;
uint16_t playlistEntryDur = 0;


void shufflePlaylist() {
  int currentIndex = playlistLen, randomIndex;

  PlaylistEntry temporaryValue, *entries = reinterpret_cast<PlaylistEntry*>(playlistEntries);

  // While there remain elements to shuffle...
  while (currentIndex--) {
    // Pick a random element...
    randomIndex = random(0, currentIndex);
    // And swap it with the current element.
    temporaryValue = entries[currentIndex];
    entries[currentIndex] = entries[randomIndex];
    entries[randomIndex] = temporaryValue;
  }
}

void unloadPlaylist() {
  if (playlistEntries != nullptr) {
    delete[] playlistEntries;
    playlistEntries = nullptr;
  }
  currentPlaylist = playlistIndex = -1;
  playlistLen = playlistEntryDur = 0;
}

void loadPlaylist(JsonObject playlistObj) {
  unloadPlaylist();
  
  JsonArray presets = playlistObj["ps"];
  playlistLen = presets.size();
  if (playlistLen == 0) return;
  if (playlistLen > 100) playlistLen = 100;
  uint16_t dataSize = sizeof(ple) * playlistLen;
  playlistEntries = new byte[dataSize];
  PlaylistEntry* entries = reinterpret_cast<PlaylistEntry*>(playlistEntries);

  byte it = 0;
  for (int ps : presets) {
    if (it >= playlistLen) break;
    entries[it].preset = ps;
    it++;
  }

  it = 0;
  JsonArray durations = playlistObj["dur"];
  if (durations.isNull()) {
    entries[0].dur = playlistObj["dur"] | 100;
    it = 1;
  } else {
    for (int dur : durations) {
      if (it >= playlistLen) break;
      entries[it].dur = dur;
      it++;
    }
  }
  for (int i = it; i < playlistLen; i++) entries[i].dur = entries[it -1].dur;

  it = 0;
  JsonArray tr = playlistObj["transition"];
  if (tr.isNull()) {
    entries[0].tr = playlistObj["transition"] | (transitionDelay / 100);
    it = 1;
  } else {
    for (int transition : tr) {
      if (it >= playlistLen) break;
      entries[it].tr = transition;
      it++;
    }
  }
  for (int i = it; i < playlistLen; i++) entries[i].tr = entries[it -1].tr;

  playlistRepeat = playlistObj[F("repeat")] | 0;
  playlistEndPreset = playlistObj[F("end")] | 0;

  currentPlaylist = 0; //TODO here we need the preset ID where the playlist is saved
}


void handlePlaylist() {
  if (currentPlaylist < 0 || playlistEntries == nullptr || presetCyclingEnabled) return;
  
  if (millis() - presetCycledTime > (100*playlistEntryDur)) {
    presetCycledTime = millis();
    if (bri == 0 || nightlightActive) return;

    ++playlistIndex %= playlistLen; // -1 at 1st run (limit to playlistLen)

    if (!playlistRepeat && !playlistIndex) { //stop if repeat == 0 and restart of playlist
      unloadPlaylist();
      if (playlistEndPreset) applyPreset(playlistEndPreset);
      return;
    }
    // playlist roll-over
    if (!playlistIndex) {
      if (playlistRepeat > 0) {// playlistRepeat < 0 => endless loop with shuffling presets
        playlistRepeat--; // decrease repeat count on each index reset
      } else {
        shufflePlaylist();  // shuffle playlist and start over
      }
    }

    PlaylistEntry* entries = reinterpret_cast<PlaylistEntry*>(playlistEntries);

    jsonTransitionOnce = true;
    transitionDelayTemp = entries[playlistIndex].tr * 100;

    applyPreset(entries[playlistIndex].preset);
    playlistEntryDur = entries[playlistIndex].dur;
    if (playlistEntryDur == 0) playlistEntryDur = 10;
  }
}
