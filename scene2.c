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
    /* ── REFINED DAWN SKY ── */
    // Added deeper oranges and soft teals for a more realistic pre-sunrise transition
    Stop sky2[] = {
        {0.00f, 0.05f, 0.02f, 0.15f}, // Deep Navy Top
        {0.30f, 0.12f, 0.08f, 0.28f}, // Muted Purple
        {0.55f, 0.35f, 0.15f, 0.25f}, // Burning Orange Horizon
        {0.75f, 0.92f, 0.58f, 0.35f}, // Soft Gold
        {1.00f, 0.98f, 0.92f, 0.85f}  // Near-Sun Brilliance
    };
    skyGrad(sky2, 5);



    /* ── MOUNTAIN RANGES ── */
    float fogr = 0.82f, fogg = 0.78f, fogb = 0.85f;

    // Far Mountains: Deeply recessed into the atmosphere
    mtnFill(0, 185, 410, 390, 295, 0.3f, 0.25f, 0.45f, fogr, fogg, fogb, 310);
    mtnFill(170, 405, 455, 620, 295, 0.25f, 0.22f, 0.42f, fogr, fogg, fogb, 310);
    mtnFill(400, 610, 430, 800, 295, 0.32f, 0.28f, 0.48f, fogr, fogg, fogb, 310);
    snowCap(185, 410, 45);
    snowCap(405, 455, 42);
    snowCap(610, 430, 38);

    // Mid Mountains: Sharper detail, warmer "alpenglow" tint
    mtnFill(40, 250, 365, 500, 295, 0.45f, 0.38f, 0.55f, fogr, fogg, fogb, 305);
    mtnFill(260, 505, 385, 760, 295, 0.42f, 0.35f, 0.52f, fogr, fogg, fogb, 305);
    snowCap(250, 365, 32);
    snowCap(505, 385, 30);

    /* ── VOLUMETRIC VALLEY MIST ── */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (float yy = 290; yy < 325; yy += 0.8f)
    {
        float t = (yy - 290) / 35.0f;
        float a = sinf(t * 3.14f) * 0.45f; // Peaks in the middle of the band
        glColor4f(0.92f, 0.90f, 0.95f, a);
        dda(0, yy, W, yy);
    }
    glDisable(GL_BLEND);

    /* ── PINE TREES (FOG-ADAPTIVE) ── */
    float txL[] = {45, 120, 195, 270}, txR[] = {565, 640, 715, 790};
    for (int i = 0; i < 4; i++)
    {
        float ph = 115 - i * 15.0f; // Varied heights
        float fd = cl(i / 4.0f) * 0.6f;
        realisticPine(txL[i], 295, ph, fd);
        realisticPine(txR[i], 295, ph, fd);
    }

    /* ── GRASS ── */
    col3(0.12f, 0.35f, 0.15f);
    for (int i = 0; i < 24; i++)
    {
        float gx = i * 35.0f + (rand() % 10);
        float gy = 295 + sinf(i * 1.8f) * 6;
        dda(gx, gy, gx + 2, gy + 10); // More vertical tufts
        dda(gx + 4, gy, gx + 1, gy + 11);
    }

    /* ── CANONICAL FOREGROUND (ROAD/CARS/PERSON) ── */
    // Keep road and car logic exactly as provided for consistency
    for (float yy = 0; yy < 222; yy += 0.5f) {
        float t = yy / 222.0f;
        col3(0.11f + t * 0.03f, 0.11f + t * 0.03f, 0.15f + t * 0.04f);
        dda(0, yy, W, yy);
    }
    for (float yy = 55; yy < 168; yy += 0.5f) {
        float t = (yy - 55) / 113.0f;
        col3(0.14f + t * 0.02f, 0.14f + t * 0.02f, 0.18f + t * 0.02f);
        dda(0, yy, W, yy);
    }
    col3(0.48f, 0.48f, 0.52f); dda(0, 55, W, 55); dda(0, 167, W, 167);
    col3(0.60f, 0.60f, 0.65f); dda(0, 56, W, 56); dda(0, 168, W, 168);
    for (int i = 0; i < 14; i++) {
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
