#include "midas.uic"
#include "midaswww.uic"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>

/* MidasMakeUid (): to make a uid file from the data input of a uic file.
   parameters: filename 
   return: 1 if make uid file success, 0 if make uid file fail.
           filename if make uid file success
*/

int MidasMakeUid(filename)
char **filename;
{
	int fileout;
 	char *buffer = XtMalloc(L_tmpnam); 
	char *temp = tmpnam(buffer);
        
	if ((fileout = open(temp, O_CREAT | O_RDWR, 0666)) > 0)
        {
  	  if (strstr(*filename, "midas.uid"))
	      write(fileout, midas, sizeof(midas)); 
	  else if (strstr(*filename, "midaswww.uid"))
	      write(fileout, midaswww, sizeof(midaswww));
	  else
	  {
	      printf("MidasMakeUid error: don't find the UIC file of %s\n",*filename);
	      unlink(temp);
              XtFree(temp);
	      return 0;
	  }
	  *filename = temp;
	  return 1;
	}
        else
        { 
		printf("Unable to open file %s \n",temp);
		XtFree(temp);
		return 0;
	}
}
