

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

void drawVelGauge() {
    int absvel = sqrt(sq(physic1.xvel) + sq(physic1.yvel)) * 16;
    
    arduboy.drawRect(2, 2, 2+32, 2+1, WHITE);
    arduboy.drawFastHLine(2, 3, 2+absvel, WHITE);

}

void drawConstrainLine(float x1, float y1, float x2, float y2, float w, float h) {
// Constrain a Line to the display borders
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

