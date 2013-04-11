#ifndef GIGAPAN
#define GIGAPAN

/********************************************** 
 * UCSD NGS Stabilized Aerial Camera Platform *
 * Gigapan Coordinate Generator               *
 *                                            *
 * gigapan.h:                                 *
 * auxiliary functions, structures, etc.      *
 *                                            *
 * Compatibility: ANSI C and up               *
 *                                            *
 * See README or gigancoords.pdf for an       *
 * explanation of the program/usage           *
 *                                            *
 * @author Sergei I. Radutnuy                 *
 *         sradutnu@ucsd.edu                  *
 *                                            *
 * Last Modified: March 1 2013                *
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/************Auxiliary functions, constants, structures**************/

/* Conversion factors to/from 10th's of degrees - radians. */
#define pi 3.14159265359
double deg2rad = pi/180.0;
double rad2deg = 180.0/pi;


/* struct to hold coordinates for a point - p is pitch, y is yaw */
typedef struct point {
  double y;
  double p;
} point;


/* void printPt ( point to print, output file pointer) 
 * 
 * Prints out coordinates of the given point, to tenth of degree
 * accuracy, in the following format:
 * pitch,yaw<newline>
 */
void printPt( point* x, FILE* outf ) {
  fprintf( outf, "%.1f\t%.1f\n", x->y, x->p );
}


/* double yawDelta ( current point, horizontal field of view, 
 *                   vertical field of view, horizontal overlap ) 
 *
 * Calculates appropriate yaw increment for a given point in param domain, 
 * fields of view, and overlap
 */
double yawDelta( point* x, int hfov, int vfov, double ol ) {
  
  /* top and bottom pitches of the rectangle */
  int top = x->p + (int)( 0.5*vfov );
  int bottom = x->p - (int)( 0.5*vfov );
  
  /* distance of pitch from equator (just |pitch|) */ 
  
  int eqdistop = fabs(top);
  int eqdisbot = fabs(bottom);
  
  /* factor in overlap */
  hfov = hfov*(1.0 - 0.02*ol);
  
  /* yaw delta calculation: every time you project a rectangle onto a sphere, 
   * the top/bottom represents a part of a line of latitude. Lines of latitude 
   * form circles on the sphere, and these circles get smaller further from the 
   * equator, by a factor of cos(pitch). We adjust for this by dividing the 
   * hfov by that factor.
   */

  /* top closer to equator, use top */
  if(eqdistop < eqdisbot) {
    /* round to nearest 10th of degree & return */
    int roundtop = (int) 10.0*hfov/cos( deg2rad*top ) + 0.5;
    return 0.1*roundtop; 
  }

  /* bottom closer to equator, or they're the same, use bottom */
  else {
    /* round to nearest 10th of degree & return */
    int roundbot = (int) 10.0*hfov/cos( deg2rad*top ) + 0.5;
    return 0.1*roundbot;
  }
}  


/* int isInYawRange( minimum point, maximum point, 
 *                   current point, yaw increment )
 * 
 * Returns true (non-zero int) or false (0) depending on whether 
 * or not the current point's yaw is in the interval given by 
 * (smallest allowable yaw - increment, largest allowable yaw + increment)
 */
int isInYawRange( point* m, point* M, point* c, double d ){
  
  /* if the min and max yaw are close enough
   * together, then the panorama is a vertical
   * strip. Return false to keep the yaw from
   * changing/looping. */
  if( fabs(m->y - M->y) < 0.1 )
    return 0;
  
  /* negest is lowest allowable yaw - d  and a 1/10 degree buffer */
  double negest = m->y < M->y ? m->y : M->y;
  /* posest is  highest allowable yaw + d and a 1/10 degree buffer*/
  double posest = m->y < M->y ? M->y : m->y;
  
  return ( negest < c-> y) && ( c->y < posest );      
}


/* int isInPitchRange ( minimum point, maximum point, 
 *                      current point, pitch increment )
 * 
 * Returns true (non-zero int) or false (0) depending on whether 
 * or not the current point's pitch is in the interval given by 
 * (smallest allowable pitch - increment, largest allowable pitch + increment)
 */
int isInPitchRange( point* m, point* M, point* c, double d ){
  
  /* if the min and max pitch are close enough
   * together, then the panorama is a horizontal
   * strip. Return false to keep the pitch from
   * changing/looping. */
  if( fabs(m->p - M->p) < 0.1 )
    return 0;
  
  /* negest is lowest allowable yaw - d and a 1/10 degree buffer */
  double negest = m->p < M->p ? m->p : M->p;
  /* posest is  highest allowable yaw + d and a 1/10 degree buffer*/
  double posest = m->p < M->p ? M->p : m->p;
  
  return ( negest < c->p ) && ( c->p < posest );      
}
#endif /* GIGAPAN */
