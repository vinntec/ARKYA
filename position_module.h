#define COMPASS_PORT S2
#define COMPASS_MAX 359

#define RAD_TO_DEG 57.296

int current_angle = 0;
int variable_offset = 0;

inline int readCurrentOrientation(int global_offset)
{
       int val = SensorHTCompass(COMPASS_PORT) - global_offset;
       if(val >= 0) return val;
       return val + COMPASS_MAX;
}

#define ORIENT_AM_WEIGHT 5
#define ORIENT_AM_PRECISION 5

//Utilities -------------------------------------------------------------------------

inline int square_distance(int x1, int y1, int x2, int y2)
{
     int dx = x1 - x2;
     int dy = y1 - y2;
     
     return dx*dx + dy*dy;
}

inline int square_distance_to_destination()
{
     return square_distance(dest_x, dest_y, pos_x, pos_y);
}

inline int destination_angle()
{

     int dy = dest_y - pos_y;
     int dx = dest_x - pos_x;
     
     if(dx == 0)
           return 180*(dy < 0);
     
     int theta = atan(dy/dx)*RAD_TO_DEG;
     
     if(dx > 0)
     {
          if(dy > 0)
              return 90 - theta;
          else if(dy == 0)
              return 90;
          else
              return 90 - theta;
     }
     else
     {
          if(dy > 0)
              return 270 - theta;
          else if(dy == 0)
              return -90;
          else
              return 270 - theta;
     }
}

inline int angle_barycenter(int npos_x, int npos_y)
{
     float dist1 = 1.0/pow(square_distance(npos_x, npos_y, P1_X, P1_Y),16);
     float dist2 = 1.0/pow(square_distance(npos_x, npos_y, P2_X, P2_Y),16);
     float dist3 = 1.0/pow(square_distance(npos_x, npos_y, P3_X, P3_Y),16);
     float dist4 = 1.0/pow(square_distance(npos_x, npos_y, P4_X, P4_Y),16);
     float dist5 = 1.0/pow(square_distance(npos_x, npos_y, P5_X, P5_Y),16);
     
     return (dist1 * P1_A + dist2 * P2_A + dist3 * P3_A + dist4 * P4_A + dist5 * P5_A)
                     /(dist1 + dist2 + dist3 + dist4 + dist5);
}


//Tasks -----------------------------------------------------------------------------
int calc = 0;

/**
 *  This task's aim is to constantly provide x, y and a "region" inside the
 *  territory, given according to the following values:
 *   -> POS_REGION_HOME
 *   -> POS_REGION_BORDER
 *  It is of course recommended to ignore certain line scans when in home region.
 *  The module also takes into account whether the initial position is known or not.

 *  In a first simple version, this module will try to know the position at all times.
 *  In a further version, this might take a little much CPU power, and so
 *  it might be interesting to change.

 *  The aim of this module is to define the position at a 5 cm radius.
 **/
task positioning()
{
    //Init
	pos_x = POS_INIT_KNOWN_X;
    pos_y = POS_INIT_KNOWN_Y;
    pos_region = POS_REGION_HOME;

    //This is where it is important to respect init constraints!!!
    current_angle = pos_orientation_angle;
    int current_motor_state = MOTORS_NONE;
    int prec_distance = 0;
    int prec_pos_x = pos_x;
    int prec_pos_y = pos_y;
    int diff = 0;

    init_counter++;

    //Main routine
    while(1)
    {
         //Check motors.
         //If any movement, update positions with relative values,
         // computed from the current orientation value, the speed, and any distance
         // indications.
         
         #define ADV_SIGN ((current_motor_state == MOTORS_STRAIGHT)*2 - 1)

         if(current_motor_state == MOTORS_STRAIGHT || current_motor_state == MOTORS_BACK) //If something is happening
         {
             //Update position
             diff = distance - prec_distance;

			 #define X (sin(current_angle/RAD_TO_DEG)*diff)*ADV_SIGN + prec_pos_x
			 #define Y (cos(current_angle/RAD_TO_DEG)*diff)*ADV_SIGN + prec_pos_y
			 #define MIN_MAX(u,v) 0.5 * (0.5 * (u + abs(u)) + v - abs(0.5 * (u + abs(u)) - v))
			 #define MIN(u,v) 0.5 * (u + v - abs(u - v))
			 #define MAX(u,v) 0.5 * (u + v + abs(v - u))
			 #define MIN_MAX_Y(Ymax, v) MIN(Ymax, MAX(10,  v));

             pos_x = MIN_MAX(X, POS_MAX_X);
             pos_y = MIN_MAX_Y(POS_MAX_Y, Y);

             if(real_motors_state != current_motor_state) {
                 current_motor_state = MOTORS_NONE;
                 prec_distance = distance;

                 prec_pos_x = pos_x;
                 prec_pos_y = pos_y;
                 calc = 0;
             }
         }
         else if(real_motors_state == MOTORS_STRAIGHT || real_motors_state == MOTORS_BACK) //If something just started happening
         {
             current_motor_state = real_motors_state;
             current_angle = pos_orientation_angle;
             calc = ADV_SIGN;
         }

         //Check sensors.
         //If any obstacles, update positions in absolute values
         switch(obstacle_type)
         {
             case OBSTACLE_WALL:
                  //Ok, but which wall is  it??
                  if(pos_orientation_angle <= 90)
                      //Right wall
                      pos_x = POS_MAX_X - (US_SENSOR_OFFSET + obstacle_distance)/2 *sin(pos_orientation_angle/RAD_TO_DEG);
                  else if(pos_orientation_angle >= 270)
                      //Left wall
                      pos_x = -(US_SENSOR_OFFSET + obstacle_distance)/2 *sin(pos_orientation_angle/RAD_TO_DEG);
                  else
                  {
                      if(pos_orientation_angle < 180)
                      {
                          if(POS_MAX_X > pos_x - pos_y * tan(pos_orientation_angle/RAD_TO_DEG))
                              pos_y = -(US_SENSOR_OFFSET + obstacle_distance)/2 *cos(pos_orientation_angle/RAD_TO_DEG);
                          else
                              pos_x = POS_MAX_X - (US_SENSOR_OFFSET + obstacle_distance)/2 *sin(pos_orientation_angle/RAD_TO_DEG);
                      }
                      
                      else
                      {
                          if(pos_x > pos_y * tan(pos_orientation_angle/RAD_TO_DEG))
                              pos_y = -(US_SENSOR_OFFSET + obstacle_distance)/2 *cos(pos_orientation_angle/RAD_TO_DEG);
                          else
                              pos_x = -(US_SENSOR_OFFSET + obstacle_distance)/2 *sin(pos_orientation_angle/RAD_TO_DEG);
                      }
                      
                  }
                  
                  //Stop session!
                  
                  current_motor_state = MOTORS_NONE;
                  prec_distance = distance;

                  prec_pos_x = pos_x;
                  prec_pos_y = pos_y;
                  calc = 0;
                  
                  break;
             case OBSTACLE_LINE:
                  //Depends on region
                  if(pos_region == POS_REGION_BORDER)
                      //Easy, set pos_y to maximum
                      pos_y = POS_MAX_Y - distance_from_obstacle/2;
                  else
                  {
                      //Well it depends; depending on previous x and y,
                      //it might be possible to get a better idea of position.
                      //But that requires sampling.
                  }
                  break;
             default:
                  break;
         }

         //Deduce region.

         if(pos_y > POS_REGION_LIMIT_Y)
             pos_region = POS_REGION_BORDER;
         else
             pos_region = POS_REGION_HOME;
    }
}


/**
 * This task constantly gives a value and a status for the robot's orientation.
 * Status values are:
 *  -> OK
 *  -> UNSTABLE
 * Depending on the definition of simple_mode, the value of status will be updated
 * or not. In (not simple mode), the module will set its status to ADJUSTING
 * whenever the orientation given is not precise up to ORIENT_AM_PRECISION degrees!
 * In the end, advanced mode might not be necessary...
 **/
task orientation()
{
    //Init: the robot *must* be pointing north as much as possible.
    //Failing to do so will only result in a wrong calibration.

    SetSensorLowspeed(IN_2);

    //This seems to be necessary, else the offset is wrong.
    Wait(1000);

    const int room_offset = SensorHTCompass(S2);
    #ifdef BARYCENTRIC_ANGLE
           variable_offset = angle_barycenter(POS_INIT_KNOWN_X,POS_INIT_KNOWN_Y);
    #endif

    #ifndef POS_ORIENT_SIMPLE_MODE
         //In advanced mode, define an array for the last AM_WEIGHT positions
         int current_value = 0;
         int current_middle = 0;
         int diff = 0;
    #endif

    //Confirm initialisation
    init_counter++;

    /* This is the main routine
     * Constantly update the compass variable!
     */
    while(1)
    {
         #ifdef POS_ORIENT_SIMPLE_MODE
              #ifdef BARYCENTRIC_ANGLE
                   variable_offset = angle_barycenter(pos_x,pos_y);
                   pos_orientation_angle = readCurrentOrientation(room_offset - variable_offset);
              #else
                   pos_orientation_angle = readCurrentOrientation(room_offset);
              #endif
              Wait(POS_ORIENT_RDELAY);
         #else
              current_value = readCurrentOrientation(global_offset);
              current_middle = ((ORIENT_AM_WEIGHT-1)*current_middle + current_value)/ORIENT_AM_WEIGHT;
              if((diff = abs(current_middle-current_value)) < ORIENT_AM_PRECISION
                       || (360 - diff) < ORIENT_AM_PRECISION)
                  pos_orientation_status = POS_STATUS_OK;
              else
                  pos_orientation_status = POS_STATUS_UNSTABLE;

              Wait(POS_ORIENT_RDELAY);
         #endif
    }
}
