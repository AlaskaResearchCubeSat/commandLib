#include <string.h>
#include <stdlib.h>
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

//set which errors are logged
int logCmd(char **argv,unsigned short argc){
  const unsigned char logLevels[]={ERR_LEV_DEBUG,ERR_LEV_INFO,ERR_LEV_WARNING,ERR_LEV_ERROR,ERR_LEV_CRITICAL};
  const char *(levelNames[])={"debug","info","warn","error","critical"};
  int found,i;
  char *end;
  unsigned char level;
  //check for too many arguments
  if(argc>1){
    printf("Error : %s takes 0 or 1 arguments\r\n",argv[0]);
    return -1;
  }
  //check if argument given
  if(argc==1){
    if(!strcmp("levels",argv[1])){
      //print a list of level names
      for(i=0;i<sizeof(logLevels)/sizeof(logLevels[0]);i++){
        printf("% 3u - %s\r\n",logLevels[i],levelNames[i]);
       }
       return 0;
    }    
    //attempt to parse a numeric address
    level=strtol(argv[1],&end,0);
    //check for errors
    if(end==argv[1]){
      //check for matching level names
      for(i=0,found=0;i<sizeof(logLevels)/sizeof(logLevels[0]);i++){
        if(!strcmp(levelNames[i],argv[1])){
          //match found
          found=1;
          //set log level
          level=logLevels[i];
          //done
          break;
        }
      }
      if(!found){
        printf("Error : Could not parse log level \"%s\"\r\n",argv[1]);
        return -2;
      }
    }else{
      //check for suffix
      if(*end!=0){
        printf("Error : unknown sufix \"%s\" at end of log level\r\n",end);
        return -3;
      }
    }
    //set log level
    set_error_level(level);
  }
  //print (new) log level
  printf("Log level = %u\r\n",get_error_level());
  return 0;
}
