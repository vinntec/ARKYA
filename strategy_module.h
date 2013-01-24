/* This file defines the strategy module.
   It's based on the overview state diagrams. */

//Known positions ---------------------------
//#define KNOWN_POSITIONS 2
#define KP_1_X 50
#define KP_2_X 30
#define KP_1_Y 60
#define KP_2_Y 70

//Linear search strategy constants
#define LS_INIT_X 7
#define LS_INIT_Y 7
#define LS_SWEEP_INTERVAL 30

#define HS_SQUARE_DISTANCE 30
#define DIST_REACHED_DEST 10

#define MOTORS_FULL_SPEED MOTORS_SPEED_FAST
#define MOTORS_PRUDENCE_SPEED MOTORS_SPEED_SLOW



int state =  ST_INIT;
int prev_obst = OBSTACLE_NONE;

#define DEST_NOT_REACHED 0
#define DEST_REACHED 1
#define DEST_OBSTACLE 2

int destination_status = 0;

//LOOK_AROUND_DELAY
#define LOOK_AROUND_DELAY 3000

//Catching
#define CATCH_MIN_DIST 6

#define ST_SEARCH_STRATEGY ST_RANDOM_SEARCH

sub go_to_destination_while_checking()
{

long tick = CurrentTick();

destination_status = DEST_NOT_REACHED;

	while(destination_status == DEST_NOT_REACHED)
	{
		while(obstacle_type == OBSTACLE_NONE && square_distance_to_destination() >= HS_SQUARE_DISTANCE)
		{
			aimed_angle = destination_angle();

			motors_state = MOTORS_STRAIGHT;
			motors_speed = MOTORS_FULL_SPEED;
		}

		/*motors_speed = MOTORS_PRUDENCE_SPEED;

		while(obstacle_type == OBSTACLE_NONE && square_distance_to_destination() >= DIST_REACHED_DEST)
		{
			aimed_angle = destination_angle();

			Wait(500);
		} */

		motors_state = MOTORS_NONE;

		if(obstacle_type == OBSTACLE_BALL)
			break;

		destination_status = DEST_REACHED;
	}
}



task strategy()
{
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define NODIR 4

	//Variables
	int dest_defined = 0;
	int direction = NODIR;
	int angle1 = 0;
	int angle2 = 0;
	long t = -1;

  Wait(2000);

#ifdef KNOWN_POSITIONS
	//Look for known positions first.

	int current_kp = 1;


#endif

	//State machine, to be completed....
	while(1)
	{
		switch(state)
		{
		case ST_INIT:
      #ifdef KNOWN_POSITIONS
      if(current_kp <= KNOWN_POSITIONS) {
      switch(current_kp)
		{
		case 1:
			dest_x = KP_1_X;
			dest_y = KP_1_Y;
			break;
		case 2:
			dest_x = KP_2_X;
			dest_y = KP_2_Y;
			break;
		}
		
      go_to_destination_while_checking();
      
      if(obstacle_type == OBSTACLE_BALL)
          state = ST_ADJUST_POSITION;
      else
          state = ST_LOOK_AROUND;
          
      current_kp++;
      
      } else
      
      #endif
			state = ST_SEARCH_STRATEGY;
			break;

			//LINEAR-SEARCH-STRATEGY
			#ifdef LINEAR_SEARCH_STRATEGY
		case ST_GO_TO_INIT_POINT:
			dest_x = LS_INIT_X;
			dest_y = LS_INIT_Y;

			go_to_destination_while_checking();

			if(destination_status == DEST_REACHED)
				state = ST_FOLLOW_DETERMINED_PATH;
			else
			{
				//Ball found!
				//Catch-throw strategy
				state = ST_ADJUST_POSITION;
			}

			break;

		case ST_FOLLOW_DETERMINED_PATH:
			//Check destination settings
			if(!dest_defined)
			{
				switch(direction)
				{
				case NODIR:
					//We're beginning
					//We want to go right
					dest_x = POS_MAX_X - AIMED_OBST_DIST;
					dest_y = pos_y;
					direction = RIGHT;
					break;
				case RIGHT:
					//Check if it's still possible to go up
					if(pos_y + 2*LS_SWEEP_INTERVAL < POS_MAX_Y)
					{
						dest_x = pos_x;
						dest_y = pos_y + LS_SWEEP_INTERVAL;
						direction = UP;
					}
					else //Loop
					state = ST_GO_TO_INIT_POINT;
					break;
				case UP:
					dest_x = AIMED_OBST_DIST;
					dest_y = pos_y;
					direction = RIGHT;
					break;
				}
				
				dest_defined=1;
      }
				go_to_destination_while_checking();

				if(obstacle_type == OBSTACLE_BALL)
					state = ST_ADJUST_POSITION;

      dest_defined = 0;
      break;
      
      #else
      
    case ST_RANDOM_SEARCH:
    
         
         

         if(prev_obst)
         {
             switch(prev_obst)
             {
                 case OBSTACLE_LINE:
                      if(pos_region == POS_REGION_HOME)
                          break;
                      motors_state = MOTORS_BACK;
                      Wait(600);
                      aimed_angle = (pos_orientation_angle + TURN_ANGLE) % 360;
                      break;
                 case OBSTACLE_WALL:
                      motors_speed = MOTORS_SPEED_MEDIUM;
                      motors_state = MOTORS_BACK;
                      Wait(200);
                      aimed_angle = (pos_orientation_angle + TURN_ANGLE) % 360;
                      break;
                 case OBSTACLE_BALL:
                      state = ST_ADJUST_POSITION;
                      break;
             }
             prev_obst = OBSTACLE_NONE;
         }
         if(t + LOOK_AROUND_DELAY <= CurrentTick())
         {
             look_around();
             t = CurrentTick();
         }
    
         break;
      
      #endif


		case ST_ADJUST_POSITION :
      motors_speed = 10;
			motors_state = MOTORS_STRAIGHT;
			if (obstacle_distance <= CATCH_MIN_DIST)
			{
				state = ST_CATCH;
        break;
      }


		case ST_CATCH :
			catch_cmd = CATCH_CATCH;
			if(catch_state == CST_CAUGHT_BALL)
			{
				Wait(100);
        if(catch_state == CST_CAUGHT_NO_BALL)
				   state = ST_LOOK_AROUND;
        else
           state = ST_AIM;
        }
			break;


		case ST_AIM:
			while( pos_orientation_angle !=0)
			{
				aimed_angle = 0;
			}
			state = ST_THROW;
			break;


		case ST_LOOK_AROUND : // pas optimale
			motors_speed = 10;
			motors_state = MOTORS_STRAIGHT;
			Wait(2000);
			motors_state = MOTORS_NONE;
			angle1 = pos_orientation_angle - 20;
			angle2 = pos_orientation_angle + 20;
			while(obstacle_type == OBSTACLE_NONE && pos_orientation_angle != angle1 )
			{
				aimed_angle = pos_orientation_angle - 20;
			}
			while(obstacle_type == OBSTACLE_NONE && pos_orientation_angle != angle2)
			{
				aimed_angle = pos_orientation_angle +40;
			}
			if( obstacle_type == OBSTACLE_NONE)
			{
				state = ST_SEARCH_STRATEGY;
			}
			else if(obstacle_type == OBSTACLE_BALL)
			{
				state = ST_ADJUST_POSITION;
			}
			break;

		case ST_THROW :
			motors_speed = 100;
			motors_state = MOTORS_BACK;
			Wait(3000);
			catch_cmd = CATCH_LETGO;
			motors_speed = 100;
			motors_state = MOTORS_STRAIGHT;
			Wait(2000);
			motors_state = MOTORS_NONE;
					state = ST_SEARCH_STRATEGY;
			break;

		default:
			break;
		}


	}
}
