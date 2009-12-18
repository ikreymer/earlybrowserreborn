/*
 * msgHandler.h
 *
 * The messageHandler mechanism is the way Viola scripts can invoke external
 * C routines of the front end program.
 *
 * Scott Silvey, 7/20/93
 *
 */

#ifndef _MSGHANDLER_H_
#define _MSGHANDLER_H_


typedef void (*VMsgHandler)(/*
			    char *arg[],
			    int argc,
			    void *clientData,
			    */);

typedef struct VMsgHandlerStruct {
    char *msgName;
    VMsgHandler msgHandler;
    void *clientData;
    long handlerID;
    struct VMsgHandlerStruct *next;
} VMsgHandlerStruct;


/*
 * Registers a message handler that responds to the message named in msgName.
 */
void ViolaRegisterMesssageHandler(/*
			          char *msgName,
			          VMsgHandler msgHandler,
				  void *clientData,
				  long handlerID
			          */);

/*
 * Deletes the message handler named by msgName and linked to the function
 * msgHandler.
 */
void ViolaDeleteMessageHandler(/*
			       char *msgName,
			       VMsgHandler msgHandler,
			       void *clientData,
			       long handlerID
			       */);

/*
 * Looks up and invokes all message handlers registered to the message
 * named by msgName.
 */
void ViolaInvokeMessageHandler(/*
			       char *arg[],
			       int argc,
			       long handlerID
			       */);
			       

#endif _MSGHANDLER_H_

    
