// Runs the show on the Mac and writes an HTML page that plays every frame.
// Build and run with tools/preview/build.sh; open build/preview.html in a browser.
//   preview.html              plays the show
//   preview.html?t=12.5       paused at 12.5 s
//   preview.html?sheet=0-30   a still every 0.5 s from 0 to 30 s (&every=250 for every 250 ms)
// With PREVIEW_SCORES pointing at a file of game lines (like the Pi's scores file), the
// scores screens show up too.
#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include "show.h"

struct Frame {
  unsigned ms;
  std::string hex;
};

static std::vector<Frame> frames;
static unsigned peakLit;

bool frame(uint16_t ms) {
  static const char HEX[] = "0123456789abcdef";
  std::string hex;
  unsigned lit = 0;
  for (int x = 0; x < WIDTH; x++) {
    hex += HEX[fb[x] >> 4];
    hex += HEX[fb[x] & 15];
    lit += __builtin_popcount(fb[x]);
  }
  if (lit > peakLit) peakLit = lit;
  if (!frames.empty() && frames.back().hex == hex) frames.back().ms += ms;
  else frames.push_back({ms, hex});
  return true;
}

uint32_t rnd(uint32_t n) { return n ? (uint32_t)std::rand() % n : 0; }

const char *phoneMessage(bool *isNew) {
  *isNew = false;
  return nullptr;
}

static std::vector<std::string> scores;  // sample games, if PREVIEW_SCORES names a file

uint8_t scoreCount() { return scores.size(); }
const char *scoreLine(uint8_t i) { return i < scores.size() ? scores[i].c_str() : nullptr; }

static const char PAGE_HEAD[] = R"(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Sign Preview</title>
<style>
body{background:#0d0d10;color:#ddd;font:14px/1.4 system-ui,sans-serif;margin:16px}
h1{font-size:18px;margin:0 0 12px}
canvas{display:block;width:100%;max-width:1052px;height:auto;background:#000;border-radius:6px}
.bar{display:flex;gap:8px;align-items:center;margin:8px 0;max-width:1052px}
.bar input{flex:1}
.sheet{display:grid;gap:4px}
.sheet div{display:flex;gap:8px;align-items:center;color:#888;font-size:12px}
.sheet span{width:48px;text-align:right}
.sheet canvas{max-width:526px}
</style></head><body>
<h1>Sign preview</h1>
<div id="player"><canvas id="c"></canvas>
<div class="bar"><button id="pp">Pause</button><input id="seek" type="range" min="0" step="10"><span id="time"></span></div></div>
<div id="sheet" class="sheet"></div>
<script>
const F=[
)";

static const char PAGE_TAIL[] = R"(];
const W=64, GAP=4, starts=[]; let total=0;
for (const f of F) { starts.push(total); total += f[0]; }
function setup(cv, cell) { cv.width = W*cell + 7*GAP; cv.height = 8*cell; }
function draw(cv, hex, cell) {
  const g = cv.getContext('2d');
  g.fillStyle = '#000'; g.fillRect(0, 0, cv.width, cv.height);
  for (let x = 0; x < W; x++) {
    const col = parseInt(hex.substr(x*2, 2), 16), cx = x*cell + Math.floor(x/8)*GAP + cell/2;
    for (let y = 0; y < 8; y++) {
      g.fillStyle = (col >> y) & 1 ? '#ff4a1c' : '#2a0d06';
      g.beginPath(); g.arc(cx, y*cell + cell/2, cell*0.38, 0, 7); g.fill();
    }
  }
}
function at(t) { let lo = 0, hi = F.length-1; while (lo < hi) { const m = (lo+hi+1)>>1; if (starts[m] <= t) lo = m; else hi = m-1; } return F[lo][1]; }
const q = new URLSearchParams(location.search);
if (q.has('sheet')) {
  document.getElementById('player').remove();
  const [a, b] = (q.get('sheet') || '0-30').split('-').map(Number), every = +(q.get('every') || 500);
  for (let t = a*1000; t <= Math.min(b*1000, total); t += every) {
    const row = document.createElement('div'), label = document.createElement('span'), cv = document.createElement('canvas');
    label.textContent = (t/1000).toFixed(2) + 's'; setup(cv, 8); draw(cv, at(t), 8);
    row.append(label, cv); document.getElementById('sheet').append(row);
  }
} else {
  const cv = document.getElementById('c'), seek = document.getElementById('seek'), pp = document.getElementById('pp'), time = document.getElementById('time');
  setup(cv, 16); seek.max = total;
  let t = q.has('t') ? +q.get('t')*1000 : 0, playing = !q.has('t'), prev = performance.now();
  pp.textContent = playing ? 'Pause' : 'Play';
  pp.onclick = () => { playing = !playing; pp.textContent = playing ? 'Pause' : 'Play'; };
  seek.oninput = () => { t = +seek.value; };
  (function tick(now) {
    if (playing) { t = (t + now - prev) % total; seek.value = t; }
    prev = now; draw(cv, at(t), 16);
    time.textContent = (t/1000).toFixed(1) + ' / ' + (total/1000).toFixed(1) + ' s';
    requestAnimationFrame(tick);
  })(prev);
}
</script></body></html>
)";

int main(int argc, char **argv) {
  const char *out = argc > 1 ? argv[1] : "preview.html";
  std::srand(1031);
  if (const char *path = std::getenv("PREVIEW_SCORES")) {
    std::ifstream in(path);
    for (std::string line; std::getline(in, line);)
      if (!line.empty()) scores.push_back(line);
  }
  for (int pass = 0; pass < 9; pass++) runShow();  // nine passes show all eighteen jokes

  FILE *f = std::fopen(out, "w");
  if (!f) {
    std::perror(out);
    return 1;
  }
  std::fputs(PAGE_HEAD, f);
  unsigned long total = 0;
  for (const Frame &fr : frames) {
    std::fprintf(f, "[%u,\"%s\"],\n", fr.ms, fr.hex.c_str());
    total += fr.ms;
  }
  std::fputs(PAGE_TAIL, f);
  std::fclose(f);

  // LED current: about 40 mA per lit segment, 1/8 of the time, at (2*brightness+1)/32 duty
  double ma = peakLit * 40.0 / 8 * (2 * BRIGHTNESS + 1) / 32;
  std::printf("%zu frames, %.1f s for nine passes -> %s\n", frames.size(), total / 1000.0, out);
  std::printf("busiest frame: %u of %d LEDs lit (%.0f%%), about %.0f mA at brightness %d\n",
              peakLit, WIDTH * 8, 100.0 * peakLit / (WIDTH * 8), ma, BRIGHTNESS);
  return 0;
}
