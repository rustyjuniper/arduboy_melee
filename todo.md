# TODOs

 * !!! takeAction(physic4) --> Enemy, turn to ship, trust, shoot
 * add pew pew sounds: tone()
 * update to <Arduino2.h>
 * model + physic = object, make one common struct or include model as pointer in physic
 * add parallax starfield
 
 * asteroid: model + physic-property
 * collision test, use radius property
 * draw explosion
 * save high score to PROGMEM
 * show live gauge
 * new algorithm for stars/dust --> maybe introduce 16 bit Fibonacci-LFSR or Galois-LFSR

## Weaponary

 * additional Weapon: Translocation-Portal
 * additional Weapon: Wingbots
 * additional Weapon: floating time bomb
 * additional Weapon: homing missile
 * bullets: xvel, yvel, xpos, ypos, enable, force, maxdistance, traveled
 * rocket:  xvel, yvel, xpos, ypos, xacc, yacc, enable, force, maxdistance, traveled, mass, rot, scale, 
 * homing missile: transform position of enemy to missile orientation, then steer by resulting position
 * bomb:    xvel, yvel, xpos, ypos, xacc, yacc, enable, force, mass, scale --> release (btn) and ignite (btn)
 
# DONE
 
 * done; use a model as partial circle to indicate enemy or hit, has property of ship and additional angle
 * done: fast arctan
 * done: add Live Gauge to objects: ship, enemy, asteroid, planet
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

# Rejected
 * rejected: remove scale property in struct model, maybe :-)
