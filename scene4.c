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
  col3(0.12f * shine, 0.35f * shine, 0.08f);
  glBegin(GL_POLYGON);
  glVertex2f(x - 1.2f * perspective, y);
  glVertex2f(x + 1.2f * perspective, y);
  glVertex2f(x + sway, y + h);
  glEnd();

  float cobX = x + sway + 1.5f;
  float cobY = y + h * 0.5f;
  col3(0.95f * shine, 0.75f * shine, 0.1f);
  fc(cobX, cobY, 4.5f * perspective);
  glDisable(GL_BLEND);
}

/* ══════════════════════════════════════════════════════════════════
   2. MAIN SCENE FUNCTION
   ══════════════════════════════════════════════════════════════════ */

void scene4()
{
  glPushMatrix();

  // Cinematic Camera
  float camShift = sinf(T * 0.03f) * 40.0f;
  glTranslatef(camShift, 0, 0);

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

  // --- ROAD FROM COTTAGE ---
  glEnable(GL_BLEND);
  glBegin(GL_POLYGON);
  glColor4f(0.35f, 0.25f, 0.15f, 1.0f);
  glVertex2f(W * 0.18f, 232); // Near cottage door
  glVertex2f(W * 0.24f, 232);
  glColor4f(0.45f, 0.35f, 0.25f, 0.8f);
  glVertex2f(W * 0.65f, 0); // Sweeps across to foreground
  glVertex2f(W * 0.35f, 0);
  glEnd();
  glDisable(GL_BLEND);

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

  glPopMatrix();
}
