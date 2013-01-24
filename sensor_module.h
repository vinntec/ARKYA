// enlever commentaire ligne 47  58

//Test and change this value in fonction of the environement
#define NEAR   18 //cm
#define cbb1   140  //sup color bleu balle
#define cbb2   60 // inf color bleu balle
#define SUP_LIM_BLACK   330 // sup color black line

// the value of obstacle_distance
#define OBSTACLE_DISTANCE_LONG  24
#define OBSTACLE_DISTANCE_MIN   6
#define COLOR_BLEU_BALL_CAPTURE 89
 int CapteurUs = 0;
 int CapteurCouleurBall = 0;
 int CapteurCouleurLine = 0;


task sensor_module(){

 SetSensorType(IN_3, SENSOR_TYPE_COLORRED);
 SetSensorLowspeed(IN_4);
 SetSensorType(IN_1, SENSOR_TYPE_COLORRED);
 
 while(1){
  obstacle_distance = SensorUS(S4);


     CapteurUs = (!(catch_state == CST_CAUGHT_BALL || catch_state == CST_NOT_CAUGHT_BALL)) && (SensorUS(IN_4) < NEAR);
     CapteurCouleurLine = (pos_region != POS_REGION_HOME) && ((SENSOR_1 <= SUP_LIM_BLACK) || (SENSOR_3 <= SUP_LIM_BLACK)) ;
     
    // Black line detected
   if(CapteurCouleurLine ==1)
   {
    obstacle_type = 2;
   }
    // wall detected
   else if( (CapteurCouleurLine ==0) && (CapteurUs == 1))
   {
    obstacle_type = 1;
   }
    // bleu ball detected
   /*else if( (CapteurCouleurLine ==0) && (CapteurCouleurBall == 1) )
   {
    obstacle_type = 3;
   }*/
   else
       obstacle_type = OBSTACLE_NONE;
 }
}

