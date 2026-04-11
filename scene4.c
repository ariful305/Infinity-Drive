#include "drive.h"

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
