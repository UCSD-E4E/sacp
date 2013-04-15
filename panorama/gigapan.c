/********************************************** 
 * UCSD NGS Stabilized Aerial Camera Platform *
 * Gigapan Coordinate Generator               *
 *                                            *
 * File: UCSD-E4E/sacp/panorama/gigapan.c     *
 * Requires ./gigapan.h                       *
 *                                            *
 * Author: Sergei I. Radutnuy                 *
 *         sradutnu@ucsd.edu                  *
 *                                            *
 * Last Modified: April 14 2013               *
 **********************************************/
#include "gigapan.h"

void usage() {
  puts( "\nTo enter command line arguments and skip dialog:\n\n" );
  puts( "gigapan <focal length> <sensor width> <sensor height> " );
  puts( "<start yaw> <start pitch> <how right> <how left> <how up> " ); 
  puts( "<how down> <horizontal overlap> <vertical overlap>\n\n" );
  puts( "There should be 11 arguments total, and this message will be " );
  puts( "printed again if there is an error in any of them.\n" );
}

/**** EVERYTHING HERE DEPENDS ON SPECIFIC SPHERE/IMU PARAMETERIZATION ********/
int main( int argc, char** argv ) {

  /****************** Parameters and Variables******************/
  
  /* focal length, image sensor dimensions */
  double flength, sensw, sensh;
  
  /*allocate start/min/max coords */
  point* start = (point*)mallox(sizeof(point));
  point* max = (point*)malloc(sizeof(point)); 
  point* min = (point*)malloc(sizeof(point));

  if( !( start && min && max ) ) {
    printf("There was a problem allocating memory.");
    return -1;
  }
  
  /* horizontal and vertical overlap */
  double hover, yover;
  
  /* output filename */
  char* filename = "coords.txt";

  /***** Command line args processing (could also use interactive dialog) ****/

  /*TODO: If you're using command line arguments to save time: */
  if( argc == 12 ) {

    /* boolean to see if der vass problem */
    int der_vass_problem = 0;
    
    /* interpret every argument string as a double, do checks for correctness */
    flength = strtod( argv[1] );
    if( !(0 < flength) ) {
      puts( "Focal length needs to be greater than 0." );
      ++der_vass_problem;
    }

    sensw = strtod( argv[2] );
    
    sensh = strtod( argv[3] ); 

    start->y = strtod( argv[1] ); 
    start->p = strtod( argv[1] ); 
    max->y = strtod( argv[1] ); 
    min->y = strtod( argv[1] ); 
    max->p = strtod( argv[1] ); 
    min->p = strtod( argv[1] ); 
    hover = strtod( argv[1] ); 
    yover = strtod( argv[1] );

    if( der_vass_problem ) {
      
    }
  }
  /******** Interactive dialog (if cmd args wrong or not used *****************/
  else if( argc == 1 ) {
    puts( "\nWhat are the camera's focal length, image sensor width and height?");
    puts( "Please enter 3 numbers greater than 0. Unit of length doesn't matter");
    puts( "as long as all 3 values use the same one".)
    scanf( "%lf %lf %lf", &flength, &sensw, &sensh );

    puts( "Where is the gigapan going to start? Please enter a yaw and a" );
    puts( "pitch, in degrees (-180 <= yaw <= 180, -90 <= pitch <= 90." );
    scanf( "%lf %lf", &(start->y), &(start->p) );
  
    puts( "How far right should the gigapan go? Enter a positive number of" );
    puts( "degrees (from 0 to 180)" );
    scanf( "%lf", &(max->y) );
  
    puts( "How far left should the gigapan go? Enter a negative number of" );
    puts( "degrees (from 0 to -180)" );
    scanf("%lf", &(min->y) );

    puts( "How far up should the gigapan go? Enter a positive number of" );
    puts( "degrees (from 0 to 90)" );
    scanf( "%lf", &(max->p) );

    puts( "How far down should the gigapan go? Enter a negative number of" );
    puts( "degrees (from 0 to -90)" );
    scanf( "%lf", &(min->p) );
  
    puts( "\nPercent horizontal and vertical overlap between images?" );
    puts( "This is the minimum amount by which a picture overlaps with its" );
    puts( "neighbor. A negative percentage puts space between images. Please" );
    puts( " enter 2 numbers between -100 and 100, horizontal then vertical.\n" );
    scanf( "%lf %lf", &hover, &yover );

    puts( "\nNote: for future reference, if you would like to skip this " );
    puts( "dialog and simply enter the command line arguments when calling ");
    puts( "the executable, here is the format for that:\n" );
    usage();
  }

  else {
    puts( "\nIt looks like you had the wrong number of command line; " );
    puts( "you should have either none or 11.\n" );
    usage();
  }
  
  /********* Start of gigapan loops and coordinate printing *******************/  
  
  /* open output file, check for errors */
  FILE* output = fopen( filename, "w" );
  
  if( !output ) {
    printf( "Failed to open a file for output" );
    return -1;
  }
  
  /* allocate "current" point in gigapan, representing how far the pan has 
   * moved away from the start point */
  point* curr = (point*)malloc(sizeof(point)); 
  
  if( !curr ) {
    printf("There was a problem allocating memory.");
    return -1;
  }
  
  *curr = { 0.0, 0.0 };
  
  /* direction for yaw increment (goes back & forth on rows).
   * starts out going right (+1) then alternates (-1)^(row#-1) */
  double hdir = 1.0;

  /* separate variable for horizontal field of view, will be use
   * multiple times to determine the yaw increment*/
  double HFOV = fov( sensw, flength );
  
  /* the yaw increment is a nonlinear function of pitch, to be
   * calculated in the loop */
  double ydelta = 0.0;
  
  /* pitch incremement for the pan, stays the same throughout.
   * Simply vertical field of view with overlap factored in.*/ 
  double pdelta = (1.0 - 0.02*yover)*fov( sensh, flenght );
 
  /* Top half first. Starts at start point, goes up to max pitch 
   * zig-zagging the rows. */
  do {
    ydelta = yawDelta( (curr->p + start->p), HFOV, VFOV, hover );  
    do  {
      printPt( shiftPt( curr, start ), output );
      
      curr->y += hdir*ydelta;
    } while( -180.0 - ydelta < curr->y && curr->y < -180.0 + ydelta ) ;
    
    hdir *= -1.0;
    curr->p += pdelta;
    printPt( shiftPt( curr, start), output );
  } while( isInPitchRange( min, max, curr, pdelta ) ); 

  *curr = { 0.0, 0.0 };
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

  /* close the output file */
  if( fclose(output) ) {
    printf("There was a problem closing the output file");
    return -1;
  }
  
  printf( "\nThe program has completed successfully. Check \"coords.txt\"" );
  printf( "in the current directory.\n" );
  
  /* memory management */
  free(start); free(min); free(max); free(curr); free(middle);

  return 0;
}
