/*
author: Jens FROEBEL created: 2017-03-12 modified: 2017-04-25
version poligone_009.ino
*/
#include <Arduboy.h>
Arduboy arduboy;

/* TODOs!
 * CalcPos with pointers
 * CalcVel with Pointers
 * add Live Gauge to objects: ship, enemy, asteroid, planet
 * add pew pew sounds: tone()
 * update to <Arduino2.h>
 * model + physic = object, make one common struct
 * add parallax starfield
 * takeAction(physic4) --> Enemy, turn to ship, trust, shoot
 * bullets: xvel, yvel, xpos, ypos, enable, force, maxdistance, traveled
 * rocket:  xvel, yvel, xpos, ypos, xacc, yacc, enable, force, maxdistance, traveled, mass, rot, scale, 
 * homing missile: transform position of enemy to missile orientation, then steer by resulting position
 * bomb:    xvel, yvel, xpos, ypos, xacc, yacc, enable, force, mass, scale --> release (btn) and ignite (btn)
 * asteroid: model + physic-property
 * collision test, use radius property
 * done: blink thruster
 * draw explosion
 * save high score to PROGMEM
 * show velocity
 * show live gauge
 * new algorithm for stars/dust --> maybe introduce 16 bit Fibonacci-LFSR
 * done: takeAction(physic1) --> gravity
 * done: define const Universe borders
 * done: cam follow ship in high distance
 * done: autozoom cam to objects: ship, enemy, (planet) --> CalcCam()
 * done: takeAction(cam) --> set position based on objects
 * done: add non parallax dustfield
 * done: draw dustfield directly without saving coordibates
 * done: init camphy {x, y, zoom};
 * done: takeAction(physic1) --> force, velocity, position --> physic1 for thruster
 * done: translate: x, y, scale --> transform();
 * done: draw thruster on ship when button pressed, copy physic1 for thruster
 * done: (arduboy.buttonpressed(A_BUTTON))?{xacc = sin(rot)*thrust;yacc = cos(rot)*thrust}:{xacc=0;yacc=0)
 * done: xvel = constrain(0, maxspeed, xvel + xacc/timestep);
 * done: effect: shake cam
 */

const byte MAX_Y = 64;
const byte MAX_X = 128;
const byte MAX_STARS = 30;
const byte MAX_BULLETS = 5;
const int BULLET_TIME = 2000;
const int UNIVERSE_X = MAX_X * 6;
const int UNIVERSE_Y = MAX_Y * 6;
//char text[16] = "Press Buttons!";;      //General string buffer

//unsigned long lasttimestamp = 0;
//unsigned long  newtimestamp = 0;
bool thrusterenabled = 0;
bool shakeCam = 0;

const int edges = 5;
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

property physic1 = { 1, 2,  5,          -128, 0, 0, 0, 0, 0, 180, 1}; // ship
property physic2 = {10, 1, 12,          0, 0, 0, 0, 0, 0,   0, 1}; // outer pentagone
property physic3 = {10, 1, 12*809/1000, 0, 0, 0, 0, 0, 0, 180, 1}; // inner pentagone

int dust[MAX_STARS][2];
//dot bullet[3];

struct bul {
  float xpos, ypos;
  unsigned long int timestamp;
  int rot;
  bool isEnabled;
} bullet[MAX_BULLETS];

void drawStar(int xpos, int ypos) {
  float xdisp, ydisp;
  
  // Coordinates Transformation by 1st Cam Pan and then 2nd Zoom
    xdisp     =   (xpos) - (cam.x);
    ydisp     =   (ypos) - (cam.y);
    xdisp     =   (xdisp) * (cam.zoom) + (MAX_X / 2);
    ydisp     =   (ydisp) * (cam.zoom) + (MAX_Y / 2);
  
      if (xdisp < 128 && xdisp >= 0 && ydisp < 64 && ydisp >= 0) {
          arduboy.drawPixel((int) xdisp, (int) ydisp, WHITE);
          //arduboy.drawCircle((int) xdisp, (int) ydisp, 2, WHITE);
      }
}

void transform(struct model *pntr, struct property *physic, struct model *outptr) {
  float sinrot, cosrot;
  sinrot = sin((physic->rot) * PI / 180.0); // calculate temporary sine
  cosrot = cos((physic->rot) * PI / 180.0); // calculate temporary cosine

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
    outptr->x     +=  (MAX_X / 2);
    outptr->y     +=  (MAX_Y / 2);
    
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
        drawConstrainLine(object1[i].x, 
                        object1[i].y, 
                        object1[(i+1)%edges].x, 
                        object1[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
     }
  }

  if (physic2.enable) {
    for (int i = 0; i < edges; i++) {
       drawConstrainLine(object2[i].x, 
                        object2[i].y, 
                        object2[(i+1)%edges].x, 
                        object2[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }

  if (physic3.enable) {
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object3[i].x, 
                        object3[i].y, 
                        object3[(i+1)%edges].x, 
                        object3[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }  

  if (thrusterenabled) {
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object4[i].x, 
                        object4[i].y, 
                        object4[(i+1)%edges].x, 
                        object4[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }

}

void setup() {
  //Serial.begin(9600);  
  // initiate arduboy instance
  arduboy.beginNoLogo();
  arduboy.setFrameRate(30);
  //arduboy.setRGBled(0,32,32);

  // init Dustfield
  
  for (int i = 0; i < MAX_STARS; i++) {dust[i][0] = random(-UNIVERSE_X * 1.3, UNIVERSE_X * 1.3) ; dust[i][1] = random(-UNIVERSE_Y * 1.3, UNIVERSE_Y * 1.3);}
  //for (int i = 0; i < MAX_STARS; i++) {dust[i][0] = i; dust[i][1] = 0;}
  for (int i = 0; i < MAX_BULLETS; i++) {bullet[i].isEnabled = 0;};

u_int16t LFSR(u_int16t lfsr, int count) {
bool lsb;
for (int i = 0; i < count; i++) {
        unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
        lfsr >>= 1;                /* Shift register */
        if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
            lfsr ^= 0xB400u;
        }
    }
  return lfsr;
}

void ButtonAction() {
  
  if(arduboy.pressed(RIGHT_BUTTON))  {--physic1.rot %= 360;}
  if(arduboy.pressed(LEFT_BUTTON))   {++physic1.rot %= 360;}
  //if(arduboy.pressed(UP_BUTTON) && (scale < 80))      {scale *= 1.1; delay(KEYDELAY);}
  //if(arduboy.pressed(DOWN_BUTTON) && (scale > 1))  {scale /= 1.1; delay(KEYDELAY);}
  
  //if(arduboy.pressed(A_BUTTON)  &&  arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0xFF,0xFF,0x00);} // A + B --> yellow
  //if(arduboy.pressed(A_BUTTON)  && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x22);} // A     --> blue  
  //if(!arduboy.pressed(A_BUTTON) &&  arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0xFF,0x00);} // B     --> green
  //if(!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON)) {arduboy.setRGBled(0x00,0x00,0x00);} // none  --> reset

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

  // ShakeCam
  if(arduboy.pressed(DOWN_BUTTON)) {shakeCam = 1;} else {shakeCam = 0;};

  // Trigger Bullet
  //if(arduboy.pressed(B_BUTTON)) {bullet_triggered = 1; 

  // Rotate the Planet
  ++physic2.rot %= 360;
  --physic3.rot %= 360;
}

void CalcGravity() {
  float distance, distance2, distx, disty, gravity;
  distx = (physic1.xpos - physic2.xpos);
  //distx = constrain(distx, 10, 120);           // never zero, prevent
  disty = (physic1.ypos - physic2.ypos);
  //disty = constrain(disty, 10, 120);           // never zero, prevent
  distance2 = sq(constrain(abs(distx), 10, 256)) + sq(constrain(abs(disty), 10, 256));           // square of diagonal distance
  distance  = sqrt(distance2);
  gravity   = physic1.mass * physic2.mass / distance2;

  physic1.xacc -= gravity * distx / distance;
  physic1.yacc -= gravity * disty / distance;
  //arduboy.drawLine(64, 32, 64 + gravity * distx / distance, 32 + gravity * disty / distance, WHITE);
}

void CalcVel() {
  physic1.xvel += physic1.xacc;
  physic1.xvel *= 0.997;
  physic1.xvel = constrain(physic1.xvel, -1, 1);

  physic1.yvel += physic1.yacc;
  physic1.yvel *= 0.997;
  physic1.yvel = constrain(physic1.yvel, -1, 1);
  
}

void CalcPos() {
  physic1.xpos += physic1.xvel;
  //physic1.xpos = constrain(physic1.xpos, -(MAX_X / 2) , (MAX_X / 2) );
  if (physic1.xpos > UNIVERSE_X) {physic1.xpos -= UNIVERSE_X * 2;};
  if (physic1.xpos < -UNIVERSE_X) {physic1.xpos += UNIVERSE_X * 2;};
  physic1.ypos += physic1.yvel;
  //physic1.ypos = constrain(physic1.ypos, -(MAX_Y / 2) , (MAX_Y / 2) );
  if (physic1.ypos > UNIVERSE_Y) {physic1.ypos -= UNIVERSE_Y * 2;};
  if (physic1.ypos < -UNIVERSE_Y) {physic1.ypos += UNIVERSE_Y * 2;};
  
}

void CalcCam() {
  float deltax, deltay;
  deltax  = (physic2.xpos - physic1.xpos);
  deltax  = constrain(deltax, -96, 96);               // follow the ship when distance is high
  deltay  = (physic2.ypos - physic1.ypos);
  deltay  = constrain(deltay, -96, 96);               // follow the ship when distance is high
  cam.x   = physic1.xpos + (deltax / 2);
  cam.y   = physic1.ypos + (deltay / 2);             // set cam between objects
  if (shakeCam) {cam.x += random(-3,3); cam.y += random(-2,2);}; // add shaking to scene

  cam.zoom = 64 / max(abs(deltax) / 2, abs(deltay)); // calculate zoom based ob objects
  cam.zoom /= 1.15;                                   // zoom out a bit
  cam.zoom = (float) ((int) (1.5 * cam.zoom)) / 1.5; // with round for stepping zoom; coment out for continuous zoom
  cam.zoom = constrain(cam.zoom, 0.3, 1.2);        // set upper and lower zoom limits
}

void loop() {

  ButtonAction();
  CalcGravity();
  CalcVel();
  CalcPos();
  CalcCam();  

  // blink thruster
  if (thrusterenabled) {
    thrusterenabled = ((millis() % 150) >= 100);
  }

  // ship
  for (int i = 0; i < edges; i++) {transform( &ship1[i], &physic1, &object1[i]);}
  // Planet outer pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic2, &object2[i]);}
  // Planet inner pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic3, &object3[i]);}
  // thruster: transf each vertex to current values based on model and physical property: output is object4
  for (int i = 0; i < edges; i++) {transform( &thruster[i], &physic1, &object4[i]);}

  // transform bullets, dust
  //for (int i = 0; i < max_bullets; i++) {transform( &bullet[i], &bullet_physic[i], &object5[i]);}
  for (int i = 0; i < MAX_STARS; i++) {drawStar( dust[i][0], dust[i][1]);}

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
  //arduboy.drawCircle(64, 32, 8, WHITE);
  arduboy.display();

}
