#include "drive.h"

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
  /* green flash */
  glColor4f(0.2f, 0.9f, 0.3f, 0.18f);
  fc(390, 257, 4);
  glDisable(GL_BLEND);

  /* stars appearing */
  drawStars(0.4f);

  /* ── 1. Deep Ocean (Farthest Layer) ── */
  // Moved up so it appears behind everything but the sky
  for (float yy = 222; yy < 320; yy += 0.5f)
  {
    float t = (yy - 222) / 98.0f;
    col3(lp(0.02f, 0.05f, t), lp(0.08f, 0.18f, t), lp(0.32f, 0.52f, t));
    dda(0, yy, W, yy);
  }

  /* ocean swell waves */
  float wColors[5][3] = {{0.06f, 0.20f, 0.58f}, {0.08f, 0.26f, 0.65f}, {0.10f, 0.30f, 0.70f}, {0.13f, 0.35f, 0.75f}, {0.16f, 0.40f, 0.80f}};
  for (int row = 0; row < 5; row++)
  {
    float wy = 230 + row * 12.0f; // Adjusted height to be behind road
    for (float x2 = 0; x2 < W - 1; x2++)
    {
      float y1 = wy + 5 * sinf((x2 + c5w + row * 38) * 0.048f);
      float y2 = wy + 5 * sinf(((x2 + 1) + c5w + row * 38) * 0.048f);
      col3(wColors[row][0], wColors[row][1], wColors[row][2]);
      dda(x2, y1, x2 + 1, y2);
    }
  }

  /* sun path reflection on ocean */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (int i = 0; i < 25; i++)
  {
    float rx = 375 + (i % 6) * 5 - 12 + sinf(c5w * 0.038f + i) * 8;
    float ry = 225 + i * 2.5f;
    float a = 0.55f * (1 - i / 25.0f);
    glColor4f(1.0f, 0.72f, 0.15f, a);
    dda(rx, ry, rx + 8 + i * 0.3f, ry);
  }
  glDisable(GL_BLEND);

  /* ── 2. Bank & Coastline ── */
  /* soil bank sitting right behind road top edge (y=222) */
  col3(0.28f, 0.18f, 0.08f);
  fillRect(0, 222, W, 8);

  /* pebbles */
  col3(0.50f, 0.46f, 0.42f);
  float pbX[] = {65, 140, 205, 315, 425, 515, 605, 685};
  for (int i = 0; i < 8; i++) {
    fc(pbX[i], 225, 3);
  }

  /* ── 3. Objects in Water ── */
  realisticLighthouse(700, 230);
  realisticBoat(c5b1, 260, 0.50f, 0.28f, 0.12f, 1);
  realisticBoat(c5b2, 280, 0.14f, 0.28f, 0.55f, -1);

  /* ── 4. Canonical foreground (Road & Cars) ── */
  /* This overwrites everything below y=222, putting the sea "behind" it */
  for (float yy = 0; yy < 222; yy += 0.5f)
  {
    float t = yy / 222.0f;
    col3(0.11f + t * 0.03f, 0.11f + t * 0.03f, 0.15f + t * 0.04f);
    dda(0, yy, W, yy);
  }
  for (float yy = 55; yy < 168; yy += 0.5f)
  {
    float t = (yy - 55) / 113.0f;
    col3(0.14f + t * 0.02f, 0.14f + t * 0.02f, 0.18f + t * 0.02f);
    dda(0, yy, W, yy);
  }
  col3(0.48f, 0.48f, 0.52f);
  dda(0, 55, W, 55);
  dda(0, 167, W, 167);
  col3(0.60f, 0.60f, 0.65f);
  dda(0, 56, W, 56);
  dda(0, 168, W, 168);
  for (int i = 0; i < 14; i++)
  {
    float dx = i * 60.0f;
    glow(dx + 18, 111, 0, 12, 0.75f, 0.65f, 0.0f);
    col3(0.78f, 0.68f, 0.02f);
    dda(dx, 111, dx + 36, 111);
  }
  srand(7);
  realCar(c1a, 72, 0.72f, 0.08f, 0.12f, 1);
  realCar(c1b, 118, 0.08f, 0.38f, 0.78f, -1);
  drawPerson(c1p, 182, 0.18f, 0.20f, 0.60f);
}
