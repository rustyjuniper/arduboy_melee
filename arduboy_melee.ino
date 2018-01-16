/*
author: Jens FROEBEL created: 2017-03-12 modified: 2017-04-19
version poligone_006.ino
*/
#include <Arduboy.h>
Arduboy arduboy;

/* TODOs!
 * autozoom cam to objects: ship, enemy, (planet) --> CalcCam()
 * takeAction(cam) --> set position based on objects
 * CalcPos with pointers
 * CalcVel with Pointers
 * add Live Gauge to objects: ship, enemy, asteroid, planet
 * add pew pew sounds: tone()
 * update to <Arduino2.h>
 * model + physic = object, make one common struct
 * add parallax starfield, or non parallax dustfield
 * takeAction(physic1) --> gravity
 * takeAction(physic4) --> Enemy, turn to ship, trust, shoot
 * bullets: xvel, yvel, xpos, ypos, enable, force, maxdistance, traveled
 * rocket:  xvel, yvel, xpos, ypos, xacc, yacc, enable, force, maxdistance, traveled, mass, rot, scale, 
 * bomb:    xvel, yvel, xpos, ypos, xacc, yacc, enable, force, mass, scale
 * asteroid: model + physic-property
 * collision test, use radius property
 * effect: shake cam
 * draw explosion
 * save high score to PROGMEM
 * done: init camphy {x, y, zoom};
 * done: takeAction(physic1) --> force, velocity, position --> physic1 for thruster
 * done: translate: x, y, scale --> transform();
 * done: draw thruster on ship when button pressed, copy physic1 for thruster
 * done: (arduboy.buttonpressed(A_BUTTON))?{xacc = sin(rot)*thrust;yacc = cos(rot)*thrust}:{xacc=0;yacc=0)
 * done: xvel = constrain(0, maxspeed, xvel + xacc/timestep);
 */

byte MAX_Y = 64;
byte MAX_X = 128;
int x = MAX_X / 2;
int y = MAX_Y / 2;
byte KEYDELAY = 1;
//char text[16] = "Press Buttons!";;      //General string buffer

unsigned long lasttimestamp = 0;
unsigned long  newtimestamp = 0;
float radian;
bool thrusterenabled = 0;
int edges = 5;

struct model {float x, y;};
struct camproperty {float x, y, zoom;} cam = {0, 0, 0.75};

model ship1[] = {{0,2},{-1,-1},{-0.5,-0.5},{0.5,-0.5},{1,-1}};
model planet[] = {{0,-1},{-0.951, -0.309},{-0.588, 0.809},{0.588, 0.809}, {0.951, -0.309}};
model thruster[] = {{0,-3},{1,-2},{0.5,-1},{-0.5,-1},{-1,-2}};
//model enemy1[] = {{}};

model object1[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
model object2[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
model object3[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
model object4[] = {{0,0},{0,0},{0,0},{0,0},{0,0}};

struct property {
    float mass, radius, scale; 
    float xpos, ypos, xvel, yvel, xacc, yacc;
    int rot;
    bool enable;
};

property physic1 = { 1, 2,  5,          -64, 0, 0, 0, 0, 0, 180, 1}; // ship
property physic2 = {10, 1, 10,          0, 16, 0, 0, 0, 0,   0, 1}; // outer pentagone
property physic3 = {10, 1, 10*809/1000, 0, 16, 0, 0, 0, 0, 180, 1}; // inner pentagone


void transform(struct model *pntr, struct property *physic, struct model *outptr) {
  float sinrot, cosrot, tempox, tempoy;
  sinrot = sin((physic->rot) * PI / 180.0);
  cosrot = cos((physic->rot) * PI / 180.0);
  //printf("sin: %f cos: %f\n", sinrot, cosrot);

  // Coordinates Transformation ROTATE
  (*outptr).x     = ( (pntr->x) * cosrot) + ((pntr->y) * sinrot); 
    outptr->y     = (-(pntr->x) * sinrot) + ((pntr->y) * cosrot);
  // Coordinates Transformation SCALING
    outptr->x     =   (outptr->x) * (physic->scale);
    outptr->y     =   (outptr->y) * (physic->scale);
  // Coordinates Transformation TRANSLATION
    outptr->x     =   (outptr->x) + (physic->xpos);
    outptr->y     =   (outptr->y) + (physic->ypos);
  // Coordinates Transformation by 1st Cam Pan and then 2nd Zoom
    outptr->x     =   (outptr->x) - (cam.x);
    outptr->y     =   (outptr->y) - (cam.y);
    outptr->x     =   (outptr->x) * (cam.zoom);
    outptr->y     =   (outptr->y) * (cam.zoom);
    
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
        drawConstrainLine(object1[i].x + (MAX_X / 2), 
                        object1[i].y + (MAX_Y / 2), 
                        object1[(i+1)%edges].x + (MAX_X / 2), 
                        object1[(i+1)%edges].y + (MAX_Y / 2), MAX_X-1, MAX_Y-1);
     }
  }

  if (physic2.enable) {
    for (int i = 0; i < edges; i++) {
       drawConstrainLine(object2[i].x + (MAX_X / 2), 
                        object2[i].y + (MAX_Y / 2), 
                        object2[(i+1)%edges].x + (MAX_X / 2), 
                        object2[(i+1)%edges].y + (MAX_Y / 2), MAX_X-1, MAX_Y-1);
    }
  }

  if (physic3.enable) {
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object3[i].x + (MAX_X / 2), 
                        object3[i].y + (MAX_Y / 2), 
                        object3[(i+1)%edges].x + (MAX_X / 2), 
                        object3[(i+1)%edges].y + (MAX_Y / 2), MAX_X-1, MAX_Y-1);
    }
  }  

  if (thrusterenabled) {
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object4[i].x + (MAX_X / 2), 
                        object4[i].y + (MAX_Y / 2), 
                        object4[(i+1)%edges].x + (MAX_X / 2), 
                        object4[(i+1)%edges].y + (MAX_Y / 2), MAX_X-1, MAX_Y-1);
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
  //if(arduboy.pressed(A_BUTTON)  && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x22);} // A     --> blue  
  if(!arduboy.pressed(A_BUTTON) &&  arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0xFF,0x00);} // B     --> green
  if(!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x00);} // none  --> reset

  // Thruster!
  if(arduboy.pressed(A_BUTTON)) {
    physic1.xacc = sin((physic1.rot) * PI / 180.0) / 100;
    physic1.yacc = cos((physic1.rot) * PI / 180.0) / 100;
    thrusterenabled = 1;
    } // A     --> acc = abs(1)
   else {
    physic1.xacc = 0;
    physic1.yacc = 0;
    thrusterenabled = 0;
    };

  // Rotate the Planet
  ++physic2.rot %= 360;
  --physic3.rot %= 360;
}

void CalcVel() {
  physic1.xvel += physic1.xacc;
  physic1.xvel *= 0.995;
  physic1.xvel = constrain(physic1.xvel, -1, 1);

  physic1.yvel += physic1.yacc;
  physic1.yvel *= 0.995;
  physic1.yvel = constrain(physic1.yvel, -1, 1);
  
}

void CalcPos() {
  physic1.xpos += physic1.xvel;
  //physic1.xpos = constrain(physic1.xpos, -(MAX_X / 2) , (MAX_X / 2) );
  if (physic1.xpos > MAX_X * 2) {physic1.xpos -= MAX_X * 4;};
  if (physic1.xpos < -MAX_X * 2) {physic1.xpos += MAX_X * 4;};
  physic1.ypos += physic1.yvel;
  //physic1.ypos = constrain(physic1.ypos, -(MAX_Y / 2) , (MAX_Y / 2) );
  if (physic1.ypos > MAX_Y * 2) {physic1.ypos -= MAX_Y * 4;};
  if (physic1.ypos < -MAX_Y * 2) {physic1.ypos += MAX_Y * 4;};
  
}

void CalcCam() {
  float deltax, deltay;
  deltax  = (physic2.xpos - physic1.xpos);
  deltay  = (physic2.ypos - physic1.ypos);
  cam.x   = physic1.xpos + (deltax / 2);
  cam.y   = physic1.ypos + (deltay / 2);             // set cam between objects
  cam.zoom = 64 / max(abs(deltax) / 2, abs(deltay)); // calculate zoom based ob objects
  cam.zoom /= 1.1;                                   // zoom out
  cam.zoom = (float) ((int) (1.5 * cam.zoom)) / 1.5; // with round for stepping zoom; coment out for continuous zoom
  cam.zoom = constrain(cam.zoom, 0.25, 1.25);        // set upper and lower zoom limits
}

void loop() {

  ButtonAction();
  CalcVel();
  CalcPos();
  CalcCam();  

  // ship
  for (int i = 0; i < edges; i++) {transform( &ship1[i], &physic1, &object1[i]);}
  // Planet outer pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic2, &object2[i]);}
  // Planet inner pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic3, &object3[i]);}
  // thruster: transf each vertex to current values based on model and physical property: output is object4
  for (int i = 0; i < edges; i++) {transform( &thruster[i], &physic1, &object4[i]);}
  // transform bullets
  //for (int i = 0; i < max_bullets; i++) {transform( &bullet[i], &bullet_physic[i], &object5[i]);}
  

  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return
  arduboy.clear();
  //arduboy.fillScreen(BLACK);
  draw();

  //arduboy.drawPixel(64, 32, WHITE);
  //arduboy.drawRoundRect(0, 0, MAX_X, MAX_Y, 4, WHITE);
  //arduboy.drawFastVLine(x, 5, MAX_Y-11, WHITE);
  //arduboy.drawFastHLine(5, y, MAX_X-11, WHITE);
  //arduboy.drawLine(0, 0, x, y, WHITE);
  arduboy.display();

}
