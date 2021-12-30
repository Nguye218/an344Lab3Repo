/*******************************************************************************
* EECE344 Lab 3 Demo
*
* This lab Calls Checksum in the beginning once and the displays the User with a
* UI that they can interact with. depending on the user input the program will
* go into the three different counters, software, hardware, and combination
* counter. all three counters requires the user to press a button to initiate
* the counter. This lab had us explore interrupts and User interfaces
*
* Andy Nguyen, 10/25/2021
* Todd Morton (RTIO document)
*******************************************************************************/

#include "MCUType.h"               /* Include header files                    */
#include "BasicIO.h"
#include "K65TWR_ClkCfg.h"
#include "MemoryTools.h"

#include "K65TWR_GPIO.h"
#include "MK65F18.h"

#define LOW_ADD 0x00000000
#define HIGH_ADD 0x001FFFFF

static INT16U SumAnswer;

INT16U COUNT;
INT16U MaxCount = 999;

typedef enum {HW_CNTR, SW_CNTR, COMBO_CNTR, COM_PARSE}UI_STATES_T;
UI_STATES_T CurrentState;


typedef enum {IN_LOW, IN_HIGH}SW_STATES_T;
SW_STATES_T SwState;

void PORTA_IRQHandler(void);   //public handler prototype


void main(void){

	K65TWR_BootClock();             /* Initialize MCU clocks                  */

    BIOOpen(BIO_BIT_RATE_115200);   /* Initialize Serial Port                 */


    INT8C char_in;                  /* Received character                     */
	INT8U in_curr;					// current count
	INT16U pre_count;				// Initialize pre-count for counter

	CurrentState = COM_PARSE;

	BIOPutStrg("*********************************************************");
	BIOPutStrg("\n\r");

	BIOPutStrg("\n\r");
    BIOPutStrg("Hello welcome to lab3: \n\r");

	SumAnswer = MemChkSum((INT8U*)HIGH_ADD,(INT8U*)LOW_ADD);

	BIOPutStrg("CS: ");
	BIOOutHexWord(LOW_ADD);
	BIOPutStrg(" - ");
	BIOOutHexWord(HIGH_ADD);
	BIOPutStrg(" = ");

	BIOOutHexWord(SumAnswer);

	BIOPutStrg("\n\r");
	BIOPutStrg("\n\r");

    while(1){

    	switch(CurrentState){

    	case COM_PARSE:

    		BIOPutStrg("*********************************************************");
    		BIOPutStrg("\n\r");
    		BIOPutStrg("Command Parse: \n\r");
    		BIOPutStrg("Enter the following for different counters \n\r");
    		BIOPutStrg("'s' for Software Counter \n\r");
    		BIOPutStrg("'h' for Hardware Counter \n\r");
    		BIOPutStrg("'b' for Combination Counter \n\r");
    		BIOPutStrg("Press 'q' at any point to return to Command Parse\n\r");
    		BIOPutStrg("\n\r");
    		BIOPutStrg("*********************************************************");
    		BIOPutStrg("\n\r");

    		char_in = BIOGetChar();         /* Wait for character to be entered       */

    		if(char_in == 'q'){

    			CurrentState = COM_PARSE;
    			BIOPutStrg("Command Parse\n\r");
    		}

    		else if (char_in == 'h'){

    			CurrentState = HW_CNTR;
    			BIOPutStrg("*Hardware Counter*\n\r");
    		}

    		else if (char_in == 'b'){

    			CurrentState = COMBO_CNTR;
    			BIOPutStrg("*Combination Counter*\n\r");
    		}

    		else if (char_in == 's'){

    			CurrentState = SW_CNTR;
    			BIOPutStrg("*Software Counter*\n\r");

    		}

    		break;


    	case HW_CNTR:

    		BIOPutStrg("('q' to exit)\n\r");

    		COUNT = 0;
    		pre_count = COUNT;

    	    SW2_CLR_ISF();  //Must do this before ClearPending and Enable

    	    NVIC_ClearPendingIRQ(PORTA_IRQn);
    	    NVIC_EnableIRQ(PORTA_IRQn);

    	    GpioSw2Init(PORT_IRQ_FE);


    		BIOPutStrg("[");
    		BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
        	BIOPutStrg("]\r");

    	    while(BIORead() != 'q'){

    	    	if(pre_count != COUNT && COUNT < MaxCount ){

    	    		pre_count = COUNT;   //update PreCount

    	    		BIOPutStrg("[");
    	    		BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
    	        	BIOPutStrg("]\r");

    	    	}

    	    	/* Loop until 'q' is pressed */

    		}
    	    NVIC_ClearPendingIRQ(PORTA_IRQn);
    	    NVIC_DisableIRQ(PORTA_IRQn);			//disabling IRQ, fixed a skip count in Software counter

    		CurrentState = COM_PARSE;


    	break;


    	case SW_CNTR:

    		BIOPutStrg("('q' to exit)\n\r");
			COUNT = 0;						    //reset the counter to zero

        	BIOPutStrg("[");
	        BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
	        BIOPutStrg("]\r");

    	    while(BIORead() != 'q'){

    	    	in_curr = SW2_INPUT;

    	        switch (SwState){
    	        case IN_LOW:
    	            if(in_curr == GPIO_PIN(SW2_BIT)){
    	            	SwState = IN_HIGH;
    	            }
    	            else{
    	                //nothing put here
    	            	}

    	            break;

    	        case IN_HIGH:
    	            if(in_curr == 0 && COUNT < MaxCount){
    	            	SwState = IN_LOW;
    	            	/* Service Event */
    	            		COUNT++;
    	            		BIOPutStrg("[");
    	            		BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
    	                	BIOPutStrg("]\r");
    	            }
    	            else{
    	            	//nothing put here
    	            	}

    	            break;
    	        default:

    	        	SwState = IN_LOW;
    	            break;
    	            }
    	    }

    		CurrentState = COM_PARSE;

    	break;


    	case COMBO_CNTR:

    		BIOPutStrg("('q' to exit)\n\r");

    		GpioSw2Init(PORT_IRQ_FE);   // init SW2 to detect a falling edge
    		SW2_CLR_ISF();        		// clear interrupt flag

			COUNT = 0;

    		BIOPutStrg("[");
    		BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
        	BIOPutStrg("]\r");


    		while(BIORead() != 'q'){


    	        if(SW2_ISF != 0 && COUNT < 999){      //check flag

            		COUNT++;
            		BIOPutStrg("[");
            		BIOOutDecWord(COUNT,3,BIO_OD_MODE_LZ);
                	BIOPutStrg("]\r");

                	SW2_CLR_ISF();      // Clear flag

    	        }

    	        else{
    	        	//nothing put here
    	        }

    		}
    		CurrentState = COM_PARSE;

    	break;

    	default:												//default case
    		break;

    		}
    	}
    }


void PORTA_IRQHandler(void){


    if(SW2_ISF != 0x00 && COUNT < MaxCount){
        SW2_CLR_ISF();

        /* Service Event */
        	COUNT++;


    }
    else{
        //other bit set here for PORTA if needed//
    }
 }
