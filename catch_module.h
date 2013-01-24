#define Arm_Motor OUT_B
#define Consecutive_Measures 3
#define Touch_Port IN_1   

#define CATCH_SIMPLE_MODE
#define CATCH_ANGLE 70

//commandes for sub do_catch_action(int catch_cmd)
#define CATCH_NONE 0
#define CATCH_CATCH 1
#define CATCH_LETGO 2

//state of catch_state_machine
#define ST_IDLE 0
#define ST_CATCHING 1
#define ST_CAUGHT_BALL 2
#define ST_CAUGHT_NO_BALL 3
#define ST_RELEASING 4

int catch_state = ST_IDLE;	// global variable to store the catch status
int catch_cmd = 0; 		// global variable to determine the catch action	
int prev_catch_cmd = 0;
bool ballDetected = false;

/* double check whether the ball is really caught*/
/* to be completed */
sub checkBall()
{

}

task do_catch_action()
{
    while (1){ 
        if (catch_cmd != prev_catch_cmd){
            switch(catch_cmd)
            {
                case CATCH_NONE:
                    Off(Arm_Motor);
                    Wait(100);
                case CATCH_CATCH:
                    RotateMotor(Arm_Motor, 50, CATCH_ANGLE);  
                    Wait(100);
                case CATCH_LETGO:
                    RotateMotor(Arm_Motor, 50, -CATCH_ANGLE);
                    Wait(100);
            }
            prev_catch_cmd = catch_cmd;
        }
    }
}

task catch_state_machine()
{
    long ini_deg = MotorRotationCount(Arm_Motor);
    long current_deg = ini_deg;
    long relative_deg;
    while(1)
    {
        // Periodically read the position of the arm motor
        Wait(250);
        current_deg = MotorRotationCount(Arm_Motor);
        relative_deg = current_deg - ini_deg;

        if ((catch_state == ST_IDLE ) && (catch_cmd == CATCH_CATCH)) {
            catch_state = ST_CATCHING;
        }
        if ((catch_state == ST_CATCHING) && ( SENSOR_1 == 1)){
            #ifdef CATCH_SIMPLE_MODE
                catch_state = ST_CAUGHT_BALL;
            #else
                checkBall();
                    if ( ballDetected ) {
                    catch_state = ST_CAUGHT_BALL;
                }
                else {
                    catch_state = ST_CAUGHT_NO_BALL;
                }
            #endif
        }
        if ((catch_state == ST_CATCHING) && ( relative_deg > 80) && ( SENSOR_1 != 1)) {
            catch_state = ST_CAUGHT_NO_BALL;
        }
        if (((catch_state == ST_CAUGHT_BALL) || (catch_state == ST_CAUGHT_NO_BALL)) && (catch_cmd == CATCH_LETGO)) {
            catch_state = ST_RELEASING;
        }
        if ((catch_state == ST_RELEASING) && ( relative_deg < 5)) {
            catch_state = ST_IDLE;
        }
    }
}

task test_catch()
{
    while(1)
    {
        catch_cmd = CATCH_CATCH;
        Wait(1000);
        catch_cmd = CATCH_LETGO;
        Wait(1000);
        if (catch_state == ST_CAUGHT_BALL){
            StopAllTasks();
        }
    }
}

task main()
{
    Precedes(do_catch_action,catch_state_machine,test_catch);
}


