// Port A : Left motor
// Port B : Additional motor
// Port C : Right motor

float motors_degrees_count = 0;
int gap = 0;
int real_gap = 0;

//Wheelturns
int precedent_state = MOTORS_NONE;
int precedent_speed = 0;
long tick1, tick2;
int pdistance = 0;
int in_session = 0;

int turn_speed = TURN_SPEED;

task do_action()
{
  while(1)
  {

  while(real_motors_state == 0)		// cut the motors
	{
		Off(OUT_AC);
		
	}
	while(real_motors_state == 1)		// go forward in a synchronized mode
  {
	    	OnFwdSync(OUT_AC,motors_speed, 0);
	}
	while(real_motors_state == 2)		// turn to the right in a synchronized mode
	{
		OnFwd(OUT_A,turn_speed);
		OnRev(OUT_C,turn_speed);
	}
	while(real_motors_state == 3)		// turn to the left
	{
        	OnFwd(OUT_C,turn_speed);
		OnRev(OUT_A,turn_speed);
	}
	while(real_motors_state == 4)		// go backward in a synchronized mode at max speed
	{
		OnRevSync(OUT_AC,motors_speed, 0);
	}
	
  }
}

sub calculate_session()
{
   //Compute precedent
   tick2 = CurrentTick();

   long dt = tick2 - tick1;
   float real_speed = 0;

   //Distance computation
   switch(precedent_speed)
   {
   case MOTORS_SPEED_VERY_SLOW:
       real_speed = REAL_MOTORS_SPEED_S/1.9;
	   break;

   case MOTORS_SPEED_SLOW:
       real_speed = REAL_MOTORS_SPEED_S;
       break;
   case MOTORS_SPEED_MEDIUM:
       real_speed = REAL_MOTORS_SPEED_M;
       break;
   case MOTORS_SPEED_FAST:
       real_speed = REAL_MOTORS_SPEED_F;
       break;
   }
   
   distance = pdistance + real_speed*dt/1000;
}

sub terminate_session()
{
    in_session = 0;
}

sub new_session()
{
   //And start a new one.
   
   in_session = 1;
   
   tick1 = CurrentTick();
   
   pdistance = distance;
   precedent_state = real_motors_state;
   precedent_speed = motors_speed;

}

task wheelturns()
{
  #define CHG_STATE (real_motors_state != precedent_state)
  #define CHG_SPEED (motors_speed != precedent_speed)
  #define ACTIVATE (real_motors_state == MOTORS_STRAIGHT || real_motors_state == MOTORS_BACK)

  while(1)
  {
       if(in_session)
           calculate_session();
           
           

       /*if((ACTIVATE && (CHG_SPEED || CHG_STATE)))
       {
           new_session();
       }

       else if(CHG_STATE && !ACTIVATE)
       {
           terminate_session();
       }  */
       
       //--------------------------------------------------------
       
       if(in_session)
       {
           calculate_session();
           
           if(!ACTIVATE)
               terminate_session();
           else if(CHG_SPEED || CHG_STATE)
               new_session();
       }
       else
           if(ACTIVATE)
               new_session();
               
       Wait(10);
  }
}

inline int min(int a, int b)
{
	if (a > b)
	return(b);
	return(a);
}

/**
 * Old orientation task.
 * This just turns around whenever aimed_angle is not respected by compass.
 * The problem is that is does *not* provide orientation stability for the robot.
 * See new orientation task for more information.
 */
#ifndef ADVANCED_ORIENTATION
task orientation_task()
{
	#define a pos_orientation_angle
	#define b aimed_angle
	
	int nd, sg;
	
	while (1) {
		gap = min(abs(b - a), 360 - abs(b - a));

		if (gap > ANGLE_PRECISION) {
			nd = ((abs(b - a) <= 180) * 2) - 1;
			sg = (((b - a) > 0) * 2) - 1;
			real_motors_state = ((nd * sg) > 0) + 2;
		}
		if (gap <= ANGLE_PRECISION) {
			real_motors_state = motors_state;
		}
	}

}

#else
/**
 * This task aims to solve the problem of the previous one, by
 * introducing a sessions system: whenever the aimed angle changes,
 * change the current angle. But if thereafter the current angle is not
 * precise, nothing will change
 */
task orientation_task()
{
	#define a pos_orientation_angle
	#define b aimed_angle

	int nd, sg;
	int ortask_session = 0; //By default, not in session.
	int ortask_prev_angle = a;

	while(1)
	{
		if(ortask_session)
		{
			// If we're in session, rotate the motors towards the right direction.
			gap = min(abs(b - a), 360-abs(b - a));

			if(gap > ANGLE_PRECISION)
			{
				//If the gap is too big, rotate.
				nd = ((abs(b-a)<=180)*2)-1;
				sg = (((b-a) > 0) *2) -1;
				real_motors_state = ((nd * sg) > 0) +2;
			}
			else
			{
				//Else, cancel session.
				ortask_session = 0;
				ortask_prev_angle = aimed_angle;
			}
		}
		else
		{

			//But keep checking session info
			if(ortask_prev_angle != aimed_angle)
				ortask_session = 1;

			//If we're not in session, transmit do_action,
			else
				real_motors_state = motors_state;

		}
	}

}



#endif

