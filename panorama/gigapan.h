#ifndef GIGAPAN
#define GIGAPAN

/********************************************** 
 * UCSD E4E Stabilized Aerial Camera Platform *
 * Panorama                                   *
 *                                            *
 * File: UCSD-E4E/sacp/panorama/gigapan.h     *
 *                                            *
 * Compatibility: ANSI C                      *
 *                                            *
 * Author: Sergei I. Radutnuy                 *
 *         sradutnu@ucsd.edu                  *
 *                                            *
 * Last Modified: April 14 2013               *
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/************Auxiliary functions, constants, structures**************/

/* Conversion factors between degrees and radians. */
#define pi 3.14159265359
double deg2rad = pi/180.0;
double rad2deg = 180.0/pi;


/* double fov( focal length, sensor dimension )
 *
 * Returns the field of view, in degrees, for the 
 * given focal length and sensor dimension.
 * 
 * FOV = 2*arctan( image sensor dimension / 2*focal length )
 *
 * Units for the parameters don't matter, as long as both 
 * values measured with the same unit.
 */
double fov( double f, double d ) {
  return 2*rad2deg*atan( 0.5*(d/flength) );
}

/* struct to hold coordinates for a point. y = yaw, p = pitch*/
typedef struct point {
  double y;  
  double p;  
} point;


/* point shiftPt( current point, shift point )
 *
 * Returns a point which is current + shift (component-wise),
 * making sure the yaw and pitch of the result is within the
 * yaw/pitch domain [-180,180]X[-90,90]
 *
 * DEPENDS ON SPECIFIC SPHERE/IMU PARAMETRIZATION
 */
point shiftPt( point* c, point* s ) {
  
  /* keep track of quotient of mod */
  int modcount = 0;

  /* sum & mod yaws */
  double ty = c->y + s->y; 
  /* yaw < -180 -> add 180 until ok */ 
  if( ty < -180.0 ) {
    while( ty < -180.0 ) { 
      ty += 180.0; 
      ++n;
    }
    /* n even - you've added int*360, do nothing */
    if ( n % 2 != 0 ) 
      /* n odd - you're in the other hemisphere now*/
      ty += 180.0;    
  }
  /* same explanation as 1st if */
  else if( 180.0 < ty ) {
    while( 180.0 < ty ) { 
      ty -= 180.0; 
      ++n;
    }
    if( n % 2 != 0 )  
      ty += -180.0;
  }

  /* reset for pitches */
  modcount = 0;

  /* sum & "mod" pitches */
  double tp = c->p + s->p; 
  if( tp < -90.0 ) {
    while( tp < -90.0 ) { tp += 90.0; }
    tp += 90.0;
  }
  else if( 90.0 < tp ) {
    while( 90.0 < tp ) { tp -= 90.0; }
    tp += -90.0;
  }

  point tpt = { ty, tp };
  return tpt;
}

/* void printPt ( point to print, output file pointer ) 
 * 
 * Prints out coordinates of the given point, to tenth of degree
 * accuracy, in the following format:
 * yaw pitch<newline>
 */
void printPt( point x, FILE* outf ) {
  fprintf( outf, "%.1f\t%.1f\n", x.y, x.p );
}

/* double yawDelta ( current point, horizontal field of view, 
 *                   vertical field of view, horizontal overlap ) 
 *
 * Calculates appropriate yaw increment for a given point in param domain, 
 * fields of view, and overlap
 */
double yawDelta( int pitchx, int hfov, int vfov, double ol ) {
  
  /* top and bottom pitches of the rectangle */
  double top =  pitch + (int)( 0.5*vfov );
  double bottom = pitch - (int)( 0.5*vfov );
  
  /* distance of pitch from equator (just |pitch|) */ 
  double eqdistop = fabs(top);
  double eqdisbot = fabs(bottom);
  
  /* factor in overlap */
  hfov *= (1.0 - 0.02*ol);
  
  /* Length of a rectangle's side (top or bottom) when projected
   * onto sphere is (hfov * overlap factor) * cos( pitch ). 
   * See pdf for derivation. */

  /* top closer to equator, use top */
  if(eqdistop < eqdisbot) {
    /* round to nearest 10th of degree & return */
    double roundtop = (int) 10.0*hfov/cos( deg2rad*top ) + 0.5;
    return 0.1*roundtop; 
  }
  
  /* bottom closer to equator, or they're the same, use bottom */
  else {
    /* round to nearest 10th of degree & return */
    double roundbot = (int) 10.0*hfov/cos( deg2rad*top ) + 0.5;
    return 0.1*roundbot;
  }
}  

/*** DEPRECATED but possibly still useful *******/

/* int isInRange( boundary point, boundary point, 
 *                current point, buffer )
 * 
 * Returns true (non-zero int) or false (0) depending on whether 
 * or not the current point's yaw is in the interval given by 
 * (least allowable yaw - buffer, greatest allowable yaw + buffer)
 */
int isInYawRange( point* b1, point* b2, point* c, double buff ){
  
  /* boundaries within 1/10th of degree - return false */
  if( fabs(b1->y - b2->y) < 0.1 )
    return 0;
  
  /* negest is lowest allowable yaw */
  double negest = b1->y < b2->y ? b1->y : b2->y;
  /* posest is  highest allowable yaw */
  double posest = b1->y < b2->y ? b2->y : b1->y;
  return ( negest - buff < c->y ) && ( c->y < posest );      
}


/* int isInPitchRange ( boundary point, boundary point, 
 *                      current point, buffer )
 * 
 * Returns true (non-zero int) or false (0) depending on whether 
 * or not the current point's pitch is in the interval given by 
 * (least allowable pitch - buffer, greatest allowable pitch + buffer)
 */
int isInPitchRange( point* b1, point* b2, point* c, double buff ){
  
  /* boundaries within 1/10th of degree - return false */
  if( fabs(m->p - M->p) < 0.1 )
    return 0;
  
  /* negest is lowest allowable pitch */
  double negest = b1->p < b2->p ? b1->p : b2->p;
  /* posest is  highest allowable pitch */
  double posest = b1->p < b2->p ? b2->p : b1->p;
  return ( negest - buff < c->p ) && ( c->p < posest + buff );      
}

#endif /* GIGAPAN */
