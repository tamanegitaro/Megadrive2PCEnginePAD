
//Definition for MEGADRIVE pad buttons
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_A 4
#define BUTTON_B 5
#define BUTTON_C 6
#define BUTTON_X 7
#define BUTTON_Y 8
#define BUTTON_Z 9
#define BUTTON_S 10
#define BUTTON_M 11
#define BUTTON_SPECIAL 12

//Definition for MEGADRIVE pad pins
#define PIN_D0 ((PIND & 0x01) >> 0) //PD0
#define PIN_D1 ((PIND & 0x10) >> 4) //PD4
#define PIN_D2 ((PINC & 0x40) >> 6) //PC6
#define PIN_D3 ((PIND & 0x80) >> 7) //PD7
#define PIN_D4 ((PINE & 0x40) >> 6) //PE6
#define PIN_D5 ((PINB & 0x10) >> 4)  //PB4
#define PIN_SEL_LOW PORTB &= 0xDF
#define PIN_SEL_HI PORTB |= 0x20

//MEGADRIVE pad detection
#define PAD_3B 0
#define PAD_6B 1


#define BUTTON_WAIT_WINDOW 3 //3 * 33ms = 100ms
#define BUTTON_REPEAT_RATE 1 //30ps
#define BUTTON_RELEASED 1
#define BUTTON_PRESSED 0
#define REPEAT_OFF 0
#define REPEAT_ON 1

#define BUTTON_INTERNAL_X 0
#define BUTTON_INTERNAL_Y 1
#define BUTTON_INTERNAL_Z 2


#define RUN 0
#define SELECT 1

typedef struct {
  byte button_internal; //Current button internal status
  byte button_wait; //Counter for wait window after button is pressed set by BUTTON_WAIT_WINDOW
  byte repeat_mode; //Repeat mode status
  byte repeat_counter; //repeat frequency counter set by BUTTON_REPEAT_RATE
  byte button_next; //Next output when repeat mode is enabled
}button_repeat_status;

//Apply select or run
typedef struct {
  byte button_internal; //Current button internal status
  byte button_wait; //Counter for wait window after button is pressed set by BUTTON_WAIT_WINDOW
  byte current_mode; //Current mode status
}button_select_status;

byte button_arrow;
byte button_ab;
byte button_low;
byte button_34;

void setup() 
{
  //Disable timer0 interrupt
  TIMSK0 = 0;
  
  //Configure MD pins
  pinMode(3, INPUT_PULLUP); //D0 //PD0
  pinMode(4, INPUT_PULLUP); //D1 //PD4
  pinMode(5, INPUT_PULLUP); //D2 //PC6
  pinMode(6, INPUT_PULLUP); //D3 PD7
  pinMode(7, INPUT_PULLUP); //D4 PE6
  pinMode(8, INPUT_PULLUP); //D5 PB4
  digitalWrite(9, LOW); //Set SELECT pin as low
  pinMode(9, OUTPUT); //SELECT PB5

  //Configure PCE pins
  pinMode(A0, OUTPUT); //D0 PF4
  pinMode(A1, OUTPUT); //D1 PF5
  pinMode(A2, OUTPUT); //D2 PF6
  pinMode(A3, OUTPUT); //D3 PF7
  pinMode(15, INPUT); //S  PB1
  pinMode(14, INPUT); //C  PB3      
  // initialize Timer1 to use it as free running counter
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
//  OCR1A = 500; // compare match register 16MHz/256/125Hz 8ms interval
//  OCR1A = 250; // compare match register 16MHz/256/250Hz 4ms interval
//  TCCR1B |= (1 << WGM12); // CTC mode
  //Normal mode
  //TCCR1B |= (1 << CS12); // 256 prescaler
  TCCR1B |= ((1 << CS11) | (1 << CS10)); // 64 prescaler
//  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts(); // enable all interrupts
}



void get_pad(byte* button) //Get Megadrive pad
{
  //Get Start and A button status
  button[BUTTON_S] = PIN_D5;
  button[BUTTON_A] = PIN_D4;
  
  //Set SELECT HIGH Advance counter to 1
  PIN_SEL_HI;
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  button[BUTTON_C] = PIN_D5;
  button[BUTTON_B] = PIN_D4;
  button[BUTTON_RIGHT] = PIN_D3;
  button[BUTTON_LEFT] = PIN_D2;
  button[BUTTON_DOWN] = PIN_D1;
  button[BUTTON_UP] = PIN_D0;

  //Set SELECT LOW Advance counter to 2
  PIN_SEL_LOW;

  __asm__("nop\n\t""nop\n\t");
  
  //Set SELECT HIGH Advance counter to 3
  PIN_SEL_HI;
  __asm__("nop\n\t""nop\n\t");

  //Set SELECT LOW Advance counter to 4
  PIN_SEL_LOW;
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  //Make sure that all 4 D0, D1, D2, D3 bits are LOW
  if((PIN_D0 == 0x0) && (PIN_D1 == 0x0) && (PIN_D2 == 0x0) && (PIN_D3 == 0x0))
  {
    //Continue operation
    //Set SELECT HIGH Advance counter to 5
    PIN_SEL_HI;
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  
    //Get data
    button[BUTTON_M] = PIN_D3;
    button[BUTTON_X] = PIN_D2;
    button[BUTTON_Y] = PIN_D1;
    button[BUTTON_Z] = PIN_D0;    
    //Set SELECT LOW Advance counter to 6
    PIN_SEL_LOW;
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  
    //Get special button if needed
    button[BUTTON_SPECIAL] = PIN_D0;
  }
  else
  {
    //3Button pad is connected
    button[BUTTON_M] = BUTTON_RELEASED;
    button[BUTTON_X] = BUTTON_RELEASED;
    button[BUTTON_Y] = BUTTON_RELEASED;
    button[BUTTON_Z] = BUTTON_RELEASED;    
    button[BUTTON_SPECIAL] = BUTTON_RELEASED;
    
  }
}


void loop(void)
{
  button_low = 0 | (PORTF & 0x0F);
  PORTF = button_low;
  delay(1000);

  //Disable interrupts so that time sensitive task will not be interrupted
  noInterrupts();
  
  //Synchronize
  while((PINB & 0x08) == 0);
  while(PINB & 0x08);
  while((PINB & 0x08) == 0);
  while(PINB & 0x08);
  
  while(1)
  {
    AvenuePad3();
    AvenuePad6();
  }
}


void prepare_pin_3B(byte* button, byte mode) //Prepare button value for 3B pad
{
  if(mode == RUN)
  {
    button_ab = (button[BUTTON_C] << 4) | (button[BUTTON_B] << 5) | (button[BUTTON_M] << 6) | ((button[BUTTON_A] & button[BUTTON_S]) << 7) | (PORTF & 0x0F);
  }
  else
  {
    button_ab = (button[BUTTON_C] << 4) | (button[BUTTON_B] << 5) | ((button[BUTTON_A] & button[BUTTON_M]) << 6) | (button[BUTTON_S] << 7) | (PORTF & 0x0F);    
  }
  button_arrow = (button[BUTTON_UP] << 4) | (button[BUTTON_RIGHT] << 5) | (button[BUTTON_DOWN] << 6) | (button[BUTTON_LEFT] << 7) | (PORTF & 0x0F);
  button_34 = (button[BUTTON_A] << 4) | (button[BUTTON_X] << 5) | (button[BUTTON_Y] << 6) | (button[BUTTON_Z] << 7) | (PORTF & 0x0F);
  button_low = 0 | (PORTF & 0x0F);
}

void prepare_pin_6B(byte* button) //Prepare button value for 6B pad
{
  //Prepare pin status
  button_arrow = (button[BUTTON_UP] << 4) | (button[BUTTON_RIGHT] << 5) | (button[BUTTON_DOWN] << 6) | (button[BUTTON_LEFT] << 7) | (PORTF & 0x0F);
  button_34 = (button[BUTTON_A] << 4) | (button[BUTTON_X] << 5) | (button[BUTTON_Y] << 6) | (button[BUTTON_Z] << 7) | (PORTF & 0x0F);
  button_ab = (button[BUTTON_C] << 4) | (button[BUTTON_B] << 5) | (button[BUTTON_M] << 6) | (button[BUTTON_S] << 7) | (PORTF & 0x0F);
  button_low = 0 | (PORTF & 0x0F);
}

byte confirm_switch(byte* button, byte* switch_mode_enable, int* switch_mode_count, int count)
{
  byte ret = false;
  if(button[BUTTON_SPECIAL] == 0)
  {
    if(*switch_mode_enable == true)
    {
      (*switch_mode_count)++;
      if((*switch_mode_count) > count)
      {
        ret = true;
      }
    }
  }
  else
  {
    *switch_mode_count = 0;
    *switch_mode_enable = true;
  }
  
  return ret;
}

//AvenuePad3 mode;
void AvenuePad3(void)
{
  byte timing; //Find timing to get pad status
  byte padflag; //Status of pad status

  byte switch_mode_enable; //Variable for button select system
  int switch_mode_count; //Variable for button select system
  
  byte button[13]; //Total of 13 Megadrive pad buttons
  byte pad_switch_flag; //Flag to switch 3B and 6B pad
  int i;

  //Initialize variables for rapid fire system
  button_repeat_status repeat_status[3];
  button_select_status select_status;

  //Initalize variables for button select system
  switch_mode_count = 0;
  switch_mode_enable = false;
  
  //Initialize button repeat status
  for(i = 0; i < 3; i++)
  {
    repeat_status[i].button_internal = BUTTON_PRESSED;
    repeat_status[i].button_wait = 0;
    repeat_status[i].repeat_mode = 0;
    repeat_status[i].repeat_counter = 0;
    repeat_status[i].button_next = BUTTON_RELEASED;
  }

  //Initailze variables for button select system(RUN or SELECT)
  select_status.button_internal = BUTTON_PRESSED;
  select_status.button_wait = 0;
  select_status.current_mode = RUN;

  //Initialize current button press
  button_arrow = 0xF0 | (0x0F & PINF);
  button_ab = 0xF0 | (0x0F & PINF);
  button_34 = 0xF0 | (0x0F & PINF);
  button_low = (0x0F & PINF);
  
  pad_switch_flag = 0;
  
  while(1)
  {
    //Output arrow button
    PORTF = button_arrow;

    while((PINB & 0x02))
    {
      if(PINB & 0x08)
      {
        //If EN is high, output low and toggle button select
        PORTF = button_low;

        //Wait until EN become low
        timing = 0;
        padflag = 0;
        while(PINB & 0x08)
        {
          //If this wait is longer than 200us, get pad status here
          timing++;
          if((padflag == 0) && (timing > 150))//About 200us
          {
            get_pad(button);
            button_repeat(&button[BUTTON_X], &button[BUTTON_A], &repeat_status[BUTTON_INTERNAL_X]); //Rapid fire system
            button_repeat(&button[BUTTON_Y], &button[BUTTON_B], &repeat_status[BUTTON_INTERNAL_Y]); //Rapid fire system
            button_repeat(&button[BUTTON_Z], &button[BUTTON_C], &repeat_status[BUTTON_INTERNAL_Z]); //Rapid fire system
            button_select(&button[BUTTON_SPECIAL], &select_status);      
            prepare_pin_3B(button, select_status.current_mode);
            pad_switch_flag = confirm_switch(button, &switch_mode_enable, &switch_mode_count, 90);
            padflag = 1;
          }
        }
        PORTF = button_arrow;
      }
    }

    PORTF = button_ab;
    if(pad_switch_flag == 1)
    {
      break;
    }
    
    timing = 0;
    padflag = 0;
    while((PINB & 0x02) == 0) //While SEL is 0
    {
      timing++;
      if((padflag == 0) && (timing > 150))//About 200us
      {
        get_pad(button);
        button_repeat(&button[BUTTON_X], &button[BUTTON_A], &repeat_status[BUTTON_INTERNAL_X]);
        button_repeat(&button[BUTTON_Y], &button[BUTTON_B], &repeat_status[BUTTON_INTERNAL_Y]);
        button_repeat(&button[BUTTON_Z], &button[BUTTON_C], &repeat_status[BUTTON_INTERNAL_Z]);
        button_select(&button[BUTTON_SPECIAL], &select_status);      
        prepare_pin_3B(button, select_status.current_mode);
        pad_switch_flag = confirm_switch(button, &switch_mode_enable, &switch_mode_count, 90);
        padflag = 1;
      }
    }
  }      
}


//AvenuePad6 mode;
void AvenuePad6(void)
{
  byte flip; //CS condition
  uint16_t timing; //Find timing to get pad status
  byte switch_mode_enable;
  int switch_mode_count;
  byte button[13]; //Total of 13 Megadrive pad buttons
  byte padflag ;
  byte pad_switch_flag;

  button_arrow = 0xF0 | (0x0F & PINF);
  button_ab = 0xF0 | (0x0F & PINF);
  button_34 = 0xF0 | (0x0F & PINF);
  button_low = (0x0F & PINF);

  switch_mode_enable = false;
  switch_mode_count = 0;

  padflag = 0;
  pad_switch_flag = 0;

  flip = 0;
  
  while(1)
  {
    //If CS is low, output low, else, output arrow button
    if(flip == 0)
    {
      PORTF = button_low;
    }
    else
    {
      PORTF = button_arrow;
    }
    while((PINB & 0x02))
    {
      if(PINB & 0x08)
      {
        //If EN is high, output low and toggle button select
        PORTF = button_low;
        flip ^= 1;

        //Wait until EN become low
        timing = 0;
        padflag = 0;
        while(PINB & 0x08)
        {
          timing++;
          if((padflag == 0) && (timing > 150))//About 200us
          {
            get_pad(button);
            prepare_pin_6B(button);
            pad_switch_flag = confirm_switch(button, &switch_mode_enable, &switch_mode_count, 90);
            padflag = 1;
          }
        }
        if(flip == 0)
        {
          PORTF = button_low;
        }
        else
        {
          PORTF = button_arrow;
        }
      }
    }

    //Once SEL is high, output button 3456 or 12selectrun depending on CS
    if(flip == 0)
    {
      PORTF = button_34;
    }
    else
    {
      PORTF = button_ab;
    }
    if(pad_switch_flag == 1)
    {
      break;
    }

    timing = 0;
    padflag = 0;
    while((PINB & 0x02) == 0) //While SEL is 0
    {
      timing++;
      if((padflag == 0) && (timing > 150))//About 200us
      {
        get_pad(button);
        prepare_pin_6B(button);
        pad_switch_flag = confirm_switch(button, &switch_mode_enable, &switch_mode_count, 90);
        padflag = 1;
      }
    }
  }    
}


void button_repeat(byte* button_input, byte* button_output, button_repeat_status* repeat_status)
{

  //If button wait window is 0
  if(repeat_status->button_wait == 0)
  {
    //If button's internal status is released and real button is pressed
    if((repeat_status->button_internal == BUTTON_RELEASED) && (*button_input == BUTTON_PRESSED))
    {
      //Change button's internal status to pressed
      repeat_status->button_internal = BUTTON_PRESSED;
      
      //Set button wait window
      repeat_status->button_wait = BUTTON_WAIT_WINDOW;
      
      //Toggle repeated mode
      (repeat_status->repeat_mode) ^= 1;
    } 
    //Else if button's internal status is pressed and real button is released
    else if((repeat_status->button_internal == BUTTON_PRESSED) && (*button_input == BUTTON_RELEASED))
    {
      //Change button's internal status to released to accept new button press
      repeat_status->button_internal = BUTTON_RELEASED;
    }
  }
  //Else
  else
  {
  //While in wait window, do not accept any button press and decrement button wait by 1
  (repeat_status->button_wait)--;
  }
  
  //If repeated mode is disabled
  if(!(repeat_status->repeat_mode))
  {
    //Set counter to 0
    repeat_status->repeat_counter = 0;
  }
  //Else if repeated mode is enabled
  else
  {
    //Increment counter
    (repeat_status->repeat_counter)++;
  
    //If counter equal to BUTTON_REPEAT_RATE
    if(repeat_status->repeat_counter == BUTTON_REPEAT_RATE)
    {
      //Set counter to 0
      repeat_status->repeat_counter = 0;
      
      //Toggle next button output
      (repeat_status->button_next) ^= 1;
      
      //Set output of button
      *button_output = repeat_status->button_next;
    }
  }
}


byte button_select(byte* button_input, button_select_status* select_status)
{

  //If button wait window is 0
  if(select_status->button_wait == 0)
  {
    //If button's internal status is released and real button is pressed
    if((select_status->button_internal == BUTTON_RELEASED) && (*button_input == BUTTON_PRESSED))
    {
      //Change button's internal status to pressed
      select_status->button_internal = BUTTON_PRESSED;
      
      //Set button wait window
      select_status->button_wait = BUTTON_WAIT_WINDOW;
      
      //Toggle repeated mode
      (select_status->current_mode) ^= 1;
    } 
    //Else if button's internal status is pressed and real button is released
    else if((select_status->button_internal == BUTTON_PRESSED) && (*button_input == BUTTON_RELEASED))
    {
      //Change button's internal status to released to accept new button press
      select_status->button_internal = BUTTON_RELEASED;
    }
  }
  //Else
  else
  {
  //While in wait window, do not accept any button press and decrement button wait by 1
  (select_status->button_wait)--;
  }
}

