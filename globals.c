#include "drive.h"

int W = 800, H = 600;

int scene = 0, N_SCENES = 5;
float sceneTimer = 0, SCENE_DUR = 12.0f;
float fadeA = 1.0f;
int fadIn = 1, fadOut = 0;
float T = 0.0f;

float c1a = 80, c1b = 720, c1p = 60;
float c2p = 400, c2sx = -80, c2sy = 95;
float c3m = 0, c3p = 55, c3carX = -120;
float c4p = 55, c4sw = 0;
float bx[3] = {90, 330, 570}, by2[3] = {490, 505, 495};
float c5w = 0, c5b1 = 170, c5b2 = 560, c5p = 55, c5beam = 0;

float stX[120], stY[120], stS[120], stTw[120];
int stInit = 0;
