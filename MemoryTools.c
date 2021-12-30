/*******************************************************************************
* EECE344 Lab 2
* MemChkSum() - Fills a block of memory with fillsum
*
*
* Andy Nguyen, 10/18/2021
* Todd Morton, 9/21/2020
*******************************************************************************/




#include "MCUType.h"               /* Include header files                    */
#include "MemoryTools.h"


INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr){

	INT8U *byteptr;
	INT16U fillsum;


	byteptr = endaddr;

	while (byteptr <= startaddr){
		fillsum = (fillsum + *byteptr);   //adds the values for checksum file

		byteptr++;


	}
	return fillsum;

}
