/**
 * This file defines the strategy module.
 * See state diagram for more information.
 */

#include "strategy_tools.h"

/**
 * Main strategy task.
 */
task strategy() {
	//Local variables
	long la_timer = 0; //"Look-around" timer
	long rc_timer = 0; //"Random-catch" timer
	int old_distance = 0;
	int kps_found = 0;

	Wait(1000);

	while(1) {
		while(pos_y >= 49)
			go_back_to_field();

		switch(state) {
		case ST_INIT:
			//Known positions
			switch(kps_found)
			{
#ifdef POS_INIT_KNOWN
			case 0:
				dest_x = POS_INIT_KNOWN_X_1;
				dest_y = POS_INIT_KNOWN_Y_1;

				state = ST_KNOWN_POSITIONS;
				break;
			case 1:
				dest_x = POS_INIT_KNOWN_X_2;
				dest_y = POS_INIT_KNOWN_Y_2;

				state = ST_KNOWN_POSITIONS;
				break;
#endif
			default:
				state = ST_RANDOM_SEARCH;
				la_timer = CurrentTick();
				rc_timer = CurrentTick();
			}
			break;


		case ST_KNOWN_POSITIONS:
			catch_known_pos_ball();
			kps_found++;
			break;

		case ST_RANDOM_SEARCH:
#ifdef POS_INIT_KNOWN
			if(kps_found != 2)
			{
				state = ST_INIT;
				break;
			}
#endif

			// Go slowly forward
			motors_state = MOTORS_STRAIGHT;
			motors_speed = MOTORS_SPEED_SLOW;
			catch_cmd = CATCH_LETGO;

			// Obstacles
			switch(prev_obst)
			{
			case OBSTACLE_LINE:
				if(pos_region == POS_REGION_HOME) break;
				motors_state = MOTORS_BACK;
				Wait(600);
				aimed_angle = (pos_orientation_angle + RSA_TURN_ANGLE) % 360;
				prev_obst = OBSTACLE_NONE;
				break;
			case OBSTACLE_WALL:
				motors_state = MOTORS_BACK;
				Wait(600);

				look_around();
				la_timer = CurrentTick();
				if(found_ball)
				{
					state = ST_ADJUST_POSITION;
					ball_distance = NEAR+1;
					break;
				}

				motors_state = MOTORS_BACK;
				Wait(300);
				aimed_angle = (pos_orientation_angle + RSA_TURN_ANGLE) % 360;
				prev_obst = OBSTACLE_NONE;
				break;
			default:
			}

			// Look around? TODO
			/*if(!found_ball && la_timer + LA_PERIOD <= CurrentTick())
			{
				look_around();
				la_timer = CurrentTick();
			}*/

			/*if(!found_ball && rc_timer + RC_PERIOD <= CurrentTick())
			{
				if(obstacle_distance >= RC_MIN_DISTANCE)
					random_catch();
				rc_timer = CurrentTick();
			}*/

			break;


		case ST_ADJUST_POSITION :
			motors_state = MOTORS_STRAIGHT;
			motors_speed = MOTORS_SPEED_VERY_SLOW;
			old_distance = distance;
			until((distance - old_distance) >= ball_distance/1.5 || prev_obst == OBSTACLE_LINE);
			if(prev_obst == OBSTACLE_LINE)
			{
				state = ST_RANDOM_SEARCH;
				found_ball = 0;
			}
			else
				state = ST_CATCH;
			break;



		case ST_CATCH :
			catch_cmd = CATCH_CATCH;
			motors_state = MOTORS_NONE;

			Wait(2000);

			if(catch_state == CST_CAUGHT_BALL)
				state = ST_AIM;
			else
			{
				state = ST_RANDOM_SEARCH;
				catch_state = CATCH_LETGO;
				found_ball = 0;
			}
			break;


		case ST_AIM:
			motors_state = MOTORS_NONE;
			aimed_angle = 0;
			Wait(2000);
			state = ST_THROW;
			break;

		case ST_THROW :
			motors_speed = 100;
			motors_state = MOTORS_BACK;
			Wait(700);
			catch_cmd = CATCH_LETGO;
			motors_speed = 100;
			motors_state = MOTORS_STRAIGHT;
			Wait(660);
			motors_state = MOTORS_NONE;
			state = ST_RANDOM_SEARCH;
			found_ball = 0;
			break;

		default:
			break;

		}
	}


}
