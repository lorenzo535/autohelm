/*******************************************************************

	autohelm_.c
   LBR 2014


	Description
	===========

	I/O control			On proto-board
	--------------		----------------------
	Port F bit 4		S1, switch
	Port B bit 7		S2, switch


*******************************************************************/
#class auto

#define S1  0		//switch, port B bit 1
#define S2  4		//switch, port B bit 4
#define ONOFF 5
#define EXTEND 7
#define RETRACT 4
#define END_STOP_HIGH 4
#define END_STOP_LOW 0
#define FREE_TO_GO 99
#define INIT_PWM_VALUE 100
#define PWM_MAX_VALUE  400

#define PORT_PIN0    1
#define PORT_PIN1    2
#define PORT_PIN2 	4
#define PORT_PIN3 	8
#define PORT_PIN4 	16
#define PORT_PIN5 	32
#define PORT_PIN6 	64
#define PORT_PIN7 	128
unsigned long pwm_freq;
int pwm_options;
int pwm_sweep;
int pwm_retract;
int pwm_extend;
int pwm_value;
int chpwm;
int choff;
int actuator_mode;
int actuator_softbrake;
int stop;
int on_off;
int actuator_mode_old;
int actuator_mode_before_stop;
int current_button_state;
#define ACTUATOR_EXTEND 2
#define ACTUATOR_RETRACT 1
#define ACTUATOR_STOPPED 3

//////////////////////////////////////////
// FUNCTIONS DECLARATIONS
void SoftBrake (int ch1, int ch2);
void actuatorExtend();
void actuatorRetract();
void actuatorGo(int choff , int chpwm, int pwm_value);
void fctMsDelay ( int iDelay );
int risingFront (int old, int new, int bit);
///////////////////////////////////////

void processButtons (int _currentstate)
{
	static int old_state ;

	// Debug
  //printf ("buttons %d%d%d%d%d%d%d%d\n",(current_button_state &0x01 ? 1:0),(_currentstate &0x02 ? 1:0),(_currentstate &0x04 ? 1:0),(_currentstate &0x08 ? 1:0),
  //       												(_currentstate &0x10 ? 1:0),(_currentstate &0x20 ? 1:0),(_currentstate &0x40 ? 1:0),(_currentstate &0x80 ? 1:0));


   if (old_state != _currentstate)
   // Rect on rising fronts
   {
      if (risingFront ( old_state, _currentstate, 0x01))
      	// button PB0 pressed
         printf ("PBO\n ");
      if (risingFront ( old_state, _currentstate, 0x04))
      	// button PB0 pressed
         printf ("PB2\n");
      if (risingFront ( old_state, _currentstate, 0x08))
      	// button PB0 pressed
         printf ("PB3\n");
      if (risingFront ( old_state, _currentstate, 0x10))
      	// button PB0 pressed
        {printf ("PB4\n"); actuator_mode = ACTUATOR_EXTEND;}
      if (risingFront ( old_state, _currentstate, 0x20))
      	// button PB0 pressed
       {  printf ("PB5\n");  on_off = !on_off; }
      if (risingFront ( old_state, _currentstate, 0x80))
      	// button PB0 pressed
        { printf ("PB7\n");  actuator_mode = ACTUATOR_RETRACT; }
   }
   old_state = _currentstate;
}

int risingFront (int old, int new, int bit)
{
	if (!(old & bit)  && (new & bit)  )
   	return 1;
      else
      return 0;
}

void SoftBrake (int ch1, int ch2)
{
  	actuatorGo(ch1,ch2, actuator_softbrake);
      if (actuator_softbrake > 0)
   	{
   	   actuator_softbrake = actuator_softbrake - pwm_sweep;
         fctMsDelay(5);
   	}
   else
   	actuator_softbrake = 0;
}

void actuatorExtend()
{

   pwm_retract = INIT_PWM_VALUE;
   actuator_mode_before_stop = ACTUATOR_EXTEND;
   if (actuator_softbrake != 0)
	{
 		SoftBrake(0,2);
      return;
   }
	//Prevent further extension if at high end stop
	if (stop == END_STOP_HIGH)
   	pwm_extend = 0;

   actuatorGo(2,0, pwm_extend);
   if (pwm_extend < PWM_MAX_VALUE)
   {
   	   pwm_extend = pwm_extend + pwm_sweep;
         fctMsDelay(5);
   }
   else
   	pwm_extend = PWM_MAX_VALUE;

}

void actuatorRetract()
{

   pwm_extend = INIT_PWM_VALUE;
      actuator_mode_before_stop = ACTUATOR_RETRACT;
   if (actuator_softbrake != 0)
	{
 		SoftBrake(2,0);
      return;
   }

   //Prevent further Retraction if at low end stop
	if (stop == END_STOP_LOW)
   	pwm_retract = 0;

	actuatorGo(0,2, pwm_retract);
   if (pwm_retract < PWM_MAX_VALUE)
   {
   	   pwm_retract = pwm_retract + pwm_sweep;
         fctMsDelay(5);
   }
   else
   	pwm_retract = PWM_MAX_VALUE;
}

void actuatorGo(int choff , int chpwm, int pwm_value)
{
    //set input i on low
    pwm_set(choff, 0, pwm_options);

      //set other pin pwm
    pwm_set(chpwm, pwm_value, pwm_options);

}


void actuatorStop()
{
		actuator_mode=ACTUATOR_STOPPED;
      pwm_set(0, 0, pwm_options);
      pwm_set(2, 0, pwm_options);
      //reset initial values
      pwm_retract = INIT_PWM_VALUE;
      pwm_extend = INIT_PWM_VALUE;

}

/*********************************************************************
fctMsDelay - delay some number of milliseconds
	input parameter: int number of milliseconds to delay
	return value: none
	errors: none
*/
void fctMsDelay ( int iDelay )
{	unsigned long ul0;
	ul0 = MS_TIMER;				// get current timer value
	while ( MS_TIMER < ul0 + (unsigned long) iDelay );
}


main()
{

   pwm_sweep= 100;
   pwm_value = 0;
   chpwm =0;
   choff =2;
   actuator_softbrake = 0;
   actuator_mode_old = ACTUATOR_STOPPED;
   actuator_mode = ACTUATOR_STOPPED;
   actuator_mode_before_stop = ACTUATOR_EXTEND;
   pwm_retract = INIT_PWM_VALUE;
	pwm_extend = INIT_PWM_VALUE;
   stop = FREE_TO_GO;
   current_button_state = 0;

   //initialisation PWM
   //frequence = 10kHz
   pwm_freq = pwm_init(100ul);
	printf("Actual PWM frequency = %lu Hz\n", pwm_freq);
   pwm_options = PWM_SPREAD;
   pwm_set(0, 0 ,pwm_options);	//channel 0 is PF4
   pwm_set(1, 0 ,pwm_options);   // channel 1 is PF5
   //Set PF4 as PWM channel 0
   BitWrPortI(PFCR, &PFCRShadow,4, 1);
   //Set PF6 as PWM channel 2
   BitWrPortI(PFCR, &PFCRShadow,6, 1);

  //initialize ports
  WrPortI ( PFDDR, NULL, 0xFF );   // make port F output
  WrPortI ( PBDDR, NULL, 0x00 );   // make port B input
  WrPortI ( PEDDR, NULL, 0x00 );   // make port E input

  WrPortI(PFDR, NULL, 0x00);

	on_off=0;


	while (1)
	{
      // main loop
      costate
		{
             //DEBUG print
             if(actuator_mode_old != actuator_mode)
             {
	          if (actuator_mode == ACTUATOR_STOPPED) printf (" **ACTUATOR_STOPPED** ");
	          if (actuator_mode == ACTUATOR_RETRACT) printf (" --ACTUATOR_RETRACT-- ");
	          if (actuator_mode == ACTUATOR_EXTEND) printf (" ++ACTUATOR_EXTEND++ ");
             }
//       printf (" retract %d extend %d softbrake %d \n", pwm_retract, pwm_extend, actuator_softbrake);
       if( (actuator_mode_old != actuator_mode)&&
       	(actuator_mode_old!=ACTUATOR_STOPPED)&&
       	(actuator_mode!=ACTUATOR_STOPPED))
         {
				if (actuator_mode_old == ACTUATOR_RETRACT)
               actuator_softbrake = pwm_retract;
            if (actuator_mode_old == ACTUATOR_EXTEND)
               actuator_softbrake = pwm_extend;
         }


       actuator_mode_old = actuator_mode;
       if (on_off)
       {
       	if (actuator_mode == ACTUATOR_RETRACT)
	         actuatorRetract();
	       else if (actuator_mode == ACTUATOR_EXTEND)
	       	actuatorExtend();
          else
              actuator_mode = actuator_mode_before_stop;
       }
       else
          	actuatorStop();
      }

   	costate
      {
      	current_button_state = RdPortI(PBDR);
         processButtons(current_button_state);

      }



       costate
		{
         //END_STOP_HIGH
         if (BitRdPortI(PEDR, END_STOP_HIGH))		//wait for switch S1 press
				stop = END_STOP_HIGH;
         //END_STOP_LOW
         else if (BitRdPortI(PEDR, END_STOP_LOW))		//wait for switch S1 press
				stop = END_STOP_LOW;
			else
	      	stop = FREE_TO_GO;
         fctMsDelay(20);
         if (stop == END_STOP_HIGH) printf ("stop is END_STOP_HIGH \n" );
         if (stop == END_STOP_LOW) printf ("stop is END_STOP_LOW \n" );

      }





	}
}