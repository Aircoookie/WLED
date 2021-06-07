#include "wled.h"

/*
 * Handles playlists, timed sequences of presets
 */

typedef struct PlaylistEntry {
  uint8_t preset;
  uint16_t dur;
  uint16_t tr;
} ple;

bool           playlistEndless = false;
int8_t         playlistRepeat = 1;
byte           playlistEndPreset = 0;
PlaylistEntry *playlistEntries = nullptr;
byte           playlistLen;
int8_t         playlistIndex = -1;
uint16_t       playlistEntryDur = 0;


void shufflePlaylist() {
  int currentIndex = playlistLen;
  PlaylistEntry temporaryValue;

  // While there remain elements to shuffle...
  while (currentIndex--) {
    // Pick a random element...
    int randomIndex = random(0, currentIndex);
    // And swap it with the current element.
    temporaryValue = playlistEntries[currentIndex];
    playlistEntries[currentIndex] = playlistEntries[randomIndex];
    playlistEntries[randomIndex] = temporaryValue;
  }
  DEBUG_PRINTLN(F("Playlist shuffle."));
}


void unloadPlaylist() {
  if (playlistEntries != nullptr) {
    delete[] playlistEntries;
    playlistEntries = nullptr;
  }
  currentPlaylist = playlistIndex = -1;
  playlistLen = playlistEntryDur = 0;
  DEBUG_PRINTLN(F("Playlist unloaded."));
}


void loadPlaylist(JsonObject playlistObj) {
  unloadPlaylist();
  
  JsonArray presets = playlistObj["ps"];
  playlistLen = presets.size();
  if (playlistLen == 0) return;
  if (playlistLen > 100) playlistLen = 100;

  playlistEntries = new PlaylistEntry[playlistLen];
  if (playlistEntries == nullptr) return;

  byte it = 0;
  for (int ps : presets) {
    if (it >= playlistLen) break;
    playlistEntries[it].preset = ps;
    it++;
  }

  it = 0;
  JsonArray durations = playlistObj["dur"];
  if (durations.isNull()) {
    playlistEntries[0].dur = playlistObj["dur"] | 100;
    it = 1;
  } else {
    for (int dur : durations) {
      if (it >= playlistLen) break;
      playlistEntries[it].dur = (dur > 0) ? dur : presetCycleTime;
      it++;
    }
  }
  for (int i = it; i < playlistLen; i++) playlistEntries[i].dur = playlistEntries[it -1].dur;

  it = 0;
  JsonArray tr = playlistObj[F("transition")];
  if (tr.isNull()) {
    playlistEntries[0].tr = playlistObj[F("transition")] | (transitionDelay / 100);
    it = 1;
  } else {
    for (int transition : tr) {
      if (it >= playlistLen) break;
      playlistEntries[it].tr = transition;
      it++;
    }
  }
  for (int i = it; i < playlistLen; i++) playlistEntries[i].tr = playlistEntries[it -1].tr;

  playlistRepeat = playlistObj[F("repeat")] | 0;
  playlistEndPreset = playlistObj[F("end")] | 0;

  if (playlistRepeat <= 0) playlistRepeat--; // make it endless (-2 == endless & random)

  currentPlaylist = 0; //TODO here we need the preset ID where the playlist is saved
  DEBUG_PRINTLN(F("Playlist loaded."));
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
      // playlistRepeat < 0 => endless loop
      if (playlistRepeat >  0) playlistRepeat--;  // decrease repeat count on each index reset if not an endless playlist
      if (playlistRepeat < -1) shufflePlaylist(); // shuffle playlist and start over
    }

    jsonTransitionOnce = true;
    transitionDelayTemp = playlistEntries[playlistIndex].tr * 100;
    playlistEntryDur = playlistEntries[playlistIndex].dur;
    applyPreset(playlistEntries[playlistIndex].preset, true);
  }
}
