//Positioning module variables -------------------------------------------------

#define US_SENSOR_OFFSET 11 //In cm


//Enums
#define POS_REGION_HOME 1
#define POS_REGION_BORDER 2

#define POS_REGION_LIMIT_Y 30

#define POS_STATUS_OK 0
#define POS_STATUS_UNSTABLE 1

#define POS_MAX_Y 51
#define POS_MAX_X 61

//Modes
#define POS_ORIENT_SIMPLE_MODE

//INITIAL POSITION DO NOT TOUCH
#define POS_INIT_KNOWN_X 30
#define POS_INIT_KNOWN_Y 6

//#define POS_INIT_KNOWN

#define POS_INIT_KNOWN_X_1 9
#define POS_INIT_KNOWN_Y_1 52

#define POS_INIT_KNOWN_X_2 42
#define POS_INIT_KNOWN_Y_2 52

//Refresh times in milliseconds
//Note: 10 seems to be minimum!
#define POS_ORIENT_RDELAY 25

//Output variables
int pos_orientation_angle = 0;
int pos_orientation_status = POS_STATUS_OK;

int pos_x = 0;
int pos_y = 0;

int pos_region = POS_REGION_HOME;

//Vision module variables ------------------------------------------------------

#define NONE 0
#define WALL 1
#define LINE 2
#define BALL 3

#define VISION_USUAL_OBST_DIST 10

int distance_from_obstacle = VISION_USUAL_OBST_DIST;
int vision_obstacle = NONE;

//Calibration points
#define P1_X 60
#define P1_Y 12
#define P1_A 0

#define P2_X 17
#define P2_Y 28
#define P2_A -17

#define P3_X 104
#define P3_Y 66
#define P3_A 21

#define P4_X 60
#define P4_Y 81
#define P4_A 9

#define P5_X 65
#define P5_Y 56
#define P5_A 28

//Strategy module variables ----------------------------------------------------

//For awaiting modules' initiation
//Position module requests 2 of these
int init_counter = 0;

#define INIT_COUNT_EXPECTED 3


//Aimed angle precision
#define ANGLE_PRECISION 2

//Destination variables
int dest_x = 0;
int dest_y = 0;


// RANDOM SEARCH ALGO
#define TURN_ANGLE 125
#define LOOK_AROUND_OFFSET 60
#define FOUND_BALL_OFFSET 5

// Known positions
#define KP_SQ_PC 0.95 //Percentage of square distance before checking

//Motors module variables ------------------------------------------------------

//Status
#define MOTORS_NONE 0
#define MOTORS_STRAIGHT 4
#define MOTORS_LEFT 3
#define MOTORS_RIGHT 2
#define MOTORS_BACK 1

int motors_state = MOTORS_NONE;
int motors_speed = 0;
int real_motors_state = 0;



//Wheels
int distance = 0;

//Destination
int aimed_angle = 0;

//Default speeds
#define TURN_SPEED 70
#define TURN_SPEED_SLOW 30

#define MOTORS_SPEED_VERY_SLOW 35
#define MOTORS_SPEED_SLOW 60
#define MOTORS_SPEED_MEDIUM 80
#define MOTORS_SPEED_FAST 100

#define REAL_MOTORS_SPEED_S 11.5
#define REAL_MOTORS_SPEED_M 15.5
#define REAL_MOTORS_SPEED_F 20.5

//Orientation task
#define ADVANCED_ORIENTATION



//View module ------------------------------------------------------------------

//OBSTACLE TYPES
#define OBSTACLE_NONE 0
#define OBSTACLE_WALL 1
#define OBSTACLE_LINE 2
#define OBSTACLE_BALL 3

#define AIMED_OBST_DIST 10

// the value of obstacle_distance
#define OBSTACLE_DISTANCE_LONG 24
#define OBSTACLE_DISTANCE_MIN  6

int obstacle_type = OBSTACLE_NONE;
int obstacle_distance = 0;

//Catch module -----------------------------------------------------------------

//Envt
#define ARM_MOTOR OUT_B
#define CATCH_ARM_SPEED 40

//commandes for sub do_catch_action(int catch_cmd)
#define CATCH_NONE 0
#define CATCH_CATCH 1
#define CATCH_LETGO 2

//state of catch_state_machine
#define CST_UP 0
#define CST_CATCHING 1 //To be removed
#define CST_CAUGHT_BALL 2
#define CST_NOT_CAUGHT_BALL 3
#define CST_RELEASING 4 //To be removed
#define CST_LOW 18 //To be removed
#define CST_ON_WALL 56


//States and commands
int catch_cmd = CATCH_NONE;
int real_catch_cmd = CATCH_NONE;
int catch_state = CST_UP;

