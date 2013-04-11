/********************************************** 
 * UCSD NGS Stabilized Aerial Camera Platform *
 * Gigapan Coordinate Generator               *
 *                                            *
 * gigapan.ci:                                *
 * main program here - requires gigapan.h     *
 *                                            *
 * Compatibility: ANSI C and up               *
 *                                            *
 * See README or gigapancoords.pdf for an     *
 * explanation of the program/usage           *
 *                                            *
 * @author Sergei I. Radutnuy                 *
 *         sradutnu@ucsd.edu                  *
 *                                            *
 * Last Modified: April 7 2013                *
 **********************************************/
#include "gigapan.h"

int main( int argc, char** argv ) {

  /****************** Parameters and Variables******************/
  
  /* focal length, image sensor dimensions */
  double flength, sensw, sensh;
  
  /* min/max coords */
  point* max = (point*)malloc(sizeof(point)); 
  point* min = (point*)malloc(sizeof(point));
  
  if( !( min && max ) ) {
    printf("There was a problem allocation memory.");
    return -1;
  }
  
  /* horizontal and vertical overlap */
  double hover, yover;
  
  /* output filename */
  char* filename = "coords.txt";
  
  /***************** Interactive dialog to record parameters ******************/

  puts( "\nWhat are the camera's focal length, image sensor width and height?");
  puts( "Please enter 3 values greater than 0, in millimeters.\n");
  scanf( "%lf %lf %lf", &flength, &sensw, &sensh );

  puts( "\nWhat is the min/max yaw? Please enter two values, in degrees," );
  puts( "between -180 and 180.\n" );
  scanf( "%lf %lf", &(min->y), &(max->y) );

  puts( "\nWhat is the min/max pitch? Two values between -90 and 90.\n" );
  scanf( "%lf %lf", &(min->p), &(max->p) );
  
  puts( "\nPercent horizontal and vertical overlap between images?");
  puts( "(This is the minimum amount by which a picture overlaps with its");
  puts( "neighbor. A negative percentage puts space between images).");
  puts( "Please enter 2 numbers between -100 and 100)\n");
  scanf( "%lf %lf", &hover, &yover );

  /************** Initial coord & pre-gigapan calculations ********************/

  
  /* Horizontal/Vertical Field of View = 
   * 2arctan(image sensor (width or height) / 2*focal length) */
  int HFOV = 2*rad2deg*atan( sensw/(2.0*flength) );
  int VFOV = 2*rad2deg*atan( sensh/(2.0*flength) );
  
  /* Create & set the current and "middle" points in the gigapan.
   * curr: the coordinate that is updated and printed out in the
   *       gigapan loops
   * middle: the coordinate whose yaw/pitch is the average of
   *         the min & max yaw/pitch
   */
  point* curr = (point*)malloc(sizeof(point)); 
  point* middle = (point*)malloc(sizeof(point));
  
  if( !( curr && middle ) ) {
    printf("There was a problem allocation memory.");
    return -1;
  }
  
  middle->y = 0.5*(min->y + max->y);  middle->p = 0.5*(min->p + max->p);
  curr->y = middle->y;                curr->p = middle->p;
  
  /********* Start of gigapan loops and coordinate printing *******************/  
  
  FILE* output = fopen( filename, "w" );
  if( !output ) {
    printf( "Failed to open a file for output" );
    return -1;
  }
  
  /* direction for yaw increment (goes back & forth on rows).
   * starts out going right (+1) then alternates (-1)^(row#-1) */
  int hdir = 1;

  /* the gigapan will incremement the pitch by the same 
   * amount every time - the VFOV scaled down by overlap 
   */
  double pdelta = (1.0 - 0.02*yover)*VFOV;
  
  /* the yaw increment is a nonlinear function of pitch, to be
   * calculated in the loop */
  double ydelta = 0;

  printPt( curr, output );
  
  /* Top half first. Starts in middle of range, goes up
   * zig-zagging the rows. */
  do {
    ydelta = yawDelta( curr, HFOV, VFOV, hover );  
    do  {
      curr->y += hdir*ydelta;
      printPt( curr, output );
    } while( isInYawRange( min, max, curr, ydelta ) );
    
    hdir *= -1;
    curr->p += pdelta;
    printPt( curr, output );
  } while( isInPitchRange( min, max, curr, pdelta ) ); 

  /* Start bottom half by filling in left part of middle row 
   * (the top half loops only cover the right part) */
  curr->y = middle->y;
  curr->p = middle->p;
  hdir = -1;  

  /* Bottom half. Starts in middle of range, goes down
   * zig-zagging the rows. */
  do {
    ydelta = yawDelta( curr, HFOV, VFOV, hover); 
    do {  
      curr->y += hdir*ydelta;
      printPt( curr, output );
    } while( isInYawRange( min, max, curr, ydelta ) );
    
    hdir *= -1;
    curr->p -= pdelta;
    printPt( curr, output );
  } while( isInPitchRange( min, max, curr, pdelta ) ); 

  if( fclose(output) ) {
    printf("There was a problem closing the output file");
    return -1;
  }
  
  printf( "The program has completed successfully. Check \"coords.txt\"" );
  printf( "in the current directory.\n" );
  
  /* memory management */
  free(min); free(max); free(curr); free(middle);

  return 0;
}
