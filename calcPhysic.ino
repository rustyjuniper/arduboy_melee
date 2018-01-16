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

