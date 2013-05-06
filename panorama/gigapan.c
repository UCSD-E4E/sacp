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
 * Last Modified: May 2 2013                  *
 **********************************************/

#include "gigapan.h"

void   usage() {
  puts( "\nTo enter command line arguments and skip dialog:\n" );
  
  puts( "gigapan <focal length> <sensor width> <sensor height>"    );
  puts( "        <start yaw> <start pitch> <how right> <how left>" );
  puts( "        <how up> <how down> <horizontal overlap>"         );
  puts( "        <vertical overlap>\n"                               );

  printf( "There should be 12 arguments total, and this message will be\n " );
  printf( "printed again if there is an error in any of them.\n"            );
}


/**** EVERYTHING HERE DEPENDS ON SPECIFIC SPHERE/IMU PARAMETERIZATION ********/
int main( int argc, char** argv ) {

  /****************** Parameters and Variables *******************************/
  
  /* focal length, image sensor dimensions */
  double flength, sensw, sensh;

  /* horizontal and vertical overlap */
  double hover, yover;

  /* boolean for whether or not you want the panorama optimized */
  int opt = 0;
  
  /* output filename */
  char* filename = "coords.txt";

  /*allocate start/corner coords */
  point* start     = (point*)malloc(sizeof(point));
  point* top_right = (point*)malloc(sizeof(point)); 
  point* bot_left  = (point*)malloc(sizeof(point));

  if( !( start && top_right && bot_left ) ) {
    fprintf( stderr, "There was a problem allocating memory." );
    return -1;
  }
  

  /***** Command line args processing (could also use interactive dialog) ****/
  
  /* If you're using command line arguments to save time: */
  if( argc == 13 ) {

    /* int/boolean for number of problems encountered */
    int problem = 0;
    
    /* interpret every argument string as a double, do checks for correctness */
    sscanf( argv[1], "%lf", &flength );
    if( !( 0.0 < flength ) ) {
      fprintf( stderr, "\nFocal length needs to be greater than 0.\n" );
      ++problem;
    }

    sscanf( argv[2], "%lf", &sensw );
    if( !( 0.0 < sensw ) ) {
      fprintf( stderr, "\nSensor width needs to be greater than 0.\n" );
      ++problem;
    }
    
    sscanf( argv[3], "%lf", &sensh );
    if( !( 0.0 < sensh ) ) {
      fprintf( stderr, "\nSensor height needs to be greater than 0.\n" );
      ++problem;
    }

    sscanf( argv[4], "%lf", &(start->y) ); 
    if( start->y < MIN_S_Y || MAX_S_Y < start->y ) {
      fprintf( stderr, "\nStarting yaw needs to be in interval %s\n", 
               S_Y_RANGE );
      ++problem;
    }

    sscanf( argv[5], "%lf", &(start->p) ); 
    if( start->p < MIN_S_P || MAX_S_P < start->p ) {
      fprintf( stderr, "\nStarting pitch needs to be in interval %s\n", 
               S_P_RANGE );
      ++problem;
    }

    sscanf( argv[6], "%lf", &(top_right->y) );
    if( top_right->y < 0.0 || R_EDGE < top_right->y ) {
      fprintf( stderr, 
               "\nAmount panorama goes to the right must be in interval %s\n",  
               R_RANGE );
      ++problem;
    }

    sscanf( argv[7], "%lf", &(bot_left->y) ); 
    if( bot_left->y < L_EDGE || 0.0 < bot_left->y ) {
      fprintf( stderr, 
               "\nAmount panorama goes to the left must be in interval %s\n", 
               L_RANGE );
      ++problem;
    }
    
    sscanf( argv[8], "%lf", &(top_right->p) );
    if( top_right->p < 0.0 || TOP_EDGE < top_right->p ) {
      fprintf( stderr, "\nAmount panorama goes up to be in interval %s\n", 
            UP_RANGE );
      ++problem;
    }
    
    sscanf( argv[9], "%lf", &(bot_left->p) );
    if( bot_left->p < BOT_EDGE || 0.0 < bot_left->p ) {
      fprintf( stderr, 
               "\nAmount panorama goes down needs to be in interval %s\n",
               DOWN_RANGE );
      ++problem;
    }

    sscanf( argv[10], "%lf", &hover );
    if( hover < -100.0 || 100.0 < hover ) {
      fprintf( stderr, "\nHorizontal overlap percentage needs to be in range " );
      fprintf( stderr, "[-100.0,100.0] \n" );
      ++problem;
    }
    
    sscanf( argv[11], "%lf", &yover );
    if( yover < -100.0 || 50.0 < yover ) {
      fprintf( stderr, "\nVertical overlap percentage needs to be in range " );
      fprintf( stderr, "[-100.0,50.0] \n" );
      ++problem;
    }
    
    sscanf( argv[12], "%d", &opt );
    
    if( problem ) {
      usage();
      printf("\nThere were %d problems total\n", problem);
      return -1;
    }
  }


  /******** Interactive dialog (if cmd args wrong or not used *****************/
  else if( argc == 1 ) {
    puts( "\nWhat are the camera's focal length, image sensor width and " );
    puts( "height? Please enter 3 numbers greater than 0. Any unit of "   );
    puts( "length works, as long as all 3 values use the same unit.\n"    );
    scanf( "%lf %lf %lf", &flength, &sensw, &sensh );

    puts( "\nWhere is the gigapan going to start? Please enter a yaw and a" );
    printf( "pitch, in degrees, in the range %s, %s,\nrespectively.\n\n",
            S_Y_RANGE, S_P_RANGE                                          );
    scanf( "%lf %lf", &(start->y), &(start->p) );
  
    puts( "\nHow far right should the gigapan go? Enter a number of" );
    printf( "degrees in range %s\n\n", R_RANGE                       );
    scanf( "%lf", &(top_right->y) );
  
    puts( "\nHow far left should the gigapan go? Enter a number of degrees " );
    printf( "in range %s (negative means further left)\n\n", L_RANGE         );
    scanf("%lf", &(bot_left->y) );

    puts( "\nHow far up should the gigapan go? Enter a number of degrees " );
    printf( "in range %s\n\n", UP_RANGE                                    );
    scanf( "%lf", &(top_right->p) );

    puts( "\nHow far down should the gigapan go? Enter a number of" );
    printf( "degrees in range %s\n\n", DOWN_RANGE                      );
    scanf( "%lf", &(bot_left->p) );
  
    puts( "\nPercent horizontal, vertical overlap between images? This is " );
    puts( "the minimum amount by which a picture overlaps with its "        );
    puts( "neighbor. A negative percentage puts space between images. "     );
    puts( "Enter 2 numbers between -100.0 and 50.0, horizontal then "      );
    puts( "vertical.\n"                                                     );
    scanf( "%lf %lf", &hover, &yover );

    puts( "\nWould you like this panorama optimized? 0 for no, any nonzero" );
    puts( "integer for yes.\n"                                              );
    scanf( "%d", &opt );

    puts( "\nNote: for future reference, if you would like to skip this "    );
    puts( "dialog and simply enter the command line arguments when calling " );
    puts( "the executable, here is the format for that:\n"                   );
    usage();
  }

  else {
    fprintf( stderr, "\nIt looks like you had the wrong number of command " );
    fprintf( stderr, "line args;\nyou should have either none or 12.\n"     );
    usage();
    return -1;
  }
  
  /********* Start of gigapan loops and coordinate printing *******************/  
  
  /* open output file, check for errors */
  FILE* output = fopen( filename, "w" );
  
  if( !output ) {
    fprintf( stderr, "\nFailed to open a file for output" );
    return -1;
  }
  
  /* allocate "current" point in gigapan, representing how far the pan has 
   * moved away from the start point */
  point* curr = (point*)malloc(sizeof(point)); 
  
  if( !curr ) {
    fprintf( stderr, "\nThere was a problem allocating memory." );
    return -1;
  }
  
  curr->y = 0.0; curr->p = 0.0;
  
  /* direction for yaw increment (goes back & forth on rows).
   * starts out going right (+1) then alternates (-1)^(row#-1) */
  double hdir = 1.0;

  /* horizontal and vertical fields of view */
  double HFOV = fov( sensw, flength );
  double VFOV = fov( sensh, flength ); 
 
  /* yaw incremement - defaults to HFOV w/ overlap factored in, 
   * optimized using the yawDelta function if optimization is chosen */
  double ydelta = (1.0 - 0.01*hover)*HFOV;
  
  /* pitch incremement for the pan, stays the same throughout.
   * Simply vertical field of view with overlap factored in.*/ 
  double pdelta = (1.0 - 0.01*yover)*VFOV;
 
  /* print the first point */
  printPt( *start, output );

  /* Top half first. Starts to the right of start point, zigzags across rows */
  /* begin loop for top half pitches */
  do {
    /* optimize the yaw increment */
    if( opt ) ydelta = yawDelta( (curr->p + start->p), HFOV, VFOV, hover );  
    
    /* loop for a single yaw row */ 
    do  {
      /* increment the yaw */
      curr->y += hdir*ydelta;
      /* print current point in row */
      printPt( shiftPt( curr, start ), output );
      /* until the yaw is out of bounds */
    } while( bot_left->y - ydelta < curr->y 
             && curr->y < top_right->y + ydelta ) ;
    
    /* change the yaw increment direction */ 
    hdir *= -1.0;
    /* increment the pitch */
    curr->p += pdelta;
    /* until the pitch is higher than the top */
  } while( curr->p < top_right->p + pdelta ); 

  /* reset the current displacement from start point */
  curr->p = 0.0; curr->y = 0.0;
  /* increment the yaw once, so as not to double print the middle point*/
 /*  if( opt ) ydelta = yawDelta( start->p, HFOV, VFOV, hover ); 
  curr->y = -1.0*ydelta; */
  /* start moving in opposite direction of first half of middle row */
  hdir = -1.0;

  /* Begin loop for bottom half pitches */
  do {
    /* optimize yaw increment */
    if( opt ) ydelta = yawDelta( (curr->p + start->p), HFOV, VFOV, hover ); 
    
    /* loop for single yaw row */
    do {  
      /* incremement the yaw */
      curr->y += hdir*ydelta;
      /* print current point in row */
      printPt( shiftPt( curr, start ), output );
      /* until the yaw is out of range */
    } while( bot_left->y - ydelta < curr->y 
             && curr->y < top_right->y + ydelta );
    
    /* change yaw incremement direction */
    hdir *= -1;
    /* update pitch */
    curr->p -= pdelta;
    /* until the pitch hits the bottom */
  } while( bot_left->p - pdelta < curr->p ); 

  /* close the output file */
  if( fclose(output) ) {
    fprintf( stderr, "\nThere was a problem closing the output file" );
    return -1;
  }
  
  puts( "\nThe program has completed successfully. Check \"coords.txt\"" );
  puts( "in the current directory.\n" );
  
  /* memory management */
  free(start); free(top_right); free(bot_left); free(curr); 

  return 0;
}
