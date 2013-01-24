

task catch_task()
{
	 while(1) {
		 switch(real_catch_cmd)
		 {
		 case CATCH_CATCH:
			 OnFwd(ARM_MOTOR, CATCH_ARM_SPEED);
			 break;
		 case CATCH_LETGO:
			 OnRev(ARM_MOTOR, CATCH_ARM_SPEED);
			 break;
		 default:
			 Off(ARM_MOTOR);
		 }

		 Wait(100);
	 }
}

task catch_state_machine() {
	int ini_deg = MotorRotationCount(ARM_MOTOR);
	int current_rel_deg;

	while(1)
	{
		//Compute current state
		current_rel_deg = MotorRotationCount(ARM_MOTOR) - ini_deg;

		if(current_rel_deg <= 0)
			catch_state = CST_UP;
		else if(current_rel_deg > 0 && current_rel_deg <= 130)
			catch_state = CST_ON_WALL;
		else if(current_rel_deg > 130 && current_rel_deg <= 180)
			catch_state = CST_CAUGHT_BALL;
		else
			catch_state = CST_NOT_CAUGHT_BALL;


		//Analyse command
		switch(catch_cmd)
		{
		case CATCH_CATCH:
			if(catch_state != CST_NOT_CAUGHT_BALL)
				real_catch_cmd = CATCH_CATCH;
			else
				real_catch_cmd = CATCH_NONE;
			break;
		case CATCH_LETGO:
			if(catch_state != CST_UP)
				real_catch_cmd = CATCH_LETGO;
			else
				real_catch_cmd = CATCH_NONE;
			break;
		default:

		}
	}
}
