/*
author: Jens FROEBEL created: 2017-03-12 modified: 2018-01-19
version 0.16
*/
#include <Arduboy.h>
Arduboy arduboy;

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
bool fireEnabled = 0;                     // to control firemax and fire speed
long unsigned int lastBullet = 0;         // count of shot bullets
long unsigned int now = millis();         // copy of current timestamp

const int edges = 5;                      // count of edges for the objects
struct model {float x, y;};               // struct for one vertex, to be use as array
struct camproperty {float x, y, zoom;} cam = {0, 0, 0.75}; // pan and zoom of camera

model ship1[]     = {{0,2},{-1,-1},{-0.5,-0.5},{0.5,-0.5},{1,-1}}; // model of my space ship, asteroids-like
model ship2[]     = {{0,1},{-1.5,-1},{-0.5,-0.5},{0.5,-0.5},{1.5,-1}}; // model of short space ship
model ship3[]     = {{0,3.5},{-1,-1},{-0.5,-0.5},{0.5,-0.5},{1,-1}}; // model of long space ship
model planet[]    = {{0,-1},{-0.951, -0.309},{-0.588, 0.809},{0.588, 0.809}, {0.951, -0.309}}; // model of the planet, pentagone
model thruster[]  = {{0,-3},{1,-2},{0.5,-1},{-0.5,-1},{-1,-2}}; // model of thruster fire
model enemy1[]    = {{0,1.5},{0.5,1},{0.5,-1.5},{-0.5,-1.5},{-0.5,1}};
model indicator[] = {{0,6},{-3,5},{-1,5.5},{1,5.5},{3,5}};

// template for calculating the objects
model object1[]     = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // template for an object with 5 vertices
model object2[]     = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // outer planet
model object3[]     = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // inner planet
model object4[]     = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // thruster
model objectEnemy[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // enemy
model objectindi1[] = {{0,0},{0,0},{0,0},{0,0},{0,0}}; // enemy indicator

struct property { // property of an object
    float mass, radius, scale; // mass for gravity, radius for collision, scale for reshaping by scale
    float xpos, ypos, xvel, yvel, xacc, yacc;
    int rot;
    bool enable;
};

struct projectile {  // struct of bullets, has no acceleration, no rotation
  float xpos, ypos, xvel, yvel;
  unsigned long int timestamp; // timestamp of ignition - for time to life
  bool isEnabled;
} bullet[MAX_BULLETS];

//                       m    r   s             xp yp   xv     yv   xa ya      r  e
property physic1     = { 1, 2.0,  5,          -128, 0,   0,  0.00,   0, 0,   180, 1}; // ship propeties
property physic2     = {10, 1.0, 12,             0, 0,   0,  0.00,   0, 0,     0, 1}; // outer pentagone properties
property physic3     = {10, 1.0, 12*809/1000,    0, 0,   0,  0.00,   0, 0,   180, 1}; // inner pentagone properties
property physicEnemy = { 1, 1.5,  5,          +128, 0,   0, +0.67,   0, 0,     0, 1}; // enemy propeties

int dust[MAX_STARS][2]; // array of dust particles

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
