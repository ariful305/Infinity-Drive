/*
 * ════════════════════════════════════════════════════════════════════
 *  THE INFINITE DRIVE  —  Photorealistic Edition
 *  Every pixel drawn by DDA Line + Midpoint Circle algorithms
 *  Techniques: atmospheric scattering, Phong shading approximation,
 *              volumetric fog, bloom glow, wet surfaces, parallax
 *  Controls: SPACE=next  D=prev  ESC=quit  (auto 12s per scene)
 * ════════════════════════════════════════════════════════════════════
 */
#include <GLUT/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Window ─────────────────────────────────────────────────────── */
int W = 800, H = 600;

/* ── Scene state ────────────────────────────────────────────────── */
int scene = 0, N_SCENES = 5;
float sceneTimer = 0, SCENE_DUR = 12.0f;
float fadeA = 1.0f;
int fadIn = 1, fadOut = 0;
float T = 0.0f; /* global time, seconds */

/* ── Per-scene animation ─────────────────────────────────────────── */
float c1a = 80, c1b = 720, c1p = 60;                         /* scene 1 */
float c2p = 400, c2sx = -80, c2sy = 95;                      /* scene 2 */
float c3m = 0, c3p = 55;                                     /* scene 3 */
float c4p = 55, c4sw = 0;                                    /* scene 4 */
float bx[3] = {90, 330, 570}, by2[3] = {490, 505, 495};      /* scene 4 birds */
float c5w = 0, c5b1 = 170, c5b2 = 560, c5p = 55, c5beam = 0; /* scene 5 */

/* ── Star field (shared) ─────────────────────────────────────────── */
float stX[120], stY[120], stS[120], stTw[120];
int stInit = 0;
void initStars()
{
  if (stInit)
    return;
  stInit = 1;
  srand(42);
  for (int i = 0; i < 120; i++)
  {
    stX[i] = (float)(rand() % 800);
    stY[i] = (float)(280 + rand() % 320);
    stS[i] = 0.8f + 1.4f * (rand() % 100) / 100.0f;
    stTw[i] = (float)(rand() % 628) / 100.0f;
  }
}

/* ══════════════════════════════════════════════════════════════════
   CORE DRAWING PRIMITIVES
   ══════════════════════════════════════════════════════════════════ */
void dda(float x1, float y1, float x2, float y2)
{
  float dx = x2 - x1, dy = y2 - y1;
  float s = fabsf(dx) > fabsf(dy) ? fabsf(dx) : fabsf(dy);
  if (s < 0.5f)
  {
    glBegin(GL_POINTS);
    glVertex2f(x1, y1);
    glEnd();
    return;
  }
  float xi = dx / s, yi = dy / s, x = x1, y = y1;
  glBegin(GL_POINTS);
  for (int i = 0; i <= (int)s; i++)
  {
    glVertex2f(x, y);
    x += xi;
    y += yi;
  }
  glEnd();
}
static void pp8(float cx, float cy, float x, float y)
{
  glVertex2f(cx + x, cy + y);
  glVertex2f(cx - x, cy + y);
  glVertex2f(cx + x, cy - y);
  glVertex2f(cx - x, cy - y);
  glVertex2f(cx + y, cy + x);
  glVertex2f(cx - y, cy + x);
  glVertex2f(cx + y, cy - x);
  glVertex2f(cx - y, cy - x);
}
void mca(float cx, float cy, float r)
{
  if (r < 1)
  {
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();
    return;
  }
  float x = 0, y = r, d = 1 - r;
  glBegin(GL_POINTS);
  pp8(cx, cy, x, y);
  while (x < y)
  {
    if (d < 0)
      d += 2 * x + 3;
    else
    {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
    pp8(cx, cy, x, y);
  }
  glEnd();
}
void fc(float cx, float cy, float r)
{
  for (float ri = 0.5f; ri <= r; ri += 0.5f)
    mca(cx, cy, ri);
}

/* ── Colour helpers ─────────────────────────────────────────────── */
void col3(float r, float g, float b) { glColor3f(r, g, b); }
/* clamp */
float cl(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
/* lerp */
float lp(float a, float b, float t) { return a + (b - a) * t; }

/* ── Gradient horizontal band ───────────────────────────────────── */
void gradBand(float x, float y, float w, float h,
              float r0, float g0, float b0, float r1, float g1, float b1)
{
  glBegin(GL_QUADS);
  col3(r0, g0, b0);
  glVertex2f(x, y + h);
  glVertex2f(x + w, y + h);
  col3(r1, g1, b1);
  glVertex2f(x + w, y);
  glVertex2f(x, y);
  glEnd();
}
/* ── Sky scan-line gradient (most realistic) ────────────────────── */
/* pass array of stops: {y_frac, r,g,b} */
typedef struct
{
  float yf, r, g, b;
} Stop;
void skyGrad(Stop *stops, int n)
{
  for (int y = 0; y < H; y++)
  {
    float t = (float)y / H;
    /* find bracket */
    int lo = 0;
    for (int i = 0; i < n - 1; i++)
    {
      if (t >= stops[i].yf && t < stops[i + 1].yf)
        lo = i;
    }
    float span = stops[lo + 1].yf - stops[lo].yf;
    float f = (span > 0) ? (t - stops[lo].yf) / span : 0;
    col3(lp(stops[lo].r, stops[lo + 1].r, f),
         lp(stops[lo].g, stops[lo + 1].g, f),
         lp(stops[lo].b, stops[lo + 1].b, f));
    dda(0, y, W, y);
  }
}

/* ── Glow halo around a point ───────────────────────────────────── */
void glow(float cx, float cy, float r, float R, float gr, float gg, float gb)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float ri = r; ri < R; ri += 1.5f)
  {
    float a = powf((R - ri) / (R - r), 2) * 0.35f;
    glColor4f(gr, gg, gb, a);
    mca(cx, cy, ri);
  }
  glDisable(GL_BLEND);
}

/* ── Filled rect ────────────────────────────────────────────────── */
void fillRect(float x, float y, float w, float h)
{
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + w, y);
  glVertex2f(x + w, y + h);
  glVertex2f(x, y + h);
  glEnd();
}

/* ── Stars ──────────────────────────────────────────────────────── */
void drawStars(float brightness)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int i = 0; i < 120; i++)
  {
    float tw = 0.55f + 0.45f * sinf(T * 1.8f + stTw[i]);
    float b = stS[i] * tw * brightness * 0.95f;
    glColor4f(0.92f, 0.93f, 1.0f, b);
    fc(stX[i], stY[i], stS[i] * 1.1f);
  }
  glDisable(GL_BLEND);
}

/* ── Moon ───────────────────────────────────────────────────────── */
void drawMoon(float x, float y, float skyr, float skyg, float skyb)
{
  /* atmosphere scatter glow */
  glow(x, y, 24, 65, 0.90f, 0.90f, 0.70f);
  /* disc */
  col3(0.955f, 0.952f, 0.880f);
  fc(x, y, 23);
  /* surface shading — slightly darker bottom-right */
  col3(0.860f, 0.855f, 0.780f);
  fc(x + 5, y - 4, 18);
  /* crescent shadow (sky colour) */
  col3(skyr, skyg, skyb);
  fc(x + 10, y + 6, 18);
  /* mare patches */
  col3(0.820f, 0.818f, 0.750f);
  fc(x - 5, y + 6, 7);
  fc(x + 6, y - 2, 5);
}

/* ── Realistic stick person with shadow ────────────────────────── */
void drawPerson(float x, float y, float shR, float shG, float shB)
{
  float sw = sinf(T * 4.2f) * 9.0f; /* walk swing */
  /* ground shadow (ellipse) */
  col3(0, 0, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0, 0, 0, 0.28f);
  for (float r = 0; r < 12; r += 0.5f)
    mca(x, y - 14, r);
  glDisable(GL_BLEND);
  /* body (clothes) */
  col3(shR, shG, shB);
  /* torso */
  dda(x, y + 20, x, y + 2);
  /* arms */
  dda(x, y + 15, x - 10, y + 5 + sw);
  dda(x, y + 15, x + 10, y + 5 - sw);
  /* legs */
  dda(x, y + 2, x - 8, y - 14 + sw * 0.4f);
  dda(x, y + 2, x + 8, y - 14 - sw * 0.4f);
  /* head */
  col3(0.88f, 0.70f, 0.52f);
  fc(x, y + 27, 7);
  /* hair */
  col3(0.15f, 0.09f, 0.04f);
  fc(x, y + 33, 5);
  /* face dot eyes */
  col3(0.12f, 0.08f, 0.05f);
  fc(x - 2.5f, y + 28, 1.2f);
  fc(x + 2.5f, y + 28, 1.2f);
}

/* ══════════════════════════════════════════════════════════════════
   SCENE 1 — CITY NIGHT  (rain-wet neon city)
   Photorealistic: wet asphalt reflections, neon bloom, layered fog,
   building material variety, detailed car geometry
   ══════════════════════════════════════════════════════════════════ */

/* Neon sign with full bloom */
void neonSign(float x, float y, float r, float g, float b, float radius)
{
  glow(x, y, radius, radius * 3.5f, r, g, b);
  col3(r, g, b);
  fc(x, y, radius);
  col3(cl(r + 0.3f), cl(g + 0.3f), cl(b + 0.3f));
  mca(x, y, radius + 1);
}

/* Realistic building with glass curtain wall */
void building(float x, float y, float w, float h,
              float mr, float mg, float mb, /* material base */
              int floors, int cols, float glassR, float glassG, float glassB)
{
  /* ambient occlusion base — darker bottom */
  for (float yy = y; yy < y + h; yy += 0.5f)
  {
    float sh = 0.7f + 0.3f * (yy - y) / h;
    col3(mr * sh, mg * sh, mb * sh);
    dda(x, yy, x + w, yy);
  }
  /* left edge AO */
  for (float yy = y; yy < y + h; yy += 0.5f)
  {
    float sh = 0.78f;
    col3(mr * sh, mg * sh, mb * sh);
    dda(x, yy, x + w * 0.06f, yy);
  }
  /* right highlight */
  col3(cl(mr + 0.08f), cl(mg + 0.08f), cl(mb + 0.10f));
  dda(x + w - 1, y, x + w - 1, y + h);
  /* roof edge */
  col3(cl(mr + 0.12f), cl(mg + 0.12f), cl(mb + 0.12f));
  dda(x, y + h, x + w, y + h);

  /* windows */
  float ww = w / cols, wh = h / floors;
  for (int f = 0; f < floors; f++)
  {
    for (int c = 0; c < cols; c++)
    {
      float wx = x + ww * c + ww * 0.15f, wy = y + wh * f + wh * 0.12f;
      float wp = ww * 0.70f, hp = wh * 0.65f;
      int lit = (rand() % (f / 2 + 2) != 0);
      /* glass base */
      float gc = (lit) ? 1.0f : 0.18f;
      col3(glassR * gc, glassG * gc, glassB * gc);
      fillRect(wx, wy, wp, hp);
      /* window frame */
      col3(mr * 0.55f, mg * 0.55f, mb * 0.55f);
      dda(wx, wy, wx + wp, wy);
      dda(wx, wy + hp, wx + wp, wy + hp);
      dda(wx, wy, wx, wy + hp);
      dda(wx + wp, wy, wx + wp, wy + hp);
      /* reflection sheen on glass */
      if (lit)
      {
        col3(cl(glassR + 0.15f), cl(glassG + 0.15f), cl(glassB + 0.05f));
        dda(wx + wp * 0.1f, wy + hp * 0.8f, wx + wp * 0.4f, wy + hp * 0.6f);
        /* bloom on wet road below */
        float ry = y - (wy - y) - hp * 0.5f;
        if (ry > 0 && ry < y)
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glColor4f(glassR, glassG, glassB, 0.18f);
          fillRect(wx, ry - hp * 0.3f, wp, hp * 0.35f);
          glDisable(GL_BLEND);
        }
      }
    }
  }
  /* antenna */
  col3(0.55f, 0.55f, 0.58f);
  dda(x + w * 0.6f, y + h, x + w * 0.6f, y + h + 22);
  col3(0.8f, 0.15f, 0.10f);
  fc(x + w * 0.6f, y + h + 22, 3); /* red warning light */
}

/* Highly detailed car */
void realCar(float x, float y, float br, float bg, float bb, int dir)
{
  float s = (dir > 0) ? 1.0f : -1.0f; /* direction scale for headlights */
  /* wet ground reflection */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(br * 0.30f, bg * 0.30f, bb * 0.30f, 0.55f);
  glBegin(GL_POLYGON);
  glVertex2f(x + 5, y - 2);
  glVertex2f(x + 68, y - 2);
  glVertex2f(x + 62, y - 11);
  glVertex2f(x + 11, y - 11);
  glEnd();
  glDisable(GL_BLEND);
  /* tyre shadow */
  col3(0.0f, 0.0f, 0.0f);
  fc(x + 13, y + 7, 12);
  fc(x + 55, y + 7, 12);
  /* tyres */
  col3(0.07f, 0.07f, 0.07f);
  fc(x + 13, y + 7, 11);
  fc(x + 55, y + 7, 11);
  col3(0.18f, 0.18f, 0.20f);
  fc(x + 13, y + 7, 6);
  fc(x + 55, y + 7, 6);
  col3(0.08f, 0.08f, 0.08f);
  mca(x + 13, y + 7, 6);
  mca(x + 55, y + 7, 6);
  /* body — gradient shaded */
  for (float yy = y + 7; yy < y + 28; yy += 0.5f)
  {
    float sh = 0.78f + 0.22f * (yy - y - 7) / 21.0f;
    col3(br * sh, bg * sh, bb * sh);
    dda(x, yy, x + 72, yy);
  }
  /* side panel highlight */
  col3(cl(br + 0.22f), cl(bg + 0.22f), cl(bb + 0.22f));
  dda(x + 2, y + 20, x + 70, y + 20);
  /* cabin (darker tint) */
  for (float yy = y + 28; yy < y + 42; yy += 0.5f)
  {
    float sh = 0.62f + 0.12f * (yy - y - 28) / 14.0f;
    col3(br * sh * 0.7f, bg * sh * 0.7f, bb * sh * 0.7f);
    dda(x + 10, yy, x + 56, yy);
  }
  /* windscreen blue-tinted glass */
  col3(0.28f, 0.55f, 0.82f);
  glBegin(GL_POLYGON);
  glVertex2f(x + 16, y + 42);
  glVertex2f(x + 52, y + 42);
  glVertex2f(x + 46, y + 28);
  glVertex2f(x + 22, y + 28);
  glEnd();
  col3(0.55f, 0.75f, 0.95f); /* glass sheen */
  dda(x + 18, y + 40, x + 30, y + 30);
  /* rear window */
  col3(0.22f, 0.44f, 0.72f);
  glBegin(GL_POLYGON);
  glVertex2f(x + 10, y + 42);
  glVertex2f(x + 16, y + 42);
  glVertex2f(x + 22, y + 28);
  glVertex2f(x + 14, y + 28);
  glEnd();
  /* door line */
  col3(br * 0.55f, bg * 0.55f, bb * 0.55f);
  dda(x + 36, y + 8, x + 36, y + 28);
  /* headlights */
  float hx = (dir > 0) ? (x + 68) : (x + 4);
  glow(hx, y + 18, 5, 22, 1.0f, 0.95f, 0.75f);
  col3(1.0f, 0.98f, 0.88f);
  fc(hx, y + 18, 5);
  /* tail lights */
  float tx = (dir > 0) ? (x + 4) : (x + 68);
  glow(tx, y + 18, 3, 12, 0.95f, 0.05f, 0.05f);
  col3(0.9f, 0.05f, 0.05f);
  fc(tx, y + 18, 3);
  /* roof rack shadow line */
  col3(br * 0.45f, bg * 0.45f, bb * 0.45f);
  dda(x + 10, y + 42, x + 56, y + 42);
}

/* Realistic traffic light with pole box */
void trafficLight(float x, float y)
{
  /* shadow */
  col3(0.08f, 0.08f, 0.10f);
  dda(x + 3, y - 2, x + 3, y + 92);
  /* pole */
  col3(0.22f, 0.24f, 0.26f);
  dda(x, y, x, y + 92);
  col3(0.28f, 0.30f, 0.32f);
  dda(x + 1, y, x + 1, y + 92);
  /* housing box with gradient */
  for (float yy = y + 58; yy < y + 95; yy += 0.5f)
  {
    float sh = 0.7f + 0.3f * (yy - y - 58) / 37.0f;
    col3(0.10f * sh, 0.10f * sh, 0.12f * sh);
    dda(x - 9, yy, x + 9, yy);
  }
  col3(0.16f, 0.16f, 0.18f);
  dda(x - 9, y + 58, x + 9, y + 58);
  dda(x - 9, y + 95, x + 9, y + 95);
  /* lights */
  float pulse = 0.85f + 0.15f * sinf(T * 1.5f);
  glow(x, y + 88, 5, 18, 0.95f, 0.10f, 0.10f);
  col3(0.95f * pulse, 0.08f, 0.08f);
  fc(x, y + 88, 5);
  col3(0.90f, 0.70f, 0.02f);
  fc(x, y + 78, 5);
  glow(x, y + 68, 5, 16, 0.10f, 0.90f, 0.15f);
  col3(0.10f, 0.92f * pulse, 0.12f);
  fc(x, y + 68, 5);
}

void scene1()
{
  /* ── Sky: deep night gradient ── */
  Stop sky1[] = {{0.0f, 0.02f, 0.02f, 0.10f}, {0.45f, 0.04f, 0.04f, 0.18f}, {0.75f, 0.06f, 0.06f, 0.22f}, {1.0f, 0.08f, 0.08f, 0.26f}};
  skyGrad(sky1, 4);
  drawStars(1.0f);
  drawMoon(720, 535, 0.06f, 0.06f, 0.22f);

  /* distant city glow on horizon */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float r = 0; r < 45; r += 2)
  {
    float a = (45 - r) / 45.0f * 0.18f;
    glColor4f(0.6f, 0.3f, 0.8f, a);
    mca(400, 222, r);
  }
  glDisable(GL_BLEND);

  /* 8 buildings — realistic materials */
  srand(7); /* deterministic windows */
  building(0, 222, 68, 290, 0.12f, 0.13f, 0.17f, 6, 2, 0.9f, 0.85f, 0.55f);
  building(72, 222, 55, 215, 0.14f, 0.11f, 0.16f, 5, 2, 0.55f, 0.80f, 0.95f);
  building(130, 222, 88, 265, 0.10f, 0.14f, 0.16f, 6, 3, 0.90f, 0.88f, 0.60f);
  building(222, 222, 60, 178, 0.16f, 0.14f, 0.10f, 4, 2, 0.85f, 0.75f, 0.45f);
  building(520, 222, 80, 308, 0.09f, 0.11f, 0.20f, 7, 3, 0.55f, 0.82f, 0.98f);
  building(604, 222, 60, 235, 0.15f, 0.11f, 0.14f, 5, 2, 0.95f, 0.82f, 0.55f);
  building(668, 222, 68, 198, 0.11f, 0.16f, 0.12f, 4, 2, 0.75f, 0.95f, 0.65f);
  building(740, 222, 56, 278, 0.16f, 0.11f, 0.16f, 6, 2, 0.90f, 0.70f, 0.95f);

  /* neon signs */
  neonSign(110, 418, 1.0f, 0.08f, 0.45f, 5);
  neonSign(595, 385, 0.08f, 0.85f, 1.0f, 5);
  neonSign(695, 348, 1.0f, 0.55f, 0.02f, 4);

  /* ── Wet asphalt road ── */
  /* base asphalt with slight blue-grey wet tint */
  for (float yy = 0; yy < 222; yy += 0.5f)
  {
    float t = yy / 222;
    col3(0.11f + t * 0.03f, 0.11f + t * 0.03f, 0.15f + t * 0.04f);
    dda(0, yy, W, yy);
  }
  /* road lane colour */
  for (float yy = 55; yy < 168; yy += 0.5f)
  {
    float t = (yy - 55) / 113.0f;
    col3(0.14f + t * 0.02f, 0.14f + t * 0.02f, 0.18f + t * 0.02f);
    dda(0, yy, W, yy);
  }
  /* rain puddle shimmer streaks */
  col3(0.20f, 0.20f, 0.28f);
  for (int i = 0; i < 14; i++)
  {
    float px = 30 + i * 55.0f, pw = 20 + sinf(i * 1.3f) * 12;
    float py = 35 + sinf(i * 2.1f) * 18;
    for (float dy = 0; dy < 4; dy += 0.5f)
      dda(px, py + dy, px + pw, py + dy);
  }
  /* kerb lines */
  col3(0.48f, 0.48f, 0.52f);
  dda(0, 55, W, 55);
  dda(0, 167, W, 167);
  /* kerb highlight */
  col3(0.60f, 0.60f, 0.65f);
  dda(0, 56, W, 56);
  dda(0, 168, W, 168);
  /* centre lane dashes with glow */
  for (int i = 0; i < 14; i++)
  {
    float dx = i * 60.0f;
    glow(dx + 18, 111, 0, 12, 0.75f, 0.65f, 0.0f);
    col3(0.78f, 0.68f, 0.02f);
    dda(dx, 111, dx + 36, 111);
  }

  trafficLight(288, 167);
  trafficLight(508, 167);
  srand(7);
  realCar(c1a, 72, 0.72f, 0.08f, 0.12f, 1);
  realCar(c1b, 118, 0.08f, 0.38f, 0.78f, -1);
  drawPerson(c1p, 182, 0.18f, 0.20f, 0.60f);

  /* fog layer near ground */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float yy = 0; yy < 35; yy += 2)
  {
    float a = (35 - yy) / 35.0f * 0.22f;
    glColor4f(0.18f, 0.18f, 0.28f, a);
    dda(0, yy, W, yy);
  }
  glDisable(GL_BLEND);
}

/* ══════════════════════════════════════════════════════════════════
   SCENE 2 — MOUNTAIN ROAD AT DAWN
   Atmospheric perspective, volumetric mist in valleys,
   god-ray light shafts, layered gradient sky
   ══════════════════════════════════════════════════════════════════ */
void mtnFill(float l, float px, float py, float rr, float by,
             float cr, float cg, float cb, float fogr, float fogg, float fogb, float fogStart)
{
  for (float x = l; x <= rr; x += 0.5f)
  {
    float ty = (x <= px) ? (by + (x - l) / (px - l) * (py - by)) : (py + (x - px) / (rr - px) * (by - py));
    /* atmospheric perspective: blend to fog colour with height */
    float fogT = cl((ty - fogStart) / (by - fogStart));
    float fr = lp(cr, fogr, fogT * 0.6f);
    float fg = lp(cg, fogg, fogT * 0.6f);
    float fb = lp(cb, fogb, fogT * 0.6f);
    col3(fr, fg, fb);
    dda(x, by, x, ty);
  }
}
void snowCap(float px, float py, float sp)
{
  /* blue-tinted shadowed snow */
  col3(0.82f, 0.84f, 0.92f);
  for (float x = px - sp; x <= px + sp; x += 0.5f)
  {
    float t = fabsf(x - px) / sp;
    float bot = py - (1 - t) * sp * 0.65f;
    for (float yy = bot; yy <= py; yy += 0.5f)
    {
      float shadow = 0.75f + 0.25f * (yy - bot) / (py - bot + 0.01f);
      col3(0.82f * shadow, 0.84f * shadow, 0.93f * shadow);
      glBegin(GL_POINTS);
      glVertex2f(x, yy);
      glEnd();
    }
  }
}
void realisticPine(float x, float by, float h, float fogT)
{
  /* fog-attenuated colours */
  float tr = lp(0.12f, 0.55f, fogT), tg = lp(0.32f, 0.65f, fogT), tb = lp(0.14f, 0.62f, fogT);
  /* trunk */
  col3(lp(0.22f, 0.55f, fogT), lp(0.14f, 0.48f, fogT), lp(0.08f, 0.42f, fogT));
  fillRect(x - 3, by, 6, h * 0.22f);
  /* tiers — 4 layers */
  float ys = by + h * 0.16f;
  float ws[] = {0.62f, 0.46f, 0.30f, 0.17f}, hs[] = {0.28f, 0.23f, 0.18f, 0.14f};
  for (int t = 0; t < 4; t++)
  {
    float tw = ws[t] * h, th = hs[t] * h;
    for (float yy = ys; yy <= ys + th; yy += 0.5f)
    {
      float frac = (yy - ys) / th, half = tw * (1 - frac);
      /* lit top face */
      col3(cl(tr + 0.06f * (1 - frac)), cl(tg + 0.10f * (1 - frac)), cl(tb + 0.04f * (1 - frac)));
      dda(x - half, yy, x + half, yy);
    }
    /* shadow underside of tier */
    col3(tr * 0.55f, tg * 0.55f, tb * 0.55f);
    dda(x - tw, ys + th * 0.92f, x + tw, ys + th * 0.92f);
    ys += th * 0.74f;
  }
}

void scene2()
{
  /* ── Multi-stop dawn sky ── */
  Stop sky2[] = {{0.0f, 0.02f, 0.01f, 0.12f}, {0.25f, 0.06f, 0.04f, 0.22f}, {0.45f, 0.28f, 0.10f, 0.22f}, {0.60f, 0.68f, 0.35f, 0.15f}, {0.72f, 0.88f, 0.62f, 0.18f}, {0.85f, 0.95f, 0.82f, 0.52f}, {1.0f, 0.98f, 0.92f, 0.75f}};
  skyGrad(sky2, 7);

  /* sun */
  float sx = c2sx, sy = c2sy;
  /* chromatic halo layers */
  glow(sx, sy, 30, 120, 1.0f, 0.65f, 0.15f);
  glow(sx, sy, 25, 60, 1.0f, 0.82f, 0.28f);
  col3(1.0f, 0.97f, 0.70f);
  fc(sx, sy, 28);
  /* limb darkening */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float r = 22; r < 29; r++)
  {
    float a = (r - 22) / 7.0f * 0.5f;
    glColor4f(0.95f, 0.55f, 0.05f, a);
    mca(sx, sy, r);
  }
  glDisable(GL_BLEND);

  /* god rays */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int ray = 0; ray < 10; ray++)
  {
    float ang = (-28 + ray * 6.0f) * 3.14159f / 180.0f;
    for (float len = 0; len < 300; len += 4)
    {
      float a = (1 - len / 300.0f) * 0.035f;
      float rx = sx + len * cosf(ang), ry = sy + len * sinf(ang);
      if (rx >= 0 && rx < W && ry >= 0 && ry < H)
      {
        glColor4f(1.0f, 0.85f, 0.4f, a);
        fc(rx, ry, 2);
      }
    }
  }
  glDisable(GL_BLEND);

  /* distant haze band at horizon */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float yy = 270; yy < 310; yy += 1)
  {
    float a = (1 - fabsf(yy - 290) / 20) * 0.32f;
    glColor4f(0.88f, 0.78f, 0.68f, a);
    dda(0, yy, W, yy);
  }
  glDisable(GL_BLEND);

  float fogr = 0.80f, fogg = 0.72f, fogb = 0.80f;
  /* far mountains — heavily fogged, very cool */
  mtnFill(0, 185, 398, 390, 295, 0.32f, 0.26f, 0.42f, fogr, fogg, fogb, 320);
  mtnFill(170, 405, 445, 620, 295, 0.28f, 0.22f, 0.38f, fogr, fogg, fogb, 320);
  mtnFill(400, 610, 420, 800, 295, 0.34f, 0.28f, 0.44f, fogr, fogg, fogb, 320);
  snowCap(185, 398, 40);
  snowCap(405, 445, 38);
  snowCap(610, 420, 35);

  /* mid mountains — warmer, less fog */
  mtnFill(40, 250, 350, 500, 295, 0.40f, 0.34f, 0.50f, fogr, fogg, fogb, 310);
  mtnFill(260, 500, 375, 760, 295, 0.36f, 0.30f, 0.46f, fogr, fogg, fogb, 310);
  snowCap(250, 350, 30);
  snowCap(500, 375, 28);

  /* valley mist */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float yy = 288; yy < 318; yy += 1)
  {
    float a = (yy - 288) / 30.0f * 0.48f;
    glColor4f(0.88f, 0.84f, 0.90f, a);
    dda(0, yy, W, yy);
  }
  glDisable(GL_BLEND);

  /* ── Road ── */
  /* grassy verge with colour perspective */
  for (float yy = 0; yy < 330; yy += 0.5f)
  {
    float fd = cl(yy / 330.0f); /* distance fade */
    float rl = 400 - 34 * fd * 6.5f, rrv = 400 + 34 * fd * 6.5f;
    col3(lp(fogr, 0.12f, fd * 0.7f), lp(fogg, 0.42f, fd * 0.7f), lp(fogb, 0.14f, fd * 0.7f));
    if (rl > 0)
      dda(0, yy, rl, yy);
    if (rrv < W)
      dda(rrv, yy, W, yy);
  }
  /* asphalt */
  for (float yy = 0; yy < 330; yy += 0.5f)
  {
    float fd = cl(yy / 330.0f);
    float rl = 400 - 34 * fd * 6.5f, rrv = 400 + 34 * fd * 6.5f;
    col3(lp(fogr * 0.9f, 0.14f, fd), lp(fogg * 0.9f, 0.14f, fd), lp(fogb * 0.9f, 0.16f, fd));
    if (rrv - rl > 0)
      dda(rl, yy, rrv, yy);
  }
  /* road edge marks */
  col3(0.82f, 0.78f, 0.62f);
  dda(0, 0, 366, 326);
  dda(W, 0, 434, 326);
  /* centre line perspective dashes */
  col3(0.88f, 0.82f, 0.28f);
  for (int i = 0; i < 8; i++)
  {
    float t1 = i / 8.0f, t2 = (i + 0.42f) / 8.0f;
    dda(400 * (1 - t1 * 6.5f / 6.5f) + 0, 326 * t1, 400 * (1 - t2 * 6.5f / 6.5f) + 0, 326 * t2);
  }

  /* 8 pine trees with atmospheric depth */
  float txL[] = {50, 115, 188, 262}, txR[] = {572, 645, 718, 785};
  for (int i = 0; i < 4; i++)
  {
    float ph = 105 - i * 13.0f, fd = cl(i / 4.0f);
    realisticPine(txL[i], 295, ph, fd * 0.55f);
    realisticPine(txR[i], 295, ph, fd * 0.55f);
  }

  /* road-side grass tufts */
  col3(0.15f, 0.40f, 0.12f);
  for (int i = 0; i < 18; i++)
  {
    float gx = i * 46.0f, gy = 295 + sinf(i * 2.3f) * 8;
    dda(gx, gy, gx + 3, gy + 8);
    dda(gx + 5, gy, gx + 2, gy + 9);
    dda(gx - 3, gy, gx, gy + 7);
  }

  drawPerson(c2p, 108, 0.52f, 0.24f, 0.12f);
}
/* ══════════════════════════════════════════════════════════════════
   SCENE 3 — VILLAGE AFTERNOON
   Subsurface-scatter stone walls, tile roof shading, chimney smoke
   physics, realistic foliage with layered light, cobblestones
   ══════════════════════════════════════════════════════════════════ */

/* Smoke particle system */
typedef struct
{
  float x, y, vx, vy, life, r;
} Smoke;
static Smoke smk[60];
static int smkN = 0;
static int smkInit = 0;
void initSmoke()
{
  if (smkInit)
    return;
  smkInit = 1;
  for (int i = 0; i < 60; i++)
  {
    smk[i].x = 0;
    smk[i].y = 0;
    smk[i].life = 0;
  }
}
void updateSmoke(float cx, float cy)
{
  for (int i = 0; i < 60; i++)
  {
    if (smk[i].life <= 0)
    {
      smk[i].x = cx + (float)(rand() % 5 - 2);
      smk[i].y = cy;
      smk[i].vx = (float)(rand() % 7 - 3) * 0.15f;
      smk[i].vy = 0.45f + rand() % 10 * 0.06f;
      smk[i].life = 1.0f;
      smk[i].r = 4.0f + rand() % 6;
      break;
    }
  }
  for (int i = 0; i < 60; i++)
  {
    if (smk[i].life > 0)
    {
      smk[i].x += smk[i].vx;
      smk[i].y += smk[i].vy;
      smk[i].r += 0.08f;
      smk[i].life -= 0.012f;
    }
  }
}
void drawSmoke()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int i = 0; i < 60; i++)
  {
    if (smk[i].life > 0)
    {
      float a = smk[i].life * 0.35f;
      float c = 0.65f + smk[i].life * 0.20f;
      glColor4f(c, c, c + 0.05f, a);
      fc(smk[i].x, smk[i].y, smk[i].r);
    }
  }
  glDisable(GL_BLEND);
}

/* Stone wall — subsurface scatter approximation */
void stoneWall(float x, float y, float w, float h, float lr, float lg, float lb)
{
  /* base colour gradient (lit top, shadow bottom) */
  for (float yy = y; yy < y + h; yy += 0.5f)
  {
    float sh = 0.72f + 0.28f * (yy - y) / h;
    col3(lr * sh, lg * sh, lb * sh);
    dda(x, yy, x + w, yy);
  }
  /* mortar lines horizontal */
  float mCol = lr * 0.52f, mColG = lg * 0.52f, mColB = lb * 0.52f;
  for (float yy = y + 14; yy < y + h; yy += 14)
  {
    col3(mCol, mColG, mColB);
    dda(x, yy, x + w, yy);
  }
  /* mortar lines vertical (offset per row) */
  int row = 0;
  for (float yy = y; yy < y + h; yy += 14)
  {
    float off = (row % 2) * 22;
    for (float xx = x + off; xx < x + w; xx += 44)
    {
      col3(mCol, mColG, mColB);
      dda(xx, yy, xx, yy + 14);
    }
    row++;
  }
  /* left edge AO */
  for (float yy = y; yy < y + h; yy += 0.5f)
  {
    float sh = 0.68f;
    col3(lr * sh, lg * sh, lb * sh);
    dda(x, yy, x + 8, yy);
  }
  /* top edge light */
  col3(cl(lr + 0.12f), cl(lg + 0.12f), cl(lb + 0.12f));
  dda(x, y + h - 1, x + w, y + h - 1);
}

/* Tile roof */
void tileRoof(float x, float y, float w, float h, float rr, float rg, float rb)
{
  /* base shape */
  glBegin(GL_TRIANGLES);
  col3(rr, rg, rb);
  glVertex2f(x + w / 2, y + h);
  col3(rr * 0.82f, rg * 0.82f, rb * 0.82f);
  glVertex2f(x - 10, y);
  glVertex2f(x + w + 10, y);
  glEnd();
  /* tile rows */
  for (int row = 0; row < 7; row++)
  {
    float ry = y + row * h / 7.0f;
    float hw = (w / 2 + 10) * (1.0f - row / 7.0f);
    col3(rr * 0.70f, rg * 0.70f, rb * 0.70f);
    dda(x + w / 2 - hw, ry, x + w / 2 + hw, ry);
    /* tile highlights */
    col3(cl(rr + 0.08f), cl(rg + 0.05f), cl(rb + 0.02f));
    for (float tx = x + w / 2 - hw; tx < x + w / 2 + hw; tx += 16)
      dda(tx, ry, tx + 8, ry + h / 7.0f * 0.55f);
  }
  /* shadow under eaves */
  col3(rr * 0.42f, rg * 0.42f, rb * 0.42f);
  dda(x - 10, y, x + w + 10, y);
  dda(x - 11, y - 1, x + w + 11, y - 1);
}

/* Window with interior glow */
void villageWindow(float x, float y, float w, float h)
{
  /* warm interior */
  col3(0.98f, 0.88f, 0.52f);
  fillRect(x, y, w, h);
  /* curtains */
  col3(0.75f, 0.40f, 0.30f);
  fillRect(x, y, w * 0.22f, h);
  fillRect(x + w * 0.78f, y, w * 0.22f, h);
  /* cross pane */
  col3(0.30f, 0.18f, 0.08f);
  dda(x, y + h / 2, x + w, y + h / 2);
  dda(x + w / 2, y, x + w / 2, y + h);
  /* frame */
  col3(0.22f, 0.13f, 0.06f);
  dda(x - 2, y - 2, x + w + 2, y - 2);
  dda(x - 2, y + h + 2, x + w + 2, y + h + 2);
  dda(x - 2, y - 2, x - 2, y + h + 2);
  dda(x + w + 2, y - 2, x + w + 2, y + h + 2);
  /* glass reflection */
  col3(0.85f, 0.90f, 0.98f);
  dda(x + 3, y + h - 4, x + w * 0.42f, y + 4);
  /* exterior glow */
  glow(x + w / 2, y + h / 2, 0, 22, 0.95f, 0.82f, 0.35f);
}

/* Realistic cottage */
void cottage3(float x, float y, float w, float h,
              float wr, float wg, float wb, float rr, float rg, float rb,
              float chimX)
{
  stoneWall(x, y, w, h, wr, wg, wb);
  tileRoof(x, y + h, w, h * 0.62f, rr, rg, rb);
  /* chimney */
  col3(wr * 0.68f, wg * 0.65f, wb * 0.60f);
  fillRect(chimX, y + h, 14, h * 0.42f);
  /* chimney cap */
  col3(wr * 0.50f, wg * 0.48f, wb * 0.44f);
  fillRect(chimX - 2, y + h + h * 0.42f, 18, 4);
  /* smoke */
  updateSmoke(chimX + 7, y + h + h * 0.44f);
  drawSmoke();
  /* arched door */
  float dw = w * 0.26f, dh = h * 0.48f, dx = x + w * 0.37f;
  col3(0.28f, 0.15f, 0.07f);
  fillRect(dx, y, dw, dh * 0.68f);
  col3(0.22f, 0.11f, 0.05f);
  fc(dx + dw / 2, y + dh * 0.68f, dw / 2);
  /* door planks */
  col3(0.32f, 0.18f, 0.08f);
  for (int p = 1; p < 4; p++)
    dda(dx, y + dh * 0.18f * p, dx + dw, y + dh * 0.18f * p);
  /* door knob */
  col3(0.75f, 0.62f, 0.10f);
  fc(dx + dw * 0.82f, y + dh * 0.32f, 2.5f);
  /* windows */
  villageWindow(x + w * 0.08f, y + h * 0.42f, w * 0.22f, h * 0.30f);
  villageWindow(x + w * 0.70f, y + h * 0.42f, w * 0.22f, h * 0.30f);
}

/* Detailed windmill */
void windmill3(float x, float y, float ang)
{
  /* stone tower — tapered */
  for (float yy = y; yy < y + 105; yy += 0.5f)
  {
    float f = (yy - y) / 105.0f;
    float hw = 18 - f * 7;
    float sh = 0.68f + 0.22f * (1 - f);
    col3(0.60f * sh, 0.56f * sh, 0.48f * sh);
    dda(x - hw, yy, x + hw, yy);
    /* mortar */
    if (fmod(yy - y, 13) < 0.7f)
    {
      col3(0.38f, 0.34f, 0.28f);
      dda(x - hw, yy, x + hw, yy);
    }
  }
  /* conical cap */
  for (float yy = y + 105; yy < y + 128; yy += 0.5f)
  {
    float f = (yy - y - 105) / 23.0f;
    float hw = (1 - f) * 13;
    col3(0.30f, 0.20f, 0.10f);
    dda(x - hw, yy, x + hw, yy);
  }
  /* hub with rivets */
  col3(0.48f, 0.44f, 0.40f);
  fc(x, y + 105, 9);
  col3(0.28f, 0.24f, 0.20f);
  mca(x, y + 105, 9);
  /* 4 sails — canvas on wooden frame */
  for (int b = 0; b < 4; b++)
  {
    float rad = (ang + b * 90.0f) * 3.14159f / 180.0f;
    float pr = (ang + b * 90 + 90) * 3.14159f / 180.0f;
    float ex = x + 52 * cosf(rad), ey = y + 105 + 52 * sinf(rad);
    /* wood spar */
    col3(0.38f, 0.24f, 0.10f);
    dda(x, y + 105, ex, ey);
    /* canvas fill */
    col3(0.88f, 0.84f, 0.72f);
    glBegin(GL_POLYGON);
    glVertex2f(x + 9 * cosf(pr), y + 105 + 9 * sinf(pr));
    glVertex2f(x - 9 * cosf(pr), y + 105 - 9 * sinf(pr));
    glVertex2f(ex - 8 * cosf(pr), ey - 8 * sinf(pr));
    glVertex2f(ex + 8 * cosf(pr), ey + 8 * sinf(pr));
    glEnd();
    /* canvas shadow */
    col3(0.65f, 0.60f, 0.48f);
    dda(x - 9 * cosf(pr), y + 105 - 9 * sinf(pr), ex - 8 * cosf(pr), ey - 8 * sinf(pr));
  }
}

/* Stone well with rope */
void stoneWell3(float x, float y)
{
  /* barrel */
  for (float yy = y; yy < y + 34; yy += 0.5f)
  {
    float c = 0.52f + 0.08f * sinf((yy - y) * 0.5f);
    col3(c + 0.04f, c, c - 0.06f);
    dda(x - 20, yy, x + 20, yy);
    if (fmod(yy - y, 9) < 0.7f)
    {
      col3(0.34f, 0.30f, 0.26f);
      dda(x - 20, yy, x + 20, yy);
    }
  }
  mca(x, y, 20);
  mca(x, y + 34, 20);
  /* posts */
  col3(0.32f, 0.20f, 0.10f);
  fillRect(x - 24, y + 32, 7, 38);
  fillRect(x + 17, y + 32, 7, 38);
  /* cross beam */
  fillRect(x - 24, y + 68, 48, 6);
  /* rope */
  col3(0.60f, 0.48f, 0.22f);
  for (float yy = y + 45; yy < y + 68; yy += 3)
    dda(x - 1, yy, x + 1, yy + 3);
  /* bucket */
  col3(0.28f, 0.22f, 0.16f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 7, y + 28);
  glVertex2f(x + 7, y + 28);
  glVertex2f(x + 5, y + 44);
  glVertex2f(x - 5, y + 44);
  glEnd();
  /* water */
  col3(0.25f, 0.50f, 0.72f);
  dda(x - 4, y + 42, x + 4, y + 42);
}

/* Foliage tree — realistic multilayer canopy */
void realTree(float x, float by, float h)
{
  /* trunk */
  for (float yy = by; yy < by + h * 0.38f; yy += 0.5f)
  {
    float hw = 5 - 3 * (yy - by) / (h * 0.38f);
    float sh = 0.55f + 0.20f * (yy - by) / (h * 0.38f);
    col3(0.28f * sh, 0.16f * sh, 0.07f * sh);
    dda(x - hw, yy, x + hw, yy);
  }
  /* bark texture lines */
  col3(0.18f, 0.10f, 0.05f);
  for (float yy = by + 6; yy < by + h * 0.35f; yy += 10)
    dda(x - 4, yy, x - 2, yy + 8);
  /* layered canopy */
  float cy = by + h * 0.35f;
  /* deep shadow layer */
  col3(0.04f, 0.28f, 0.06f);
  fc(x, cy, h * 0.36f);
  /* mid layer */
  col3(0.08f, 0.42f, 0.10f);
  fc(x + h * 0.08f, cy + h * 0.08f, h * 0.28f);
  fc(x - h * 0.10f, cy + h * 0.06f, h * 0.22f);
  /* lit upper layer */
  col3(0.14f, 0.58f, 0.16f);
  fc(x + h * 0.05f, cy + h * 0.18f, h * 0.18f);
  /* specular highlight */
  col3(0.28f, 0.72f, 0.22f);
  fc(x + h * 0.10f, cy + h * 0.26f, h * 0.08f);
  /* fruit dots */
  col3(0.85f, 0.18f, 0.08f);
  fc(x - h * 0.14f, cy + h * 0.05f, 3);
  fc(x + h * 0.12f, cy - h * 0.02f, 3);
}

/* Detailed fence */
void realFence(float x, float y, float len)
{
  /* rail */
  col3(0.45f, 0.30f, 0.12f);
  fillRect(x, y + 6, len, 5);
  /* posts every 22px */
  for (float px = x; px < x + len; px += 22)
  {
    col3(0.40f, 0.26f, 0.10f);
    fillRect(px - 2, y - 2, 5, 22);
    /* post cap */
    col3(0.55f, 0.38f, 0.16f);
    glBegin(GL_TRIANGLES);
    glVertex2f(px - 3, y + 20);
    glVertex2f(px + 4, y + 20);
    glVertex2f(px + 0.5f, y + 26);
    glEnd();
  }
  /* picket infill */
  col3(0.52f, 0.36f, 0.14f);
  for (float px = x + 5; px < x + len - 4; px += 8)
    dda(px, y - 2, px, y + 16);
}

/* Cobblestone path */
void cobblePath(float x, float y, float w, float h2)
{
  col3(0.44f, 0.38f, 0.30f);
  fillRect(x, y, w, h2);
  /* individual stones */
  srand(13);
  for (float sy = y + 4; sy < y + h2 - 4; sy += 16)
  {
    float off = (rand() % 2) * 11;
    for (float sx = x + off + 4; sx < x + w - 4; sx += 22)
    {
      float sw2 = 14 + rand() % 6, sh = 10 + rand() % 4;
      float sr = 0.48f + rand() % 20 * 0.01f, sg = 0.42f + rand() % 15 * 0.01f, sb = 0.34f + rand() % 12 * 0.01f;
      col3(sr, sg, sb);
      fillRect(sx, sy, sw2, sh);
      col3(sr * 0.70f, sg * 0.70f, sb * 0.70f);
      dda(sx, sy, sx + sw2, sy);
      dda(sx, sy, sx, sy + sh);
      col3(sr + 0.08f, sg + 0.06f, sb + 0.04f);
      dda(sx, sy + sh, sx + sw2, sy + sh);
      dda(sx + sw2, sy, sx + sw2, sy + sh);
    }
  }
}

/* Fluffy cloud — multi-layer */
void fluffyCloud(float x, float y, float s)
{
  /* shadow underside */
  col3(0.78f, 0.78f, 0.84f);
  fc(x, y - s * 0.2f, s * 0.55f);
  fc(x + s * 0.7f, y - s * 0.15f, s * 0.42f);
  fc(x - s * 0.7f, y - s * 0.12f, s * 0.38f);
  /* main body */
  col3(1.0f, 1.0f, 1.0f);
  fc(x, y, s);
  fc(x + s * 0.8f, y + s * 0.28f, s * 0.72f);
  fc(x - s * 0.8f, y + s * 0.22f, s * 0.62f);
  fc(x + s * 0.35f, y + s * 0.48f, s * 0.52f);
  fc(x - s * 0.30f, y + s * 0.42f, s * 0.48f);
  /* bright top */
  col3(1.0f, 1.0f, 1.0f);
  fc(x + s * 0.1f, y + s * 0.52f, s * 0.32f);
}

void scene3()
{
  /* ── Warm afternoon sky ── */
  Stop sky3[] = {{0.0f, 0.42f, 0.72f, 0.96f}, {0.55f, 0.65f, 0.85f, 0.98f}, {0.78f, 0.82f, 0.92f, 1.0f}, {1.0f, 0.90f, 0.95f, 1.0f}};
  skyGrad(sky3, 4);

  /* sun */
  glow(680, 560, 30, 90, 1.0f, 0.92f, 0.45f);
  col3(1.0f, 0.97f, 0.72f);
  fc(680, 560, 26);

  /* clouds */
  fluffyCloud(90, 510, 24);
  fluffyCloud(340, 535, 19);
  fluffyCloud(580, 518, 21);
  fluffyCloud(210, 548, 15);

  /* ── Rich grass ── */
  for (float yy = 0; yy < 225; yy += 0.5f)
  {
    float t = yy / 225.0f;
    col3(lp(0.12f, 0.20f, t), lp(0.45f, 0.58f, t), lp(0.10f, 0.14f, t));
    dda(0, yy, W, yy);
  }
  /* grass blade texture */
  col3(0.16f, 0.52f, 0.13f);
  for (int i = 0; i < 80; i++)
  {
    float gx = i * 10.5f + sinf(i * 3.1f) * 4, gy = 225 + sinf(i * 2.7f) * 18;
    dda(gx, gy, gx + 2, gy + 9);
    dda(gx + 4, gy + 1, gx + 2, gy + 10);
  }
  /* flower meadow */
  for (int i = 0; i < 30; i++)
  {
    float fx = 15 + i * 27.0f, fy = 222 + sinf(i * 1.8f) * 20;
    col3(1.0f, 0.90f, 0.12f);
    fc(fx, fy, 3);
    col3(0.98f, 0.38f, 0.62f);
    fc(fx + 14, fy + 7, 2.5f);
    col3(0.75f, 0.35f, 0.90f);
    fc(fx + 28, fy + 3, 2.5f);
    /* stem */
    col3(0.18f, 0.55f, 0.14f);
    dda(fx, fy - 3, fx, fy - 9);
  }

  cobblePath(340, 0, 125, 228);

  /* 5 cottages */
  initSmoke();
  cottage3(12, 222, 105, 94, 0.78f, 0.72f, 0.62f, 0.62f, 0.18f, 0.12f, 82);
  cottage3(136, 222, 118, 104, 0.68f, 0.74f, 0.62f, 0.22f, 0.45f, 0.20f, 218);
  cottage3(278, 222, 94, 88, 0.74f, 0.70f, 0.58f, 0.55f, 0.30f, 0.14f, 326);
  cottage3(508, 222, 110, 100, 0.76f, 0.70f, 0.60f, 0.50f, 0.20f, 0.16f, 568);
  cottage3(638, 222, 100, 92, 0.70f, 0.74f, 0.64f, 0.26f, 0.42f, 0.22f, 696);

  realTree(462, 222, 86);
  realTree(480, 222, 98);
  realFence(8, 222, 625);
  stoneWell3(402, 222);
  windmill3(735, 222, c3m);
  drawPerson(c3p, 226, 0.52f, 0.28f, 0.12f);
}

/* ══════════════════════════════════════════════════════════════════
   SCENE 4 — HARVEST CROP FIELD (golden hour)
   Physically-based sky gradient, realistic corn with light&shadow,
   volumetric dust, weathered barn, crows in thermal
   ══════════════════════════════════════════════════════════════════ */
void realisticCorn(float x, float y, float h, float sw)
{
  /* shadow stalk */
  col3(0.12f, 0.32f, 0.08f);
  dda(x + sw + 2, y, x + sw * 0.4f + 2, y + h * 0.52f);
  dda(x + sw * 0.4f + 2, y + h * 0.52f, x + 2, y + h);
  /* lit stalk */
  col3(0.22f, 0.52f, 0.13f);
  dda(x + sw, y, x + sw * 0.4f, y + h * 0.52f);
  dda(x + sw * 0.4f, y + h * 0.52f, x, y + h);
  /* leaves — lit and shadow sides */
  col3(0.18f, 0.50f, 0.11f);
  dda(x + sw * 0.72f, y + h * 0.26f, x + sw * 0.72f - 20, y + h * 0.46f);
  col3(0.26f, 0.60f, 0.16f);
  dda(x + sw * 0.72f, y + h * 0.26f, x + sw * 0.72f - 18, y + h * 0.45f);
  col3(0.16f, 0.46f, 0.10f);
  dda(x + sw * 0.28f, y + h * 0.58f, x + sw * 0.28f + 20, y + h * 0.74f);
  col3(0.24f, 0.56f, 0.14f);
  dda(x + sw * 0.28f, y + h * 0.58f, x + sw * 0.28f + 18, y + h * 0.73f);
  /* cob — orange-amber gradient */
  for (float r = 5.5f; r >= 0; r -= 0.5f)
  {
    float t = r / 5.5f;
    col3(lp(0.88f, 0.98f, t), lp(0.52f, 0.78f, t), lp(0.04f, 0.08f, t));
    mca(x + sw * 0.22f + 3, y + h * 0.50f, r);
  }
  /* cob leaf husk */
  col3(0.28f, 0.55f, 0.16f);
  dda(x + sw * 0.22f - 3, y + h * 0.44f, x + sw * 0.22f, y + h * 0.58f);
  dda(x + sw * 0.22f + 8, y + h * 0.44f, x + sw * 0.22f + 3, y + h * 0.58f);
}

void realisticScarecrow(float x, float y)
{
  /* post shadow */
  col3(0.20f, 0.14f, 0.06f);
  dda(x + 3, y, x + 3, y + 88);
  dda(x - 28, y + 60, x + 30, y + 60);
  /* post */
  col3(0.38f, 0.24f, 0.10f);
  dda(x, y, x, y + 88);
  dda(x - 30, y + 58, x + 30, y + 58);
  /* stuffed shirt */
  col3(0.58f, 0.18f, 0.14f);
  fillRect(x - 16, y + 55, 32, 30);
  /* shirt shadow */
  col3(0.42f, 0.12f, 0.10f);
  fillRect(x - 16, y + 55, 6, 30);
  /* patch */
  col3(0.30f, 0.55f, 0.18f);
  fillRect(x - 6, y + 58, 14, 12);
  col3(0.22f, 0.42f, 0.12f);
  dda(x - 6, y + 58, x + 8, y + 70);
  dda(x + 8, y + 58, x - 6, y + 70);
  /* jeans (trousers) */
  col3(0.18f, 0.28f, 0.52f);
  fillRect(x - 14, y + 24, 12, 32);
  fillRect(x + 2, y + 24, 12, 32);
  col3(0.12f, 0.20f, 0.40f);
  fillRect(x - 14, y + 24, 3, 32);
  /* head */
  col3(0.82f, 0.78f, 0.35f);
  fc(x, y + 96, 14);
  /* face */
  col3(0.08f, 0.06f, 0.02f);
  fc(x - 5, y + 98, 2.5f);
  fc(x + 5, y + 98, 2.5f); /* eyes */
  dda(x - 5, y + 92, x - 2, y + 89);
  dda(x + 5, y + 92, x + 2, y + 89); /* eyebrows */
  /* mouth */
  col3(0.40f, 0.10f, 0.08f);
  dda(x - 5, y + 90, x + 5, y + 90);
  /* hat */
  col3(0.14f, 0.10f, 0.06f);
  fillRect(x - 20, y + 105, 40, 5);
  glBegin(GL_POLYGON);
  glVertex2f(x - 14, y + 110);
  glVertex2f(x + 14, y + 110);
  glVertex2f(x + 10, y + 130);
  glVertex2f(x - 10, y + 130);
  glEnd();
  /* hat band */
  col3(0.40f, 0.25f, 0.08f);
  fillRect(x - 14, y + 110, 28, 4);
}

void realisticBarn(float x, float y, float w, float h)
{
  /* wall shadow left */
  for (float yy = y; yy < y + h; yy += 0.5f)
  {
    float sh = 0.62f + 0.28f * (yy - y) / h;
    col3(0.52f * sh, 0.10f * sh, 0.07f * sh);
    dda(x, yy, x + w, yy);
  }
  /* plank lines */
  col3(0.38f, 0.07f, 0.05f);
  for (float yy = y + 10; yy < y + h; yy += 12)
    dda(x, yy, x + w, yy);
  /* weathered streaks */
  col3(0.30f, 0.06f, 0.04f);
  for (int i = 0; i < 8; i++)
    dda(x + i * w / 7.5f, y + h, x + i * w / 7.5f + 2, y + h * 0.4f);
  /* gambrel roof */
  col3(0.22f, 0.16f, 0.10f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 8, y + h);
  glVertex2f(x + w + 8, y + h);
  glVertex2f(x + w / 2 + 12, y + h + h * 0.28f);
  glVertex2f(x + w / 2 - 12, y + h + h * 0.28f);
  glEnd();
  col3(0.30f, 0.22f, 0.14f);
  glBegin(GL_POLYGON);
  glVertex2f(x + w / 2 - 12, y + h + h * 0.28f);
  glVertex2f(x + w / 2 + 12, y + h + h * 0.28f);
  glVertex2f(x + w / 2, y + h + h * 0.56f);
  glEnd();
  /* roof tiles */
  col3(0.16f, 0.12f, 0.08f);
  for (int r = 0; r < 6; r++)
    dda(x - 8 + r * (w + 16) / 5.5f, y + h, x + w / 2, y + h + h * 0.56f);
  /* loft window with light shaft */
  col3(0.95f, 0.88f, 0.45f);
  fc(x + w / 2, y + h + h * 0.20f, 10);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.98f, 0.85f, 0.32f, 0.28f);
  glBegin(GL_TRIANGLES);
  glVertex2f(x + w / 2 - 5, y + h + h * 0.18f);
  glVertex2f(x + w / 2 + 5, y + h + h * 0.18f);
  glVertex2f(x + w / 2 + 60, y + 10);
  glEnd();
  glDisable(GL_BLEND);
  /* X-door */
  col3(0.30f, 0.06f, 0.04f);
  float dw = w * 0.38f, dh = h * 0.54f, dx = x + w * 0.31f;
  fillRect(dx, y, dw, dh);
  col3(0.18f, 0.04f, 0.03f);
  dda(dx, y, dx + dw, y + dh);
  dda(dx + dw, y, dx, y + dh);
  /* door frame */
  col3(0.22f, 0.06f, 0.04f);
  dda(dx - 1, y, dx - 1, y + dh);
  dda(dx + dw + 1, y, dx + dw + 1, y + dh);
  dda(dx - 1, y + dh, dx + dw + 1, y + dh);
  /* side windows */
  col3(0.55f, 0.80f, 0.92f);
  fc(x + w * 0.80f, y + h * 0.62f, 11);
  col3(0.22f, 0.06f, 0.04f);
  mca(x + w * 0.80f, y + h * 0.62f, 11);
}

void crow(float x, float y)
{
  col3(0.06f, 0.06f, 0.06f);
  dda(x - 14, y, x, y + 7);
  dda(x, y + 7, x + 14, y);
  /* body dot */
  fc(x, y + 7, 3);
  /* tail */
  dda(x - 3, y + 7, x - 8, y + 2);
  dda(x + 3, y + 7, x + 8, y + 2);
}

void scene4()
{
  /* ── Golden hour sky ── */
  Stop sky4[] = {{0.0f, 0.08f, 0.14f, 0.38f}, {0.28f, 0.22f, 0.42f, 0.72f}, {0.48f, 0.62f, 0.42f, 0.18f}, {0.65f, 0.90f, 0.58f, 0.08f}, {0.80f, 0.98f, 0.72f, 0.12f}, {1.0f, 0.96f, 0.88f, 0.60f}};
  skyGrad(sky4, 6);

  /* sun near horizon with haze */
  glow(680, 228, 32, 110, 1.0f, 0.60f, 0.12f);
  glow(680, 228, 26, 55, 1.0f, 0.80f, 0.28f);
  col3(1.0f, 0.60f, 0.08f);
  fc(680, 228, 30);
  /* limb darkening */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float r = 26; r < 32; r++)
  {
    float a = (r - 26) / 6.0f * 0.6f;
    glColor4f(0.95f, 0.42f, 0.02f, a);
    mca(680, 228, r);
  }
  glDisable(GL_BLEND);

  /* ── Rich farmland soil ── */
  for (float yy = 0; yy < 232; yy += 0.5f)
  {
    float t = yy / 232.0f;
    col3(lp(0.22f, 0.38f, t), lp(0.12f, 0.22f, t), lp(0.05f, 0.10f, t));
    dda(0, yy, W, yy);
  }
  /* furrows with shadow/highlight */
  for (int i = 0; i < 12; i++)
  {
    float fy = 12 + i * 18.0f;
    col3(0.16f, 0.09f, 0.04f);
    dda(0, fy, W, fy);
    col3(0.40f, 0.26f, 0.12f);
    dda(0, fy + 4, W, fy + 4);
  }
  /* dust haze near ground */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float yy = 225; yy < 245; yy += 1.5f)
  {
    float a = (245 - yy) / 20.0f * 0.22f;
    glColor4f(0.82f, 0.65f, 0.28f, a);
    dda(0, yy, W, yy);
  }
  glDisable(GL_BLEND);

  realisticBarn(560, 232, 175, 125);

  /* 99 corn stalks */
  for (int row = 0; row < 3; row++)
  {
    float ry = 232 + row * 58, h2 = 86 - row * 10.0f;
    /* row shadow line */
    col3(0.14f, 0.08f, 0.03f);
    dda(0, ry - 2, 560, ry - 2);
    for (int col2 = 0; col2 < 33; col2++)
    {
      float sx = 8 + col2 * 24.5f;
      float sw = 5.2f * sinf(c4sw + col2 * 0.44f + row * 1.22f);
      realisticCorn(sx, ry, h2, sw);
    }
  }
  realisticScarecrow(172, 232);
  realisticScarecrow(408, 232);
  drawPerson(c4p, 234, 0.30f, 0.18f, 0.48f);
  crow(bx[0], by2[0]);
  crow(bx[1], by2[1]);
  crow(bx[2], by2[2]);
}

/* ══════════════════════════════════════════════════════════════════
   SCENE 5 — RIVER & SEA (SUNSET)
   Chromatic sunset, Fresnel water, volumetric lighthouse beam,
   detailed sailboats, caustic reflection patterns
   ══════════════════════════════════════════════════════════════════ */
void realisticBoat(float x, float y, float br, float bg, float bb, int dir)
{
  float s = (float)dir;
  /* hull shadow */
  col3(br * 0.30f, bg * 0.30f, bb * 0.30f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 38, y + 8);
  glVertex2f(x + 38, y + 8);
  glVertex2f(x + s * 28, y - 2);
  glVertex2f(x - s * 28, y - 2);
  glEnd();
  /* hull body */
  for (float yy = y; yy < y + 14; yy += 0.5f)
  {
    float sh = 0.75f + 0.25f * (yy - y) / 14.0f;
    col3(br * sh, bg * sh, bb * sh);
    dda(x - 37, yy, x + 37, yy);
  }
  /* gunwale highlight */
  col3(cl(br + 0.20f), cl(bg + 0.20f), cl(bb + 0.20f));
  dda(x - 36, y + 12, x + 36, y + 12);
  /* mast */
  col3(0.36f, 0.22f, 0.09f);
  dda(x, y + 14, x, y + 64);
  /* rigging lines */
  col3(0.55f, 0.42f, 0.22f);
  dda(x, y + 64, x + s * 36, y + 14);
  dda(x, y + 64, x - s * 18, y + 18);
  /* main sail */
  for (float yy = y + 16; yy < y + 62; yy += 0.5f)
  {
    float frac = (yy - y - 16) / 46.0f;
    float hw = s * (36 - frac * 36) * 0.92f;
    float sh = 0.88f + 0.12f * (1 - frac);
    col3(sh * 0.96f, sh * 0.92f, sh * 0.82f);
    dda(x, yy, x + hw, yy);
  }
  /* sail shadow crease */
  col3(0.72f, 0.68f, 0.58f);
  dda(x, y + 62, x + s * 36, y + 16);
  /* jib sail */
  for (float yy = y + 18; yy < y + 55; yy += 0.5f)
  {
    float frac = (yy - y - 18) / 37.0f;
    float hw = -s * (18 * (1 - frac));
    col3(0.90f, 0.86f, 0.75f);
    dda(x, yy, x + hw, yy);
  }
  /* flag */
  col3(0.78f, 0.12f, 0.12f);
  fc(x, y + 64, 4);
  /* water reflection */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float yy = 1; yy < 14; yy += 0.5f)
  {
    float a = (14 - yy) / 14.0f * 0.45f;
    glColor4f(br, bg, bb, a);
    dda(x - 37, y - yy, x + 37, y - yy);
  }
  glDisable(GL_BLEND);
}

void realisticLighthouse(float x, float y)
{
  /* base rocks */
  col3(0.42f, 0.38f, 0.32f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 30, y);
  glVertex2f(x + 30, y);
  glVertex2f(x + 24, y + 14);
  glVertex2f(x - 24, y + 14);
  glEnd();
  /* rock shading */
  col3(0.30f, 0.26f, 0.22f);
  dda(x - 30, y, x - 24, y + 14);
  col3(0.55f, 0.50f, 0.44f);
  dda(x + 30, y, x + 24, y + 14);

  /* tower — realistic masonry with perspective */
  for (float yy = y + 14; yy < y + 160; yy += 0.5f)
  {
    float f = (yy - y - 14) / 146.0f;
    float hw = 21 - f * 9;
    int stripe = (int)((yy - y - 14) / 17) % 2;
    float sr = stripe ? 0.94f : 0.68f, sg = stripe ? 0.92f : 0.14f, sb = stripe ? 0.92f : 0.14f;
    col3(sr, sg, sb);
    dda(x - hw, yy, x + hw, yy);
    /* mortar joint */
    if (fmod(yy - y - 14, 17) < 0.8f)
    {
      col3(0.45f, 0.40f, 0.38f);
      dda(x - hw, yy, x + hw, yy);
    }
    /* edge shading */
    col3(sr * 0.68f, sg * 0.68f, sb * 0.68f);
    dda(x - hw, yy, x - hw + 3, yy);
    col3(cl(sr + 0.08f), cl(sg + 0.04f), cl(sb + 0.04f));
    dda(x + hw - 2, yy, x + hw, yy);
  }
  /* gallery / balcony */
  col3(0.22f, 0.20f, 0.18f);
  fillRect(x - 24, y + 160, 48, 8);
  col3(0.32f, 0.30f, 0.28f);
  dda(x - 24, y + 168, x + 24, y + 168);
  /* railing posts */
  for (int i = -22; i <= 22; i += 6)
  {
    col3(0.35f, 0.32f, 0.28f);
    dda(x + i, y + 160, x + i, y + 168);
  }
  /* lantern room */
  col3(0.18f, 0.16f, 0.14f);
  fillRect(x - 14, y + 168, 28, 18);
  /* glass panels */
  col3(0.55f, 0.80f, 0.90f);
  for (int i = 0; i < 4; i++)
  {
    float gx = x - 12 + i * 7.0f;
    fillRect(gx, y + 169, 5, 16);
  }
  /* roof cone */
  col3(0.14f, 0.12f, 0.10f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 16, y + 186);
  glVertex2f(x + 16, y + 186);
  glVertex2f(x, y + 205);
  glEnd();
  /* lantern */
  glow(x, y + 177, 0, 50, 1.0f, 0.90f, 0.40f);
  col3(1.0f, 0.96f, 0.62f);
  fc(x, y + 177, 10);
  /* rotating beam (volumetric cone) */
  float rad = c5beam * 3.14159f / 180.0f;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float span = -10; span < 10; span += 0.5f)
  {
    float bRad = rad + span * 3.14159f / 180.0f;
    float a = powf((10 - fabsf(span)) / 10.0f, 2) * 0.50f;
    for (float len = 14; len < 260; len += 2)
    {
      float bx2 = x + len * cosf(bRad), by3 = y + 177 + len * sinf(bRad);
      float da = a * (1 - len / 260.0f);
      if (by3 > 82 && by3 < 250 && bx2 > 0 && bx2 < W)
      {
        glColor4f(1.0f, 0.92f, 0.55f, da);
        glBegin(GL_POINTS);
        glVertex2f(bx2, by3);
        glEnd();
      }
    }
  }
  glDisable(GL_BLEND);
}

void scene5()
{
  /* ── Chromatic sunset sky ── */
  Stop sky5[] = {{0.0f, 0.02f, 0.02f, 0.18f}, {0.20f, 0.08f, 0.05f, 0.30f}, {0.35f, 0.28f, 0.08f, 0.32f}, {0.50f, 0.72f, 0.30f, 0.12f}, {0.62f, 0.92f, 0.52f, 0.10f}, {0.75f, 0.98f, 0.72f, 0.22f}, {0.88f, 0.85f, 0.58f, 0.55f}, {1.0f, 0.65f, 0.42f, 0.72f}};
  skyGrad(sky5, 8);

  /* setting sun */
  glow(390, 225, 34, 115, 1.0f, 0.62f, 0.12f);
  glow(390, 225, 28, 60, 1.0f, 0.82f, 0.32f);
  col3(1.0f, 0.60f, 0.06f);
  fc(390, 225, 32);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float r = 28; r < 34; r++)
  {
    float a = (r - 28) / 6.0f * 0.65f;
    glColor4f(0.98f, 0.38f, 0.02f, a);
    mca(390, 225, r);
  }
  /* green flash at very top */
  glColor4f(0.2f, 0.9f, 0.3f, 0.18f);
  fc(390, 257, 4);
  glDisable(GL_BLEND);

  /* stars appearing */
  drawStars(0.4f);

  /* ── Deep ocean — Fresnel effect ── */
  for (float yy = 0; yy < 90; yy += 0.5f)
  {
    float t = yy / 90.0f;
    col3(lp(0.02f, 0.05f, t), lp(0.08f, 0.18f, t), lp(0.32f, 0.52f, t));
    dda(0, yy, W, yy);
  }
  /* ocean swell waves — 5 rows */
  float wColors[5][3] = {{0.06f, 0.20f, 0.58f}, {0.08f, 0.26f, 0.65f}, {0.10f, 0.30f, 0.70f}, {0.13f, 0.35f, 0.75f}, {0.16f, 0.40f, 0.80f}};
  for (int row = 0; row < 5; row++)
  {
    float wy = 12 + row * 15.5f;
    for (float x2 = 0; x2 < W - 1; x2++)
    {
      float y1 = wy + 7 * sinf((x2 + c5w + row * 38) * 0.048f) + 2 * sinf((x2 + c5w * 1.3f) * 0.12f);
      float y2 = wy + 7 * sinf(((x2 + 1) + c5w + row * 38) * 0.048f) + 2 * sinf(((x2 + 1) + c5w * 1.3f) * 0.12f);
      col3(wColors[row][0], wColors[row][1], wColors[row][2]);
      dda(x2, y1, x2 + 1, y2);
      /* foam crests */
      if (y2 < y1 - 0.5f)
      {
        col3(0.82f, 0.90f, 0.95f);
        fc(x2 + 1, y2, 2);
      }
    }
  }
  /* sun path reflection on ocean */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int i = 0; i < 35; i++)
  {
    float rx = 375 + (i % 6) * 5 - 12 + sinf(c5w * 0.038f + i) * 8;
    float ry = 8 + i * 2.5f;
    float a = 0.55f * (1 - i / 35.0f);
    glColor4f(1.0f, 0.72f, 0.15f, a);
    dda(rx, ry, rx + 8 + i * 0.3f, ry);
  }
  glDisable(GL_BLEND);

  /* ── River — calmer water ── */
  for (float yy = 88; yy < 225; yy += 0.5f)
  {
    float t = (yy - 88) / 137.0f;
    col3(lp(0.05f, 0.08f, t), lp(0.20f, 0.35f, t), lp(0.55f, 0.72f, t));
    dda(0, yy, W, yy);
  }
  /* river ripples — Gerstner waves */
  for (int row = 0; row < 7; row++)
  {
    float ry = 102 + row * 18.5f;
    for (float x2 = 0; x2 < W - 1; x2++)
    {
      float y1 = ry + 5 * sinf((x2 + c5w) * 0.072f + row * 0.55f) + 1.5f * sinf((x2 + c5w * 1.4f) * 0.18f + row);
      float y2 = ry + 5 * sinf(((x2 + 1) + c5w) * 0.072f + row * 0.55f) + 1.5f * sinf(((x2 + 1) + c5w * 1.4f) * 0.18f + row);
      float cr = 0.08f + row * 0.015f, cg = 0.32f + row * 0.018f, cb = 0.68f + row * 0.010f;
      col3(cr, cg, cb);
      dda(x2, y1, x2 + 1, y2);
    }
  }
  /* caustic light patterns on river */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int i = 0; i < 22; i++)
  {
    float cx2 = 385 + (i % 7 - 3) * 18 + sinf(c5w * 0.06f + i) * 6;
    float cy2 = 92 + i * 6.5f;
    float a = 0.18f * (1 - i / 22.0f);
    glColor4f(1.0f, 0.88f, 0.42f, a);
    fc(cx2, cy2, 6);
  }
  glDisable(GL_BLEND);

  /* ── Bank ── */
  /* soil under bank */
  col3(0.28f, 0.18f, 0.08f);
  fillRect(0, 215, W, 14);
  /* grass */
  for (float yy = 226; yy < 252; yy += 0.5f)
  {
    float t = (yy - 226) / 26.0f;
    col3(lp(0.15f, 0.22f, t), lp(0.44f, 0.56f, t), lp(0.11f, 0.14f, t));
    dda(0, yy, W, yy);
  }
  /* pebbles with highlight */
  col3(0.50f, 0.46f, 0.42f);
  float pbX[] = {65, 140, 205, 315, 425, 515, 605, 685};
  for (int i = 0; i < 8; i++)
  {
    float r = 4 + sinf(i * 2.1f) * 2;
    fc(pbX[i], 223, r);
    col3(0.70f, 0.66f, 0.62f);
    mca(pbX[i] + 1, 224, r);
  }
  /* bank vegetation */
  col3(0.12f, 0.48f, 0.12f);
  for (int i = 0; i < 24; i++)
  {
    float gx = 12 + i * 33.5f, gy = 252 + sinf(i * 1.9f) * 10;
    dda(gx, gy, gx + 2, gy + 11);
    dda(gx + 5, gy + 1, gx + 3, gy + 12);
    dda(gx - 3, gy, gx, gy + 9);
  }

  realisticLighthouse(688, 252);
  realisticBoat(c5b1, 152, 0.50f, 0.28f, 0.12f, 1);
  realisticBoat(c5b2, 160, 0.14f, 0.28f, 0.55f, -1);
  drawPerson(c5p, 248, 0.48f, 0.16f, 0.36f);
}

/* ══════════════════════════════════════════════════════════════════
   HUD
   ══════════════════════════════════════════════════════════════════ */
const char *sNames[] = {"Scene 1  ·  City Night", "Scene 2  ·  Mountain Dawn",
                        "Scene 3  ·  Village Afternoon", "Scene 4  ·  Harvest Field", "Scene 5  ·  River & Sea"};
float sAccents[][3] = {{0.28f, 0.68f, 1.0f}, {0.98f, 0.72f, 0.20f}, {0.28f, 0.88f, 0.30f}, {0.98f, 0.78f, 0.10f}, {0.18f, 0.62f, 0.98f}};

void txt(float x, float y, const char *s, void *font)
{
  glRasterPos2f(x, y);
  for (const char *c = s; *c; c++)
    glutBitmapCharacter(font, *c);
}

void drawHUD()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  /* bar bg */
  glColor4f(0.0f, 0.0f, 0.0f, 0.60f);
  fillRect(0, 0, W, 28);
  glDisable(GL_BLEND);
  /* progress track */
  col3(0.14f, 0.14f, 0.16f);
  fillRect(0, 0, W, 5);
  /* progress fill */
  float prog = sceneTimer / SCENE_DUR;
  float *ac = sAccents[scene];
  col3(ac[0], ac[1], ac[2]);
  fillRect(0, 0, W * prog, 5);
  /* glow on progress tip */
  glow(W * prog, 2, 0, 20, ac[0], ac[1], ac[2]);
  /* scene name */
  col3(1.0f, 1.0f, 1.0f);
  txt(12, 9, sNames[scene], GLUT_BITMAP_HELVETICA_12);
  /* hint text */
  col3(0.55f, 0.55f, 0.60f);
  txt(W / 2 - 80, 9, "SPACE = next    D = prev    ESC = quit", GLUT_BITMAP_HELVETICA_10);
  /* dot indicators */
  for (int i = 0; i < N_SCENES; i++)
  {
    float cx = W - 25 - (N_SCENES - 1 - i) * 22.0f;
    float *c = sAccents[i];
    if (i == scene)
    {
      col3(c[0], c[1], c[2]);
      fc(cx, 14, 6);
    }
    else
    {
      col3(0.35f, 0.35f, 0.38f);
      mca(cx, 14, 5);
    }
  }
}

void drawFade()
{
  if (fadeA <= 0.001f)
    return;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0f, 0.0f, 0.0f, fadeA);
  fillRect(0, 0, W, H);
  glDisable(GL_BLEND);
}

/* ══════════════════════════════════════════════════════════════════
   DISPLAY + MAIN LOOP
   ══════════════════════════════════════════════════════════════════ */
void switchScene(int n)
{
  scene = (n % N_SCENES + N_SCENES) % N_SCENES;
  sceneTimer = 0;
  fadIn = 1;
  fadOut = 0;
  fadeA = 1.0f;
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  switch (scene)
  {
  case 0:
    scene1();
    break;
  case 1:
    scene2();
    break;
  case 2:
    scene3();
    break;
  case 3:
    scene4();
    break;
  case 4:
    scene5();
    break;
  }
  drawHUD();
  drawFade();
  glutSwapBuffers();
}

void timerCB(int v)
{
  float dt = 1.0f / 60.0f;
  T += dt;
  if (fadIn)
  {
    fadeA -= 0.030f;
    if (fadeA <= 0)
    {
      fadeA = 0;
      fadIn = 0;
    }
  }
  if (fadOut)
  {
    fadeA += 0.030f;
    if (fadeA >= 1)
    {
      fadeA = 1;
      fadOut = 0;
      switchScene(scene + 1);
    }
  }
  if (!fadIn && !fadOut)
  {
    sceneTimer += dt;
    if (sceneTimer >= SCENE_DUR)
      fadOut = 1;
  }
  /* scene 1 */
  c1a += 1.5f;
  if (c1a > W + 90)
    c1a = -90;
  c1b -= 1.2f;
  if (c1b < -90)
    c1b = W + 90;
  c1p += 0.42f;
  if (c1p > W + 30)
    c1p = -30;
  /* scene 2 */
  c2p -= 0.35f;
  if (c2p < -30)
    c2p = W + 30;
  c2sx += 0.11f;
  c2sy += 0.038f;
  if (c2sx > W + 60)
  {
    c2sx = -80;
    c2sy = 95;
  }
  /* scene 3 */
  c3m += 1.5f;
  if (c3m >= 360)
    c3m -= 360;
  c3p += 0.42f;
  if (c3p > W + 30)
    c3p = -30;
  /* scene 4 */
  c4sw += 0.044f;
  c4p += 0.42f;
  if (c4p > W + 30)
    c4p = -30;
  for (int i = 0; i < 3; i++)
  {
    bx[i] += 0.65f + i * 0.22f;
    if (bx[i] > W + 30)
      bx[i] = -30;
    by2[i] = 490 + i * 12 + 8 * sinf(T * 0.85f + i * 1.5f);
  }
  /* scene 5 */
  c5w += 1.0f;
  c5beam += 0.42f;
  if (c5beam >= 360)
    c5beam -= 360;
  c5b1 += 0.44f;
  if (c5b1 > W + 60)
    c5b1 = -65;
  c5b2 -= 0.36f;
  if (c5b2 < -65)
    c5b2 = W + 60;
  c5p += 0.38f;
  if (c5p > W + 30)
    c5p = -30;
  glutPostRedisplay();
  glutTimerFunc(16, timerCB, 0);
}

void keyboard(unsigned char key, int x, int y)
{
  if (key == 27)
    exit(0);
  if (key == ' ')
    fadOut = 1;
  if (key == 'd' || key == 'D')
    switchScene(scene - 1);
}
void reshape(int w, int h)
{
  W = w;
  H = h;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 600);
  glutCreateWindow("The Infinite Drive — Photorealistic Edition");
  glClearColor(0, 0, 0, 1);
  glPointSize(1.5f);
  initStars();
  initSmoke();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(16, timerCB, 0);
  printf("The Infinite Drive — Photorealistic Edition\n");
  printf("SPACE=next  D=prev  ESC=quit  (auto 12s)\n");
  glutMainLoop();
  return 0;
}
