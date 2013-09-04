#include <Error.h>
#include <SDlib.h>

#include "commandLib.h"


//replay errors from the Error log
int replayCmd(char **argv,unsigned short argc){
  error_log_replay();
  return 0;
}

//clear saved errors from the SD card
int clearErrCmd(char **argv,unsigned short argc){
  int ret;
  ret=clear_saved_errors();
  if(ret){
    printf("Error erasing errors : %s\r\n",SD_error_str(ret));
  }
  return 0;
}

