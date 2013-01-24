

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
		else if(current_rel_deg > 0 && current_rel_deg <= 150)
		{
			if(real_catch_cmd == CATCH_CATCH)
				catch_state = CST_CATCHING;
			else
				catch_state = CST_RELEASING;
		}
		else
			catch_state = CST_LOW;


		//Analyse command
		switch(catch_cmd)
		{
		case CATCH_CATCH:
			if(catch_state != CST_LOW)
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
