/**
 * Header functions for strategy.
 */

//States
//More detailed ones have to be created
#define ST_INIT 0
#define ST_GO_TO_INIT_POINT 1
#define ST_FOLLOW_DETERMINED_PATH 2
#define ST_LOOK_AROUND 3
#define ST_ADJUST_POSITION 4
#define ST_CATCH 5
#define ST_AIM 6
#define ST_THROW 7
#define ST_RANDOM_SEARCH 8
#define ST_RS_RC 19
#define ST_KNOWN_POSITIONS 56

//Misc constants
//Look-around and random catch.
#define LA_PERIOD 10000
#define RC_PERIOD 500
#define RC_MIN_DISTANCE 25
#define RSA_TURN_ANGLE 110

//Global variables ------------------------------------
int state = ST_INIT;
int vtab[100], vtab_offset = 0;

/**
 * Obstacle checker: raises flag if obstacle found.
 */
//Global
int prev_obst = OBSTACLE_NONE;

task obstacle_checker()
{
    int i = 0;

    while(1)
    {
        until((i = obstacle_type) != OBSTACLE_NONE);
        prev_obst = i;
    }
}

int pa;
int la;
int ua;
int window;
int ball_distance;
int found_ball;
int ball_angle = 0;

#define SWEEP_WEIGHT 3

int compute_average(int tab[])
{
	int i, sum = 0;
	for(i = 0; i < SWEEP_WEIGHT; i++)
		sum += tab[i];
	return sum/SWEEP_WEIGHT;
}

void sweep(long init_time)
{
	int tab[SWEEP_WEIGHT];
	int i, sw_index = 0;
	int cd, pd;
	int fba = 0;
	int useful_values = 0;

	for(i = 0; i < SWEEP_WEIGHT; i++)
		tab[i] = obstacle_distance;
	pd = obstacle_distance;

	Wait(200);

	while(CurrentTick() < init_time + 4000 && found_ball == 0)
	{
		cd = SensorUS(S4);
		if(cd <= 200)
		{
			tab[sw_index] = cd;
			sw_index = (sw_index+1)%SWEEP_WEIGHT;

			//Compute current value
			cd = compute_average(tab);

			if(window && pd/(1.0*cd) > 0.8 && pd/(1.0*cd) < 1.2)
				useful_values ++;

			if(window && (cd - pd >= FOUND_BALL_OFFSET)) {
				if(useful_values >= 5 && useful_values < 15) {
					found_ball = 1;
					ball_angle = pos_orientation_angle;
					ball_angle = (fba + ((ball_angle - fba + 360 ) % 360)/2 + 356) % 360;
					window = 0;
					break;
				}
				else
				{
					window = 0;
					found_ball = 0;
					useful_values = 0;
				}
			}
			else if (pd - cd >= FOUND_BALL_OFFSET) {
				window = 1;
				ball_distance = cd;
				fba = pos_orientation_angle;
			}

			pd = cd;

			//TODO to delete
			vtab[vtab_offset] = cd;
			vtab_offset = (vtab_offset + 1) % 100;
		}
	}
}


/**
 * Part of the random search algorithm.
 * Look around after random moving.
 * 
 * Principle: detect a discontinuity in "obstacle_distance" which would correspond to a ball.
 */
sub look_around()
{
    motors_state = MOTORS_NONE;
    pa = pos_orientation_angle;
    int cd = 0;
    turn_speed = 30;
    window = 0;
    found_ball = 0;
    
    aimed_angle = la = (aimed_angle - LOOK_AROUND_OFFSET + 360) % 360;
    Wait(1500);

    //Now we go twice as far, and check whether there is an obstacle.
    int pd = obstacle_distance;
    long init_time = CurrentTick();
    turn_speed = TURN_SPEED_SLOW;
    
    ua = (aimed_angle + 2*LOOK_AROUND_OFFSET + 360) % 360;
    aimed_angle = ua;
    
    

    sweep(init_time);

    if(found_ball)
    {
        aimed_angle = ball_angle;
        state = ST_ADJUST_POSITION;

		vtab[vtab_offset] = cd;
		vtab_offset = (vtab_offset + 1) % 100;
    }

    else
        aimed_angle = pa;
        
    Wait(2000);
    turn_speed = TURN_SPEED;
}

/**
 * Catch, and see if anything was caught, in a random manner
 */
sub random_catch()
{
	motors_speed = MOTORS_SPEED_VERY_SLOW;

	catch_cmd = CATCH_CATCH;

	long init_time = CurrentTick();
	while(CurrentTick() <= init_time + 1500 && prev_obst != OBSTACLE_LINE);

	if(catch_state == CST_CAUGHT_BALL)
	{
		state = ST_AIM;
	}
	else
	{
		catch_cmd = CATCH_LETGO;
		init_time = CurrentTick();
		while(CurrentTick() <= init_time + 1000 && prev_obst != OBSTACLE_LINE);
	}

	if(prev_obst == OBSTACLE_WALL)
		prev_obst = OBSTACLE_NONE;
}

/**
 * This is supposed to make the robot go get a ball at a known position,
 * specified by coordinates (dest_x, dest_y)
 */
sub catch_known_pos_ball()
{
	#define SQUARE(x) x*x

	//1. Set destination angle
	aimed_angle = destination_angle();

	Wait(1000);

	//2. Get distance
	int d = square_distance_to_destination();

	//3. Go to partial distance
	int pd = KP_SQ_PC*d;
	const int last_distance = distance;

	motors_speed = MOTORS_SPEED_SLOW;
	motors_state = MOTORS_STRAIGHT;
	while(SQUARE(last_distance-distance) <= pd);

	motors_state = MOTORS_NONE;

	look_around();

	if(found_ball)
		state = ST_ADJUST_POSITION;
	else
		state = ST_INIT;
}

/**
 * When we cross the black line
 */
sub go_back_to_field()
{
	motors_state = MOTORS_NONE;

	aimed_angle = 180;
	Wait(1500);

	motors_speed = 70;
	motors_state = MOTORS_STRAIGHT;

	until(prev_obst == OBSTACLE_WALL);

	state = ST_RANDOM_SEARCH;
	motors_state = MOTORS_NONE;
}

int is_actually_wall = 0;

/**
 * When sensors see a "wall", it might not be one.
 * This, on a higher level, checks whether it really is one.
 * Only call when prev_obst is WALL, else fails.
 */
sub analyse_wall_obstacle()
{
	motors_state = MOTORS_NONE;
	int iterations = 0;

	while(iterations < 5)
	{
		if(prev_obst != OBSTACLE_WALL)
			return;
		prev_obst = OBSTACLE_NONE;

		//Go forward "a little"
		motors_speed = MOTORS_SPEED_VERY_SLOW;
		Wait(50);

		//Stop and grab and wait
		motors_state = MOTORS_NONE;
		catch_cmd = CATCH_CATCH;
		Wait(1000);

		//Read arm position
		switch(catch_state)
		{
		case CST_CAUGHT_BALL:
			state = ST_AIM;
			return;
		case CST_ON_WALL:
			is_actually_wall = 1;
			return;
		case CST_NOT_CAUGHT_BALL:
			catch_cmd = CATCH_LETGO;
			Wait(1000);
			iterations ++;
		}
	}
}
