#include "drive.h"

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
