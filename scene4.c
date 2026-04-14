#include "drive.h"

/* ══════════════════════════════════════════════════════════════════
   1. ASSET DEFINITIONS
   ══════════════════════════════════════════════════════════════════ */

// Realistic Tree with trunk and swaying canopy
void drawCinematicTree(float x, float y, float scale)
{
  float wind = sinf(T * 1.1f + x * 0.01f) * 3.0f;

  // Trunk
  col3(0.25f, 0.15f, 0.05f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 4 * scale, y);
  glVertex2f(x + 4 * scale, y);
  glVertex2f(x + 2 * scale + wind * 0.2f, y + 40 * scale);
  glVertex2f(x - 2 * scale + wind * 0.2f, y + 40 * scale);
  glEnd();

  // Foliage layers (Dark to Light)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  float leafColors[3][3] = {{0.05f, 0.2f, 0.05f}, {0.1f, 0.35f, 0.1f}, {0.2f, 0.5f, 0.15f}};
  for (int i = 0; i < 3; i++)
  {
    glColor4f(leafColors[i][0], leafColors[i][1], leafColors[i][2], 0.9f);
    float ox = x + wind * (i + 1) * 0.5f;
    float oy = y + (45 + i * 15) * scale;
    fc(ox, oy, (35 - i * 5) * scale);
    // Add small leaf highlights
    glColor4f(1.0f, 1.0f, 0.5f, 0.2f);
    fc(ox + 5, oy + 5, (10) * scale);
  }
  glDisable(GL_BLEND);
}

void realisticScarecrow(float x, float y)
{
  col3(0.40f, 0.25f, 0.12f);
  dda(x, y, x, y + 80);
  dda(x - 25, y + 55, x + 25, y + 55);
  col3(0.85f, 0.80f, 0.40f);
  fc(x, y + 88, 12);
}

void drawRealisticBird(float bx, float by, float base_scale, float flap_phase)
{
  float wing_flap = sinf(T * 5.0f + flap_phase) * 12.0f;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.05f, 0.05f, 0.05f, 0.9f);
  glBegin(GL_POLYGON); // Wing Curve
  glVertex2f(bx, by);
  glVertex2f(bx - 18 * base_scale, by + wing_flap);
  glVertex2f(bx, by + 3);
  glVertex2f(bx + 18 * base_scale, by + wing_flap);
  glEnd();
  fc(bx, by + 1, 3 * base_scale);
  glDisable(GL_BLEND);
}

void drawRealisticBirdFlock(float cx, float cy)
{
  for (int i = 0; i < 7; i++)
  {
    float offsetX = i * 35.0f;
    float offsetY = (i % 2 == 0 ? 1 : -1) * (i * 15.0f);
    drawRealisticBird(cx - offsetX, cy + offsetY, 1.0f - (i * 0.05f), i * 0.4f);
  }
}

void drawSmallCottage(float x, float y)
{
  // Shadow under cottage
  col3(0.1f, 0.05f, 0.02f);
  fillRect(x - 5, y - 5, 80, 10);

  col3(0.55f, 0.45f, 0.35f); // Walls
  fillRect(x, y, 70, 50);

  // Door
  col3(0.2f, 0.1f, 0.05f);
  fillRect(x + 25, y, 20, 35);

  glBegin(GL_TRIANGLES);
  col3(0.45f, 0.35f, 0.15f); // Roof
  glVertex2f(x - 15, y + 45);
  glVertex2f(x + 85, y + 45);
  col3(0.75f, 0.65f, 0.35f);
  glVertex2f(x + 35, y + 90);
  glEnd();
}

void drawCinematicCorn(float x, float y, float h, float sw, float perspective)
{
  float time_sync = T * 1.3f + x * 0.005f;
  float sway = sinf(time_sync) * sw;
  float shine = 1.0f + 0.3f * sinf(time_sync * 2.0f);

  glEnable(GL_BLEND);
  /* stalk with subtle gradient */
  for (float yy = 0; yy < h; yy += 0.8f)
  {
    float t = yy / h;
    float w = (1.4f - t * 0.6f) * perspective;
    float g = (0.34f + 0.22f * (1 - t)) * shine;
    float r = (0.10f + 0.05f * (1 - t)) * shine;
    col3(r, g, 0.08f * shine);
    dda(x + sway * t - w, y + yy, x + sway * t + w, y + yy);
  }
  /* leaf blades */
  col3(0.10f * shine, 0.40f * shine, 0.08f);
  dda(x, y + h * 0.45f, x - 14 * perspective + sway * 0.5f, y + h * 0.30f);
  dda(x, y + h * 0.55f, x + 16 * perspective + sway * 0.6f, y + h * 0.40f);
  col3(0.14f * shine, 0.52f * shine, 0.10f);
  dda(x, y + h * 0.62f, x - 10 * perspective + sway * 0.7f, y + h * 0.50f);
  dda(x, y + h * 0.70f, x + 12 * perspective + sway * 0.8f, y + h * 0.56f);
  /* highlight edge */
  col3(0.22f * shine, 0.68f * shine, 0.16f * shine);
  dda(x + sway * 0.95f, y + h, x + sway * 0.35f, y + h * 0.55f);

  float cobX = x + sway + 1.5f;
  float cobY = y + h * 0.5f;
  col3(0.95f * shine, 0.75f * shine, 0.1f);
  fc(cobX, cobY, 4.5f * perspective);
  /* tassel */
  col3(0.72f, 0.62f, 0.28f);
  for (int i = 0; i < 4; i++)
    dda(x + sway, y + h - 4, x + sway + (i - 1.5f) * 3.0f * perspective, y + h + 10 + i * 2);
  glDisable(GL_BLEND);
}

/* ══════════════════════════════════════════════════════════════════
   2. MAIN SCENE FUNCTION
   ══════════════════════════════════════════════════════════════════ */

void scene4()
{
  glPushMatrix();

  // Sky
  Stop sky4[] = {{0.0f, 0.02f, 0.08f, 0.25f}, {0.6f, 0.95f, 0.5f, 0.2f}, {1.0f, 1.0f, 0.8f, 0.4f}};
  skyGrad(sky4, 3);

  // Sun
  glow(W * 0.8f, 270, 40, 120, 1.0f, 0.7f, 0.3f);
  col3(1.0f, 0.95f, 0.8f);
  fc(W * 0.8f, 270, 35);

  // Ground
  col3(0.18f, 0.12f, 0.06f);
  fillRect(0, 0, W, 232);

  // House and Trees
  drawSmallCottage(W * 0.15f, 232);
  drawCinematicTree(W * 0.08f, 232, 1.1f);
  drawCinematicTree(W * 0.85f, 232, 1.3f);
  drawCinematicTree(W * 0.92f, 225, 0.9f);

  // Field Rows
  for (int row = 0; row < 3; row++)
  {
    float rowY = 195 - (row * 75);
    float scale = 0.55f + (row * 0.25f);
    for (int col = 0; col < 35; col++)
    {
      float sx = (col * (W / 32.0f)) - 30;
      // Gap logic for the sweeping road
      float roadX = lp(W * 0.21f, W * 0.50f, 1.0f - (rowY / 232.0f));
      if (sx > roadX - 60 && sx < roadX + 60)
        continue;

      float wave = (6.0f + row * 2.0f) * sinf(T * 1.3f + sx * 0.005f);
      drawCinematicCorn(sx, rowY, 110 * scale, wave, scale);
    }
  }

  // Scarecrows
  realisticScarecrow(W * 0.05f, 160);
  realisticScarecrow(W * 0.75f, 180);

  // Animated Flock - Start offset changed so they appear instantly
  float flockX = fmodf((T + 10.0f) * 45.0f, W + 600) - 200;
  float flockY = H * 0.82f + sinf(T * 0.5f) * 20.0f;
  drawRealisticBirdFlock(flockX, flockY);

  // Atmosphere
  glEnable(GL_BLEND);
  for (int i = 0; i < 40; i++)
  {
    float px = fmodf(i * 88.0f + T * 15.0f, W);
    float py = fmodf(i * 120.0f, H);
    glColor4f(1.0f, 0.9f, 0.7f, 0.12f);
    fc(px, py, 1.5f);
  }
  glDisable(GL_BLEND);

  /* ── Canonical foreground (match Scene 1 placement exactly) ── */
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

  glPopMatrix();
}
