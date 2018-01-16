/*
author: Jens FROEBEL created: 2017-03-12 modified: 2017-10-29
version poligone_014.ino
*/
#include <Arduboy.h>
Arduboy arduboy;

/* TODOs!
 * use a model as partial circle to indicate enemy or hit, has property of ship and additional angle
 * done: fast arctan
 * add Live Gauge to objects: ship, enemy, asteroid, planet
 * add pew pew sounds: tone()
 * update to <Arduino2.h>
 * model + physic = object, make one common struct or include model as pointer in physic
 * add parallax starfield
 * takeAction(physic4) --> Enemy, turn to ship, trust, shoot
 * bullets: xvel, yvel, xpos, ypos, enable, force, maxdistance, traveled
 * rocket:  xvel, yvel, xpos, ypos, xacc, yacc, enable, force, maxdistance, traveled, mass, rot, scale, 
 * homing missile: transform position of enemy to missile orientation, then steer by resulting position
 * bomb:    xvel, yvel, xpos, ypos, xacc, yacc, enable, force, mass, scale --> release (btn) and ignite (btn)
 * asteroid: model + physic-property
 * collision test, use radius property
 * draw explosion
 * save high score to PROGMEM
 * show live gauge
 * remove scale property in struct model, maybe :-)
 * new algorithm for stars/dust --> maybe introduce 16 bit Fibonacci-LFSR or Galois-LFSR
 * done: blink thruster
 * done: show velocity
 * done: remove KEYDELAY
 * done: more comments 
 * done: CalcPos with pointers
 * done: CalcVel with Pointers
 * done: make grid as backgroud calculated on the fly for each frame, not from memory
 * done: grid with lines
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
const byte MAX_STARS = 10;                // max number of stars for the random starfield
const byte MAX_BULLETS = 3;               // Max Number of Bullets in one screen
const int BULLET_INTERVAL = 250;          // min interval between bullets
const int BULLET_TIME = 2000;             // Time to life for each bullet
const int UNIVERSE_X = MAX_X * 8;         // x size of this world
const int UNIVERSE_Y = MAX_Y * 4;         // y size of this world
//char text[16] = "Press Buttons!";;      // General string buffer

//unsigned long lasttimestamp = 0;
//unsigned long  newtimestamp = 0;
bool thrusterenabled = 0;                 // define global bool for thruster
bool shakeCam = 0;                        // define global bool for the shake effect
bool fireEnabled = 0;                     // ?
long unsigned int lastBullet = 0;         // count of shot bullets
long unsigned int now = millis();         // copy of current timestamp

const int edges = 5;                      // count of edges for the objects
struct model {float x, y;};               // struct for one vertex, to be use as array
struct camproperty {float x, y, zoom;} cam = {0, 0, 0.75}; // pan and zoom of camera

model ship1[] = {{0,2},{-1,-1},{-0.5,-0.5},{0.5,-0.5},{1,-1}}; // model of my space ship, asteroids-like
model planet[] = {{0,-1},{-0.951, -0.309},{-0.588, 0.809},{0.588, 0.809}, {0.951, -0.309}}; // model of the planet, pentagone
model thruster[] = {{0,-3},{1,-2},{0.5,-1},{-0.5,-1},{-1,-2}}; // model of thruster fire
model enemy1[] = {{0,1.5},{0.5,1},{0.5,-1.5},{-0.5,-1.5},{-0.5,1}};
model indicator[] = {{0,6},{-3,5},{-1,5.5},{1,5.5},{3,5}};

// template for calculating the objects
model object1[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // template for an object with 5 vertices
model object2[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // outer planet
model object3[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // inner planet
model object4[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // thruster
model objectEnemy[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // enemy
model objectindi1[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // enemy indicator
struct property { // property of an object
    float mass, radius, scale; // mass for gravity, radius for collision, scale for reshaping by scale
    float xpos, ypos, xvel, yvel, xacc, yacc;
    int rot;
    bool enable;
};

//                   m  r   s          xp yp   xv yv   xa ya      r  e
property physic1 = { 1, 2,  5,       -128, 0,   0, 0,   0, 0,   180, 1}; // ship propeties
property physic2 = {10, 1, 12,          0, 0,   0, 0,   0, 0,     0, 1}; // outer pentagone properties
property physic3 = {10, 1, 12*809/1000, 0, 0,   0, 0,   0, 0,   180, 1}; // inner pentagone properties
property physicEnemy = { 1, 1.5, 5,  +128, 0,   0,+0.67,   0, 0,     0, 1}; // enemy propeties


int dust[MAX_STARS][2]; // array of dust particles

struct projectile {  // struct of bullets, has no acceleration, no rotation
  float xpos, ypos, xvel, yvel;
  unsigned long int timestamp; // timestamp of ignition - for time to life
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

void drawGrid() {
  int x, y, gridGap, xWindowMin, xWindowMax, yWindowMin, yWindowMax, xGridMin, xGridMax, yGridMin, yGridMax;  

  gridGap = 64; 
  xWindowMin = round(-MAX_X/(cam.zoom*2) + cam.x);
  xWindowMax = round( MAX_X/(cam.zoom*2) + cam.x);
  yWindowMin = round(-MAX_Y/(cam.zoom*2) + cam.y);
  yWindowMax = round( MAX_Y/(cam.zoom*2) + cam.y);

  xGridMin = xWindowMin - (xWindowMin % gridGap);
  xGridMax = xWindowMax + (gridGap - xWindowMax % gridGap);
  yGridMin = yWindowMin - (yWindowMin % gridGap);
  yGridMax = yWindowMax + (gridGap - yWindowMax % gridGap);
  
  for (int ix = xGridMin; ix < xGridMax; ix = ix + gridGap) {
    for (int iy = yGridMin; iy < yGridMax; iy = iy + gridGap) {
      x = (int) ((ix - cam.x) * cam.zoom) + (MAX_X / 2);
      y = (int) ((iy - cam.y) * cam.zoom) + (MAX_Y / 2);
      arduboy.drawPixel(x, y, WHITE);       
    }  
  }

  gridGap = 192; 
  xWindowMin = round(-MAX_X/(cam.zoom*2) + cam.x);
  xWindowMax = round( MAX_X/(cam.zoom*2) + cam.x);
  yWindowMin = round(-MAX_Y/(cam.zoom*2) + cam.y);
  yWindowMax = round( MAX_Y/(cam.zoom*2) + cam.y);

  xGridMin = xWindowMin - (xWindowMin % gridGap);
  xGridMax = xWindowMax + (gridGap - xWindowMax % gridGap);
  yGridMin = yWindowMin - (yWindowMin % gridGap);
  yGridMax = yWindowMax + (gridGap - yWindowMax % gridGap);

  // Linewise Grid
  for (int ix = xGridMin; ix < xGridMax; ix = ix + gridGap) {
    x = (int) ((ix - cam.x) * cam.zoom) + (MAX_X / 2);
    //arduino.vline
    arduboy.drawFastVLine(x, 0, MAX_Y-1, WHITE);

  }
  for (int iy = yGridMin; iy < yGridMax; iy = iy + gridGap) {
    y = (int) ((iy - cam.y) * cam.zoom) + (MAX_Y / 2);
    //arduino.hline
    arduboy.drawFastHLine(0, y, MAX_X-1, WHITE);
    
  }
  
  //    arduboy.drawPixel(x, y, WHITE);       
  //arduboy.drawFastVLine(x, 5, MAX_Y-11, WHITE);
  //arduboy.drawFastHLine(5, y, MAX_X-11, WHITE);  
}

void transform(struct model *pntr, struct property *physic, struct model *outptr) {
  float sinrot, cosrot;
  sinrot = sin((physic->rot) * PI / 180.0); // calculate temporary sine
  cosrot = cos((physic->rot) * PI / 180.0); // calculate temporary cosine

  // Coordinates Transformation ROTATE
  (*outptr).x     = ( (pntr->x) * cosrot) + ((pntr->y) * sinrot); 
    outptr->y     = (-(pntr->x) * sinrot) + ((pntr->y) * cosrot);
  // Coordinates Transformation SCALING
    outptr->x     *=  (physic->scale);
    outptr->y     *=  (physic->scale);
  // Coordinates Transformation TRANSLATION
    outptr->x     +=  (physic->xpos);
    outptr->y     +=  (physic->ypos);
  // Coordinates Transformation by 1st Cam Pan and then 2nd Zoom
    outptr->x     -=  (cam.x);
    outptr->y     -=  (cam.y);
    outptr->x     *=  (cam.zoom);
    outptr->y     *=  (cam.zoom);
    outptr->x     +=  (MAX_X / 2);
    outptr->y     +=  (MAX_Y / 2);
    
}

int fastArcTan(float a, float b) {
  float m = 0.0;
  float absa = a; if (absa < 0) {absa *= -1;};
  float absb = b; if (absb < 0) {absb *= -1;};
  float atan = -1.0;
  // a ist Gegenkathede,  b ist Ankathede 
  if (absa < absb) {m = absa / absb;} else {m = absb / absa;};

  // Polynom 2. Grades R2 = 0,9999
  //atan = (60.619 * m) - (15.421 * m * m);
  // Polynom 3. Grades R2 = 1
  //atan = (58.825 * m) - (4.6852 * m * m * m) - (9.2673 * m * m);
  // Linear m*x+c is faster ;-)
  // two linear approximations are fitted and connect at m = 0.6
  // modified for 0° and 90°
  if (m < 0.6) {atan = 52.419 * m + 0.4999;} else {atan = 35.565 * m + 9.9253;};

  // 1. + 4. Quadrant
  if (a>=0) {
    if (b>=0) {                                            // 1. Quadrant
      if (absa > absb) { return (int) 0.5 + atan;}         //  0 .. 45     -->  45 .. 90
      else { return (int) 90.5 - atan ;};                  // 45 .. 90     -->   0 .. 45
    }
    else {                                                 // 4. Quadrant
      if (absa > absb) { return (int) 360.5 - atan;}       // 315 .. 360
      else { return (int) 270.5 + atan;};                  // 270 .. 315
    };
  }
  else { // 2. + 3. Quadrant
    if (b>=0) {                                            // 2. Quadrant
      if (absa > absb) { return (int) 180.5 - atan;}       // 135 .. 180
      else { return (int) 90.5 + atan;};                   //  90 .. 135
    }
    else {                                                 // 3. Quadrant
      if (absa > absb) { return (int) 180.5 + atan;}       // 180 .. 225
      else { return (int) 270.5 - atan;};                  // 225 .. 270
    };
  };
  return (int) -1;                                         // Fehler
}

int whereIsThat(struct property *me, struct property *that) {
  // int angle;
  // float tangens;
  // float sinrot, cosrot;
  // sinrot = sin((me->rot) * PI / 180.0); // calculate temporary sine
  // cosrot = cos((me->rot) * PI / 180.0); // calculate temporary cosine

  // relative Coordinates PAN
  float diffx = (that->xpos) - (me->xpos);
  float diffy = (that->ypos) - (me->ypos);
 
  // Coordinates Transformation ROTATE
  // diffx = ( (diffx) * cosrot) + ((diffy) * sinrot); 
  // diffy = (-(diffx) * sinrot) + ((diffy) * cosrot);

  //return fastArcTan(diffx, diffy) + (me->rot);  
  return fastArcTan(diffx, diffy);  
}

// output crossing point where x0
float intersection(float x1, float y1, float x2, float y2, float x0) {
  return ((x0-x1)*(y2-y1)/(x2-x1))+y1;
}

void drawVelGauge() {
    int absvel = sqrt(sq(physic1.xvel) + sq(physic1.yvel)) * 16;
    
    arduboy.drawRect(2, 2, 2+32, 2+1, WHITE);
    arduboy.drawFastHLine(2, 3, 2+absvel, WHITE);

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
  // ship: base model is   ship1 
  // vertices are in       object1 
  //                       physic1
  // ship
  // for (int i = 0; i < edges; i++) {transform( &ship1[i], &physic1, &object1[i]);}
     for (int i = 0; i < edges; i++) {
        drawConstrainLine(object1[i].x, 
                        object1[i].y, 
                        object1[(i+1)%edges].x, 
                        object1[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
     }
  }
  if (physic2.enable) {
  // outer planet: 
  // base model is   planet
  // vertices are    object2
  // physic is       physic2
  // Planet outer pentagone
  // for (int i = 0; i < edges; i++) {transform( &planet[i], &physic2, &object2[i]);}
  
    for (int i = 0; i < edges; i++) {
       drawConstrainLine(object2[i].x, 
                        object2[i].y, 
                        object2[(i+1)%edges].x, 
                        object2[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }
  if (physic3.enable) {
  // inner planet: 
  // base model is   planet; 
  // vertices are    object3; 
  // physic is       physic3
  // Planet inner pentagone
  // for (int i = 0; i < edges; i++) {transform( &planet[i], &physic3, &object3[i]);}

    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object3[i].x, 
                        object3[i].y, 
                        object3[(i+1)%edges].x, 
                        object3[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }  
  if (thrusterenabled) {
  // thruster: model is    thrustder 
  // vertices are          object4
  // physic is             thrusterenabled and physic1  
  // thruster: transf each vertex to current values based on model and physical property: output is object4
  // for (int i = 0; i < edges; i++) {transform( &thruster[i], &physic1, &object4[i]);}
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(object4[i].x, 
                        object4[i].y, 
                        object4[(i+1)%edges].x, 
                        object4[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
      }
  }
  if (physicEnemy.enable) {
  // Enemy: model is   objectEnemy; 
  // physics is        physicEnemy
  // enemy
  // for (int i = 0; i < edges; i++) {transform( &enemy1[i], &physicEnemy, &objectEnemy[i]);}
  
    for (int i = 0; i < edges; i++) {
      drawConstrainLine(objectEnemy[i].x, 
                        objectEnemy[i].y, 
                        objectEnemy[(i+1)%edges].x, 
                        objectEnemy[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
    }
  }

  if (physicEnemy.enable) {
  // model is          objectindi1; 
  // physics is        physicShip
  // EnemyIndicator    
      for (int i = 0; i < edges; i++) {
      drawConstrainLine(objectindi1[i].x, 
                        objectindi1[i].y, 
                        objectindi1[(i+1)%edges].x, 
                        objectindi1[(i+1)%edges].y, MAX_X-1, MAX_Y-1);
      }
  }

}
/*
uint16_t LFSR(uint16_t lfsr, int count) { // Galois LFSRs
bool lsb;
for (int i = 0; i < count; i++) {
        lsb = lfsr & 1;              // Get LSB (i.e., the output bit). 
        lfsr >>= 1;                  // Shift register 
        if (lsb) {lfsr ^= 0xB400u;}  // If the output bit is 1, apply toggle mask. 
    }
  return lfsr;
}
*/
void setup() {
  //Serial.begin(9600);  
  // initiate arduboy instance
  arduboy.beginNoLogo();
  arduboy.setFrameRate(30);
  //arduboy.setRGBled(0,32,32);

  // init Dustfield
 
  // uint16_t start_state = 0xACE1u;  /* Any nonzero start state will work. */
  
  for (int i = 0; i < MAX_STARS; i++) {dust[i][0] = random(-UNIVERSE_X * 1.3, UNIVERSE_X * 1.3) ; dust[i][1] = random(-UNIVERSE_Y * 1.3, UNIVERSE_Y * 1.3);}
  //for (int i = 0; i < MAX_STARS; i++) {dust[i][0] = i; dust[i][1] = 0;}
    
  for (int i = 0; i < MAX_BULLETS; i++) {bullet[i].isEnabled = 0;};

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

  // fire Bullet
  if(arduboy.pressed(B_BUTTON)) {
    if (now - lastBullet > BULLET_INTERVAL) {fireEnabled = 1;}   // delay between shots
    else {fireEnabled = 0;}
  }
  else {fireEnabled = 0;}

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

void CalcPosEnemy() {
  physicEnemy.xpos += physicEnemy.xvel;
  //physic1.xpos = constrain(physic1.xpos, -(MAX_X / 2) , (MAX_X / 2) );
  if (physicEnemy.xpos > UNIVERSE_X) {physicEnemy.xpos -= UNIVERSE_X * 2;};
  if (physicEnemy.xpos < -UNIVERSE_X) {physicEnemy.xpos += UNIVERSE_X * 2;};
  physicEnemy.ypos += physicEnemy.yvel;
  //physic1.ypos = constrain(physic1.ypos, -(MAX_Y / 2) , (MAX_Y / 2) );
  if (physicEnemy.ypos > UNIVERSE_Y) {physicEnemy.ypos -= UNIVERSE_Y * 2;};
  if (physicEnemy.ypos < -UNIVERSE_Y) {physicEnemy.ypos += UNIVERSE_Y * 2;};
  
}

void CalcBullets() {
  
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullet[i].isEnabled) {
      if (now - bullet[i].timestamp > BULLET_TIME) {
        bullet[i].isEnabled = 0;
      }
    }
  }

  if (fireEnabled) {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullet[i].isEnabled) {
        bullet[i].xvel = physic1.xvel + sin((physic1.rot) * PI / 180.0) * 2; // bullet velocity is ship.vel and ship.rot x bullet speed
        bullet[i].xpos = physic1.xpos;                                       // set bullets.pos to ship.pos
        bullet[i].yvel = physic1.yvel + cos((physic1.rot) * PI / 180.0) * 2;
        bullet[i].ypos = physic1.ypos; 
        bullet[i].timestamp = now;                                           // timestamp for lifetime of bullet or travel time
        bullet[i].isEnabled = 1;                                             // bullet is in use
        lastBullet = now;                                                    // this is for delay until next bullet
        break;
      }
    }
  }

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullet[i].isEnabled) {
      bullet[i].xpos += bullet[i].xvel;
      bullet[i].ypos += bullet[i].yvel;
      
    };
  };  
}

void CalcCam() {
  float deltax, deltay;
  deltax  = (physic2.xpos - physic1.xpos);
  deltax  = constrain(deltax, -96, 96);                          // follow the ship when distance is high
  deltay  = (physic2.ypos - physic1.ypos);
  deltay  = constrain(deltay, -96, 96);                          // follow the ship when distance is high
  cam.x   = physic1.xpos + (deltax / 2);
  cam.y   = physic1.ypos + (deltay / 2);                         // set cam between objects
  if (shakeCam) {cam.x += random(-3,3); cam.y += random(-2,2);}; // add shaking to scene

  cam.zoom = 64 / max(abs(deltax) / 2, abs(deltay));             // calculate zoom based ob objects
  cam.zoom /= 1.15;                                              // zoom out a bit
  cam.zoom = (float) ((int) (1.5 * cam.zoom)) / 1.5;             // with round for stepping zoom; coment out for continuous zoom
  cam.zoom = constrain(cam.zoom, 0.3, 1.2);                      // set upper and lower zoom limits
}

void loop() {
  now = millis();
  ButtonAction();
  ++physicEnemy.rot %= 360;
  CalcGravity();
  CalcVel();
  CalcPos();
  CalcPosEnemy();
  CalcBullets();
  CalcCam();  
  

  // blink thruster
  if (thrusterenabled) {
    thrusterenabled = ((now % 150) >= 100);
  }

  // ship
  for (int i = 0; i < edges; i++) {transform( &ship1[i], &physic1, &object1[i]);}
  // Planet outer pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic2, &object2[i]);}
  // Planet inner pentagone
  for (int i = 0; i < edges; i++) {transform( &planet[i], &physic3, &object3[i]);}
  // thruster: transf each vertex to current values based on model and physical property: output is object4
  for (int i = 0; i < edges; i++) {transform( &thruster[i], &physic1, &object4[i]);}
  // enemy
  for (int i = 0; i < edges; i++) {transform( &enemy1[i], &physicEnemy, &objectEnemy[i]);}
  // indicator to Enemy
  int rottemp = physic1.rot;
  physic1.rot = (90-whereIsThat(&physic1, &physicEnemy))%360;
  for (int i = 0; i < edges; i++) {transform( &indicator[i], &physic1, &objectindi1[i]);}
  physic1.rot = rottemp;

  // draw bullets, dust
  for (int i = 0; i < MAX_BULLETS; i++) {if (bullet[i].isEnabled) {drawStar(bullet[i].xpos, bullet[i].ypos);}}
  for (int i = 0; i < MAX_STARS; i++) {drawStar( dust[i][0], dust[i][1]);}

  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return
  arduboy.clear();
  //arduboy.fillScreen(BLACK);
  draw();
  drawGrid();
  drawVelGauge();
  //arduboy.drawPixel(64, 32, WHITE);
  //arduboy.drawRoundRect(0, 0, MAX_X, MAX_Y, 4, WHITE);
  //arduboy.drawFastVLine(x, 5, MAX_Y-11, WHITE);
  //arduboy.drawFastHLine(5, y, MAX_X-11, WHITE);
  //arduboy.drawLine(0, 0, x, y, WHITE);
  //arduboy.drawCircle(64, 32, 8, WHITE);
  arduboy.display();

}
