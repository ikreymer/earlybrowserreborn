/*
 * msgHandler.c
 *
 * The messageHandler mechanism is the way Viola scripts can invoke external
 * C routines of the front end program.
 *
 * Scott Silvey, 7/20/93
 *
 */

#include "msgHandler.h"
#include <stdio.h>
#include <stdlib.h>


static VMsgHandlerStruct *ViolaMessageHandlers = NULL;
static int deletedFirstMH = 0;


void ViolaRegisterMessageHandler(msgName, msgHandler, clientData)
    char *msgName;
    VMsgHandler msgHandler;
    void *clientData;
{
    if (!msgName || !msgHandler) {
	return;
    } else {
	VMsgHandlerStruct *newOne;

	newOne = (VMsgHandlerStruct *) malloc(sizeof(VMsgHandlerStruct));
	/*
	 * if (!newOne)
    	 *     outOfMemory();
	 */

	newOne->msgName = malloc(strlen(msgName)+1);
	/*
	 * if (!newOne->msgName)
	 *     outOfMemory();
	 */
	strcpy(newOne->msgName, msgName);
	newOne->msgHandler = msgHandler;
	newOne->clientData = clientData;

	if (!ViolaMessageHandlers) {
	    newOne->next = NULL;
	    ViolaMessageHandlers = newOne;
	} else {
	    newOne->next = ViolaMessageHandlers;
	    ViolaMessageHandlers = newOne;
	}
    }
}


void ViolaDeleteMessageHandler(msgName, msgHandler, clientData)
    char *msgName;
    VMsgHandler msgHandler;
    void *clientData;
{
    if (!msgName || !msgHandler || !ViolaMessageHandlers) {
	return;
    } else {
	VMsgHandlerStruct *mhp, *prev_mhp=NULL;

	mhp = ViolaMessageHandlers;
	while (mhp) {
	    if (!strcmp(msgName, mhp->msgName) &&
		msgHandler == mhp->msgHandler &&
		clientData == mhp->clientData) {
		VMsgHandlerStruct *junk = mhp;
		
		if (mhp == ViolaMessageHandlers) {
		    ViolaMessageHandlers = mhp->next;
		    deletedFirstMH = 1;
		}
		if (prev_mhp)
		    prev_mhp->next = mhp->next;
		mhp = mhp->next;
		
		free(junk->msgName);
		free(junk);
	    } else {
		prev_mhp = mhp;
		mhp = mhp->next;
	    }
	}
    }
}


void ViolaInvokeMessageHandler(arg, argc)
    char *arg[];
    int argc;
{
    int i;
    
    if (!arg[0] || !ViolaMessageHandlers) {
	return;
    } else {
	VMsgHandlerStruct *mhp = ViolaMessageHandlers;
	deletedFirstMH = 0;  /* static global used by ViolaDeleteMessageHandler() */

	while (mhp) {
	    if (!strcmp(arg[0], mhp->msgName))
		mhp->msgHandler(arg, argc, mhp->clientData);

	    if (deletedFirstMH) {
		mhp = ViolaMessageHandlers;
		deletedFirstMH = 0;
	    } else {
		mhp = mhp->next;
	    }
	}
    }
}
			       

