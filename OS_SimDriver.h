// protect from multiple compiling
#ifndef OS_SIMDRIVER_H
#define OS_SIMDRIVER_H

// header files
#include <stdio.h>
#include <stdbool.h>
#include "datatypes.h"
#include "stringUtil.h"
#include "configops.h"
#include "metadataops.h"
#include "simulator.h"

// program constants
typedef enum {MIN_NUM_ARGS = 3, LAST_FOUR_LETTERS = 4} PRGRM_CONSTANTS;

// command line struct for storing command line switch settings
typedef struct CmdLineDataStruct
{
   bool programRunFlag;
   bool configDisplayFlag;
   bool mdDisplayFlag;
   bool runSimFlag;

   char fileName[STD_STR_LEN];
} CmdLineData;

// function prototypes
/*
Name: processCmdLine
Process: checks for at least two arguments, then sets Booleans depending on
         command line switches which can be in any order, also captures config
         file name which must be the last argument
Function Input/Parameters: number of arguments (int), 
                           vector of arguments (char **)
Function Output/Parameters: pointer to command line structure (CmdLineData *)
                            with updated nmembers, set to default values if 
                            failiure to capture arguments
Function Output/Returned: Boolean result of argument capture, true if at least 
                          one switch and config file name, false otherwise
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: tbd
*/
bool processCmdLine(int numArgs, char **strVector, CmdLineData *clDataPtr);

 /*
Name: showCommandLineFormat
Process: displays command line format as assistance to user
Function Input/Parameters: none
Function Output/Parameters: none
Function Output/Returned: none
Device Input/Keyboard: none
Device Output/Monitor: data displayed as specified
Dependencies: printf
*/
void showCommandLineFormat();

/*
Name: 
Process: 
Function Input/Parameters: 
Function Output/Parameters: 
Function Output/Returned: 
Device Input/Keyboard: 
Device Output/Monitor: 
Dependencies: 
*/


#endif // OS_SIMDRIVER_H