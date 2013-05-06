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
 * Last Modified: May 2 2013                  *
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/************Auxiliary functions, constants, structures**************/

/* Conversion factors between degrees and radians. */
#define pi 3.14159265359
double deg2rad = pi/180.0;
double rad2deg = 180.0/pi;

/* Constraints on start point yaw and pitch */
#define MAX_S_Y 180.0 
#define MIN_S_Y -180.0
#define MAX_S_P 90.0
#define MIN_S_P -90.0

/* Start point ranges for printing convenience */
#define S_Y_RANGE "[-180.0,180.0]"
#define S_P_RANGE "[-90.0,90.0]"

/* Constraints on how far panorama can travel */
#define R_EDGE 180.0
#define L_EDGE -180.0
#define TOP_EDGE 90.0
#define BOT_EDGE -90.0

/* Panorama max travel ranges for printing convenience */
#define R_RANGE "[0.0,180.0]"
#define L_RANGE "[-180.0,0.0]"
#define UP_RANGE "[0.0,90.0]"
#define DOWN_RANGE "[-90.0,0.0]"


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
  return 2*rad2deg*atan( 0.5*d/f );
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
 * DEPENDS ON SPECIFIC IMU SPHERE PARAMETRIZATION
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
      ++modcount;
    }
    /* n even - you've added int*360, do nothing */
    /* n odd - you're in the other hemisphere now */
    if ( modcount % 2 ) ty += 180.0;    
  } 
  /* same explanation as 1st if */
  else if( 180.0 < ty ) {
      while( 180.0 < ty ) { 
        ty -= 180.0; 
        ++modcount;
      }
      if( modcount % 2 ) ty += -180.0;
    }

  /* reset for pitches */
  modcount = 0;

  /* sum & "mod" pitches */
  double tp = c->p + s->p; 
  
  if( tp < -90.0 ) {
    while( tp < -90.0 ) { 
      tp += 180.0; 
      ++modcount;
    }
    /* again, adding 180 is a hemisphere switch */
    if( modcount % 2 ) tp *= -1.0;
  
  } 
  else if( 90.0 < tp ) {
    while( 90.0 < tp ) { 
      tp -= 180.0; 
      ++modcount;
    }
    if( modcount % 2 ) tp *= -1.0;
  }

  point tpt = { ty, tp };
  return tpt;
}

/* void printPt ( point to print, output file pointer ) 
 * 
 * Prints out coordinates of the given point, to tenth of degree
 * accuracy, in the following format:
 * yaw<tab>pitch<newline>
 */
void printPt( point x, FILE* outf ) {
  fprintf( outf, "%.1f\t%.1f\n", x.y, x.p );
}

/* double yawDelta ( current point, horizontal field of view, 
 *                   vertical field of view, horizontal overlap ) 
 *
 * Calculates appropriate yaw increment for a given point in param domain, 
 * fields of view, and overlap
 *
 * DEPENDS ON SPECIFIC IMU SPHERE PARAMETERIZATION
 */
double yawDelta( int pitch, int hfov, int vfov, double ol ) {
  
  /* top and bottom pitches of the rectangle */
  double top =  pitch + (int)( 0.5*vfov );
  double bottom = pitch - (int)( 0.5*vfov );
  
  /* distance of pitch from equator (just |pitch|) */ 
  double eqdistop = fabs(top);
  double eqdisbot = fabs(bottom);
  
  /* factor in overlap */
  hfov *= (1.0 - 0.01*ol);
  
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

#endif /* GIGAPAN */
