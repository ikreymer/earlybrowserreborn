/*
 * event.c
 */
#include "utils.h"
#include <ctype.h>
#include <sys/types.h>
#include "hash.h"
#include "ident.h"
#include "obj.h"
#include "packet.h"
#include "event.h"

int currentTool = ACTION_TOOL;
char *toolID2Str[] = {
	"action",
	"target",
	"move",
	"resize",
	"reparent",
	NULL
};

/*
void processPeekKeys()
{
  if (keyStat_control && keyStat_shift) {
    method_generic_renderObjectParam(VioCurrentObj);
    objectPeekFlag = 1;
  } else {
    if (objectPeekFlag) {
      objectPeekFlag = 0;
      method_generic_renderObjectParam(VioCurrentObj);
    }
  }
}
*/

/*
int checkParam(param, resize_corner)
     Param *param;
     int resize_corner;
{
  if (ParamWidth(param) < 1) {
    ParamWidth(param) = 1;
    if ((resize_corner == RC_LEFT) ||
	(resize_corner == RC_UPPER_LEFT) ||
	(resize_corner == RC_LOWER_LEFT))
      resize_corner = RC_RIGHT;
    else 
      resize_corner = RC_LEFT;

  } else if (ParamHeight(param) < 1) {
    ParamHeight(param) = 1;
    if ((resize_corner == RC_UPPER) ||
	(resize_corner == RC_UPPER_RIGHT) ||
	(resize_corner == RC_UPPER_LEFT))
      resize_corner = RC_LOWER;
    else 
      resize_corner = RC_UPPER;
  }
  return resize_corner;
}
*/

