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

float intersection(float x1, float y1, float x2, float y2, float x0) {
// output crossing point where x0
  return ((x0-x1)*(y2-y1)/(x2-x1))+y1;
}

