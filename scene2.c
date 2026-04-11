#include "drive.h"

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
    float ty;
    if (x <= px)
    {
      float d = px - l;
      ty = (fabsf(d) > 1e-6f) ? (by + (x - l) / d * (py - by)) : by;
    }
    else
    {
      float d = rr - px;
      ty = (fabsf(d) > 1e-6f) ? (py + (x - px) / d * (by - py)) : py;
    }
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
