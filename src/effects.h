// What every show draws with: the frame buffer, letters, sprites, shared effects and the
// scene runner. Each show lives in src/shows/ and includes this.
#pragma once
#include "platform.h"
#include "show.h"

extern uint8_t bg[WIDTH];  // a picture to fade in or out
extern bool sceneStopped;  // the platform asked to skip the rest of this scene

#define STEP_MS 35  // one animation step

// Shows fb, and leaves the current function if the scene is being skipped
#define FRAME(ms) do { if (sceneStopped || !frame(ms)) { sceneStopped = true; return; } } while (0)

// Draws marcher number `which` of a parade at x, on frame f. Each show sets its own when it starts.
typedef void (*Marcher)(uint8_t which, int x, int f);
extern Marcher showMarcher;

// Drawing. Text and sprites are read with pgm_read_byte, so on the UNO they live in flash.
void wipe(uint8_t *buf);
void setPixel(uint8_t *buf, int x, int y);
uint8_t drawChar(uint8_t *buf, char c, int x, int y, uint8_t wide = 1);  // returns its width
uint8_t textLen(const char *s);
int textWidth(const char *s, uint8_t wide = 1);
void drawText(uint8_t *buf, const char *s, int x, int y = 0, uint8_t wide = 1);
int centered(const char *s, uint8_t wide = 1);
uint8_t widest(const char *s);  // 2 (double width) if it fits on the sign, else 1
void drawSprite(uint8_t *buf, const uint8_t *rows, int x, int y = 0);

// Effects
void dissolve(const uint8_t *target, bool lightUp);
void dropIn(const char *s);
void dropOut(const char *s);
void drawWave(uint8_t *buf, const char *s, int x, int t, uint8_t wide = 1);
void crumble();
void parade(const char *s, uint8_t marchers);  // with the show's marchers behind the words

// Scenes every show uses
void sceneJoke(const char *const *jokes, uint8_t count, uint8_t *next);  // jokes: a PROGMEM list
void sceneScores();

// Runs one scene (after a new phone message, if there is one); nothing once the day's show
// has changed, so the rest of the pass is skipped
void play(void (*scene)());
void playThenScores(void (*scene)());  // the scene, then the scores screen
