#include "drive.h"

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

/* ══════════════════════════════════════════════════════════════════
   DISPLAY + MAIN LOOP
   ══════════════════════════════════════════════════════════════════ */
void switchScene(int n)
{
  scene = (n % N_SCENES + N_SCENES) % N_SCENES;
  sceneTimer = 0;
  /* No black fade — instant cut to the next scene */
  fadIn = 0;
  fadOut = 0;
  fadeA = 0.0f;
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
  glutSwapBuffers();
}

void timerCB(int v)
{
  (void)v;
  float dt = 1.0f / 60.0f;
  T += dt;
  sceneTimer += dt;
  if (sceneTimer >= SCENE_DUR)
    switchScene(scene + 1);
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
  c3carX += 1.15f;
  if (c3carX > W + 120)
    c3carX = -140;
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
  (void)x;
  (void)y;
  if (key == 27)
    exit(0);
  if (key == ' ')
    switchScene(scene + 1);
  if (key == 'd' || key == 'D')
    switchScene(scene - 1);
}
void reshape(int w, int h)
{
  /* Render into a fixed virtual canvas so fullscreen doesn't "stick left".
     Most scene art is authored around 800x600 coordinates. We keep that
     coordinate system and letterbox/pillarbox the viewport to preserve
     aspect ratio while centering the picture. */
  const int VW = 800, VH = 600;
  W = VW;
  H = VH;

  float sx = (float)w / (float)VW;
  float sy = (float)h / (float)VH;
  float s = (sx < sy) ? sx : sy;
  int vpw = (int)(VW * s);
  int vph = (int)(VH * s);
  int vpx = (w - vpw) / 2;
  int vpy = (h - vph) / 2;
  glViewport(vpx, vpy, vpw, vph);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, VW, 0, VH);
  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(1920, 1080);
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
