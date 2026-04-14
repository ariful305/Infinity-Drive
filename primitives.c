#include "drive.h"

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
float cl(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
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
/* ── Sky scan-line gradient ─────────────────────────────────────── */
void skyGrad(Stop *stops, int n)
{
  if (n < 2)
    return;
  for (int y = 0; y < H; y++)
  {
    float t = (float)y / H;
    int lo = 0;
    if (t <= stops[0].yf)
      lo = 0;
    else if (t >= stops[n - 1].yf)
      lo = n - 2;
    else
    {
      for (int i = 0; i < n - 1; i++)
      {
        if (t >= stops[i].yf && t < stops[i + 1].yf)
        {
          lo = i;
          break;
        }
      }
    }
    float span = stops[lo + 1].yf - stops[lo].yf;
    float f = (span > 0) ? (t - stops[lo].yf) / span : 0;
    if (f < 0)
      f = 0;
    if (f > 1)
      f = 1;
    col3(lp(stops[lo].r, stops[lo + 1].r, f),
         lp(stops[lo].g, stops[lo + 1].g, f),
         lp(stops[lo].b, stops[lo + 1].b, f));
    dda(0, y, W, y);
  }
}

/* ── Glow halo around a point ───────────────────────────────────── */
void glow(float cx, float cy, float r, float R, float gr, float gg, float gb)
{
  if (R <= r + 1e-6f)
    return;
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
  glow(x, y, 24, 65, 0.90f, 0.90f, 0.70f);
  col3(0.955f, 0.952f, 0.880f);
  fc(x, y, 23);
  col3(0.860f, 0.855f, 0.780f);
  fc(x + 5, y - 4, 18);
  col3(skyr, skyg, skyb);
  fc(x + 10, y + 6, 18);
  col3(0.820f, 0.818f, 0.750f);
  fc(x - 5, y + 6, 7);
  fc(x + 6, y - 2, 5);
}

/* ── Realistic stick person with shadow ────────────────────────── */
void drawPerson(float x, float y, float shR, float shG, float shB)
{
  /* keep the same anchor (x,y) but draw with more volume + shading */
  float sw = sinf(T * 4.2f) * 6.5f;

  /* ground contact shadow */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (float r = 2; r <= 13; r += 0.5f)
  {
    float a = (13 - r) / 13.0f * 0.18f;
    glColor4f(0, 0, 0, a);
    mca(x, y - 14, r);
  }
  glDisable(GL_BLEND);

  /* legs (slight thickness) */
  col3(shR * 0.80f, shG * 0.80f, shB * 0.80f);
  dda(x - 1, y + 2, x - 9, y - 14 + sw * 0.35f);
  dda(x + 1, y + 2, x + 9, y - 14 - sw * 0.35f);
  col3(shR * 0.60f, shG * 0.60f, shB * 0.60f);
  dda(x, y + 2, x - 8, y - 14 + sw * 0.35f);
  dda(x, y + 2, x + 8, y - 14 - sw * 0.35f);
  /* feet */
  col3(0.10f, 0.10f, 0.11f);
  dda(x - 10, y - 14 + sw * 0.35f, x - 5, y - 14 + sw * 0.35f);
  dda(x + 10, y - 14 - sw * 0.35f, x + 5, y - 14 - sw * 0.35f);

  /* torso */
  for (float yy = y + 2; yy <= y + 22; yy += 0.5f)
  {
    float t = (yy - (y + 2)) / 20.0f;
    float hw = 3.5f - t * 1.2f;
    float sh = 0.70f + 0.30f * t;
    col3(shR * sh, shG * sh, shB * sh);
    dda(x - hw, yy, x + hw, yy);
  }
  col3(shR * 0.55f, shG * 0.55f, shB * 0.55f);
  dda(x - 3, y + 14, x + 3, y + 14); /* subtle waist crease */
  /* shirt highlight */
  col3(cl(shR + 0.10f), cl(shG + 0.10f), cl(shB + 0.10f));
  dda(x - 2, y + 20, x + 2, y + 20);

  /* arms */
  col3(shR * 0.78f, shG * 0.78f, shB * 0.78f);
  dda(x - 2, y + 18, x - 11, y + 7 + sw);
  dda(x + 2, y + 18, x + 11, y + 7 - sw);
  col3(shR * 0.62f, shG * 0.62f, shB * 0.62f);
  dda(x - 1, y + 18, x - 10, y + 7 + sw);
  dda(x + 1, y + 18, x + 10, y + 7 - sw);

  /* head (skin gradient + hair) */
  for (float r = 6.5f; r >= 0.5f; r -= 0.5f)
  {
    float t = (6.5f - r) / 6.5f;
    col3(lp(0.86f, 0.96f, t), lp(0.66f, 0.78f, t), lp(0.48f, 0.60f, t));
    mca(x, y + 27, r);
  }
  col3(0.14f, 0.09f, 0.05f);
  fc(x, y + 33, 5);

  /* face hints */
  col3(0.10f, 0.08f, 0.07f);
  fc(x - 2.2f, y + 28, 1.0f);
  fc(x + 2.2f, y + 28, 1.0f);
  col3(0.55f, 0.25f, 0.22f);
  dda(x - 1.2f, y + 25.5f, x + 1.2f, y + 25.5f);
}
