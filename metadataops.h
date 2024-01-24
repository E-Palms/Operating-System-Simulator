#ifndef METADATAOPS_H
#define METADATAOPS_H

#include <stdio.h>
#include <stdbool.h>
#include "stringUtil.h"
#include "StandardConstants.h"

typedef struct OpCodeStruct
   {
    char command[STD_STR_LEN];
    char inOutArg[STD_STR_LEN];
    char strArg1[STD_STR_LEN];

    int pid;
    int intArg2;
    int intArg3;

    double opEndTime;

    struct OpCodeStruct *nextNode;

   } OpCodeType;

// GLOBAL CONSTANTS
typedef enum { BAD_ARG_VAL = -1,
               NO_ACCESS_ERR,
               MD_FILE_ACCESS_ERR,
               MD_CORRUPT_DESCRIPTOR_ERR,
               OPCMD_ACCESS_ERR,
               CORRUPT_OPCMD_ERR,
               CORRUPT_OPCMD_ARG_ERR,
               UNBALANCED_START_END_ERR,
               COMPLETE_OPCMD_FOUND_MSG,
               LAST_OPCMD_FOUND_MSG } OpCodeMessages;

// functions
/*
Name: addNode
Process: adds metadata node to linked list recursively,
         handles empty list condition
Function Input/Parameters: points to new head or next linked node (OpCodeType *)
                           points to new node (OpCodeType *)
Function Output/Parameters: none
Function Output/Returned: pointer to previous node, or head node (OpCodeType *)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: malloc, copyString
*/
OpCodeType *addNode(OpCodeType *localPtr, OpCodeType *newNode);

/*
Name: clearMetaDataList
Process: recursively traverses list, frees dynamically allocated nodes
Function Input/Parameters: node op code (const OpCodeType *)
Function Output/Parameters: none
Function Output/Returned: NULL (OpCodeType *)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: tbd
*/
OpCodeType *clearMetaDataList(OpCodeType *localPtr);

/*
Name: displayMetaData
Process: data dump/display of all op code items
Function Input/Parameters: pointer to head
                           of op code/metadata list (const OpCodeType *)
Function Output/Parameters: none
Function Output/Returned: none
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: tbd
*/
void displayMetaData(const OpCodeType *localPtr);

/*
Name: getCommand
Process: parses three letter command part of op code string
Function Input/Parameters: input op code string (const char *)
                           starting index (int)
Function Output/Parameters: parsed command (char *)
Function Output/Returned: updated starting index for use
                          by calling function (int)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: none
*/
int getCommand(char *cmd, const char *inputSTr, int index);

/*
Name: getNumberArg
Process: starts at given index, captures and assembles integer argument,
         and returns as parameter
Function Input/Parameters: input string (const char *), starting index (int)
Function Output/Parameters: pointer to captured integer value
Function Output/Returned: updated index for next function start
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: isDigit
*/
int getNumberArg(int *number, const char *inputStr, int index);

/*
Name: getMetaData
Process: main driver function to upload, parse, and store list
         of op code commands in a linked list
Function Input/Parameters: file name (char *)
Function Output/Parameters: pointer
                            to op code linked list head pointer (OpCodetype **)
                            result message of function state
                            after completion (char *)
Function Output/Returned: Boolean result of operation (bool)
Device Input/Keyboard: op code list uploaded
Device Output/Monitor: none
Dependencies: copyString, fopen, getStringToDelimiter, compareString, fclose,
              malloc, getOpCommand, updateStartCount, updateEndCount,
              clearMetaDataList, free, addNode
*/
bool getMetaData(const char *filename,
                              OpCodeType **opCodeDataHead, char *endStateMsg);

/*
Name: getStringArg
Process: starts at given index, captures and assembles string argument,
         and returns as parameter
Function Input/Parameters: input string (const char *), starting index (int)
Function Output/Parameters: pointer to captured string argument (char *)
Function Output/Returned: updated index for next function start
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: none
*/
int getStringArg(char *strArg, const char *inputStr, int index);

/*
Name: getOpCommand
Process: aquires one op command line from a previously opened file,
         parses it, and sets various struct members according
         to the three letter command
Function Input/Parameters: pointer to open file handle (FILE *)
Function Output/Parameters: pointer to one op code struct (OpCodeType *)
Function Output/Returned: coded result of operation (OpCodeMessages)
Device Input/Keyboard: op code line uploaded
Device Output/Monitor: none
Dependencies: getStringToDelimiter, getCommand, copyString, verifyValidCommand,
              compareString, getStringArg, verifyFirstStringArg, getNumberArg
*/
OpCodeMessages getOpCommand(FILE *filePtr, OpCodeType *inData);

/*
Name: isDigit
Process: tests character parameter for digit, returns true if is digit,
         false otherwise
Function Input/Parameters: test character (char)
Function Output/Parameters: none
Function Output/Returned: Boolean result of test (bool)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: none
*/
bool isDigit(char testChar);

/*
Name: updateEndCount
Process: manages count of "end" arguments to be compared at end
         of process input
Function Input/Parameters: initial count (int)
                           test string for "end" (const char *)
Function Output/Parameters: none
Function Output/Returned: updated count, if "end" string found,
                          otherwise no change
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: compareString
*/
int updateEndCount(int count, const char *opString);

/*
Name: updateStartCount
Process: manages count of "start" arguments to be compared at end
         of process input
Function Input/Parameters: initial count (int)
                           test string for "start" (const char *)
Function Output/Parameters: none
Function Output/Returned: updated count, if "start" string found,
                          otherwise no change
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: compareString
*/
int updateStartCount(int count, const char *opString);

/*
Name: verifyFirstStringArg
Process: checks for all possibilities of first argument string of op command
Function Input/Parameters: test string (const char *)
Function Output/Parameters: none
Function Output/Returned: Boolean  result of test (bool)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: compareString
*/
bool verifyFirstStringArg(const char *strArg);

/*
Name: verifyValidCommand
Process: checks for all possibilities of three-letter op code command
Function Input/Parameters: test string for command (const char *)
Function Output/Parameters: none
Function Output/Returned: Boolean result of test (bool)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: compareString
*/
bool verifyValidCommand(char *testCmd);

#endif // METADATAOPS_H