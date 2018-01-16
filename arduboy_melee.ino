/*
author: Jens FROEBEL created: 2017-03-12 modified: 2017-04-19
version poligone_004.ino
*/
#include "Arduboy.h"

Arduboy arduboy;

/* TODO!
 * init camphy {x, y, zoom};
 * tranlate: x, y, scale --> transform();
 * 
 * model + physic = object, make one common struct
 * 
 * (arduboy.bottonpressed(A_BUTTON))?{xacc = sin(rot)*thrust;yacc = cos(rot)*thrust}:{xacc=0;yacc=0)
 * xvel = constrain(0, maxspeed, xvel + xacc/timestep);
 * 
 * takeAction(physc1) --> position, force
 * takeAction(physc2)
 * takeAction(physc3)
 * takeAction(cam) --> reset position
 * 
 * collision test
 */

byte MAX_Y = 64;
byte MAX_X = 128;
int x = MAX_X / 2;
int y = MAX_Y / 2;
byte KEYDELAY = 1;
//char text[16] = "Press Buttons!";;      //General string buffer

unsigned long lasttimestamp = 0;
unsigned long  newtimestamp = 0;
//int degree = 0;
//int rot = 180;
//int p1rot = 0;
//int p2rot = 180;
//float scale = 5;
//float p1scale = 15;
//float p2scale = 15*809/1000;
float radian;
int edges = 5;

struct model {float x, y;};

model ship1[] = {{0,2},{-1,-1},{-0.5,-0.5},{0.5,-0.5},{1,-1}};
model planet[] = {{0,-1},{-0.951, -0.309},{-0.588, 0.809},{0.588, 0.809}, {0.951, -0.309}};
model object1[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
model object2[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
model object3[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};


struct property {
    float mass, radius, scale; 
    float xpos, ypos, xvel, yvel, xacc, yacc;
    int rot;
    bool enable;
};

property physic1 = { 1, 2,  5,          -9, 0, 0, 0, 0, 0, 180, 1}; // ship
property physic2 = {10, 1, 10,          0, 0, 0, 0, 0, 0,   0, 1}; // outer pentagone
property physic3 = {10, 1, 10*809/1000, 0, 0, 0, 0, 0, 0, 180, 1}; // inner pentagone


void transform(struct model *pntr, struct model *outptr, float x, float y, int angle) {
  float sinrot, cosrot, tempox, tempoy;
  sinrot = sin((angle) * PI / 180.0);
  cosrot = cos((angle) * PI / 180.0);
  //printf("sin: %f cos: %f\n", sinrot, cosrot);

  // Coordinates Transformation ROTATE
  (*outptr).x     = ( (pntr->x) * cosrot) + ((pntr->y) * sinrot); 
    outptr->y     = (-(pntr->x) * sinrot) + ((pntr->y) * cosrot);
  // Coordinates Transform Translation
    outptr->x     =   (outptr->x) + x;
    outptr->y     =   (outptr->y) + y;
}

// output crossing point where x0
float intersection(float x1, float y1, float x2, float y2, float x0) {
  return ((x0-x1)*(y2-y1)/(x2-x1))+y1;
}

// Constrain a Line to the display borders
void drawConstrainLine(float x1, float y1, float x2, float y2, float w, float h) {
  //float border = 0;
  bool drawMe = true;

  if (drawMe and (x1 < 0)) {
    if (x2 < 0) {drawMe = false;}
    else {y1 = intersection(x1, y1, x2, y2, 0); x1=0;}
  }
  else if (drawMe and (x2 < 0)) {
        if (x1 < 0) {drawMe = false;}
        else {y2 = intersection(x2, y2, x1, y1, 0); x2=0;}
       };

  if (drawMe and (y1 < 0)) {
    if (y2 < 0) {drawMe = false;}
    else {x1 = intersection(y1, x1, y2, x2, 0); y1=0;}
  }
  else if (drawMe and (y2 < 0)) {
        if (y1 < 0) {drawMe = false;}
        else {x2 = intersection(y2, x2, y1, x1, 0); y2=0;}
          };

  if (drawMe) arduboy.drawLine(x1, y1, x2, y2, WHITE);
}

void draw(void) {  
  if (physic1.enable) {
     for (int i = 0; i < edges; i++) {
        drawConstrainLine(physic1.scale*object1[i].x + 64, 
                        physic1.scale*object1[i].y + 32, 
                        physic1.scale*object1[(i+1)%edges].x + 64, 
                        physic1.scale*object1[(i+1)%edges].y + 32, MAX_X-1, MAX_Y-1);
     }
  }

  if (physic2.enable) {
    for (int i = 0; i < edges; i++) {
       drawConstrainLine(physic2.scale*object2[i].x + 64, 
                        physic2.scale*object2[i].y + 32, 
                        physic2.scale*object2[(i+1)%edges].x + 64, 
                        physic2.scale*object2[(i+1)%edges].y + 32, MAX_X-1, MAX_Y-1);
    }
  }

  if (physic3.enable) {
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(physic3.scale*object3[i].x + 64, 
                        physic3.scale*object3[i].y + 32, 
                        physic3.scale*object3[(i+1)%edges].x + 64, 
                        physic3.scale*object3[(i+1)%edges].y + 32, MAX_X-1, MAX_Y-1);
    }
  }  

}


void setup() {
  //Serial.begin(9600);  
  // initiate arduboy instance
  arduboy.beginNoLogo();
  arduboy.setFrameRate(30);
  //arduboy.setRGBled(0,32,32);
}

void ButtonAction() {
  
  if(arduboy.pressed(RIGHT_BUTTON))  {--physic1.rot %= 360; delay(KEYDELAY);}
  if(arduboy.pressed(LEFT_BUTTON))   {++physic1.rot %= 360; delay(KEYDELAY);}
  //if(arduboy.pressed(UP_BUTTON) && (scale < 80))      {scale *= 1.1; delay(KEYDELAY);}
  //if(arduboy.pressed(DOWN_BUTTON) && (scale > 1))  {scale /= 1.1; delay(KEYDELAY);}
  
  if(arduboy.pressed(A_BUTTON)  &&  arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0xFF,0xFF,0x00);} // A + B --> yellow
  if(arduboy.pressed(A_BUTTON)  && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x22);} // A     --> blue  
  if(!arduboy.pressed(A_BUTTON) &&  arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0xFF,0x00);} // B     --> green
  if(!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x00);} // none  --> reset

  // Thuster!
  if(arduboy.pressed(A_BUTTON)) {
    physic1.xacc = sin((physic1.rot) * PI / 180.0) / 1000;
    physic1.yacc = cos((physic1.rot) * PI / 180.0) / 1000;
    } // A     --> acc = abs(1)
   else {
    physic1.xacc = 0;
    physic1.yacc = 0;
    };
}

void CalcVel() {
  physic1.xvel += physic1.xacc;
  physic1.xvel *= 0.99;
  physic1.xvel = constrain(physic1.xvel, -1, 1);

  physic1.yvel += physic1.yacc;
  physic1.yvel *= 0.99;
  physic1.yvel = constrain(physic1.yvel, -1, 1);
  
}

void CalcPos() {
  physic1.xpos += physic1.xvel;
  physic1.xpos = constrain(physic1.xpos, -64 / physic1.scale, 64 / physic1.scale);
  physic1.ypos += physic1.yvel;
  physic1.ypos = constrain(physic1.ypos, -32 / physic1.scale, 32 / physic1.scale);
}

void loop() {

  ButtonAction();
  CalcVel();
  CalcPos();

  /*
  if(arduboy.pressed(A_BUTTON)) {
    Serial.println("Hello"); 
    for (int i = 0; i < edges; i++) {
     Serial.print(int(scale * object1[i].x + 64));
      Serial.print(" ");
      Serial.println(int(scale * object1[i].y + 32));
     }
    delay(500);
  }
  */
  ++physic2.rot %= 360;
  --physic3.rot %= 360;
  // ship
  for (int i = 0; i < edges; i++) {
    transform( &ship1[i], &object1[i], physic1.xpos, physic1.ypos, physic1.rot);
  }
  // Planet
  for (int i = 0; i < edges; i++) {
    transform( &planet[i], &object2[i], physic2.xpos, physic2.ypos, physic2.rot);
  }
  for (int i = 0; i < edges; i++) {
    transform( &planet[i], &object3[i], physic3.xpos, physic3.ypos, physic3.rot);
  }


  

  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return
  arduboy.clear();
  //arduboy.fillScreen(BLACK);

  draw();
  
  //arduboy.drawRoundRect(0, 0, MAX_X, MAX_Y, 4, WHITE);
  //arduboy.drawFastVLine(x, 5, MAX_Y-11, WHITE);
  //arduboy.drawFastHLine(5, y, MAX_X-11, WHITE);
  //arduboy.drawLine(0, 0, x, y, WHITE);
  arduboy.display();

}
