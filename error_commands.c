#include <string.h>
#include <stdlib.h>
#include <Error.h>
#include <SDlib.h>
#include <limits.h>

#include "commandLib.h"

//Named log levels
static const unsigned char logLevels[]={ERR_LEV_DEBUG,ERR_LEV_INFO,ERR_LEV_WARNING,ERR_LEV_ERROR,ERR_LEV_CRITICAL};
//Log level names
static const char *const(levelNames[])={"debug","info","warn","error","critical"};

//parse log level with symbolic names with offset
int parse_log_level(const char *str){
  unsigned char level;
  unsigned long tmp;
  char *end;
  int found=0,i;
  size_t len;
  //attempt to parse a numeric address
  level=strtol(str,&end,0);
  //check for errors
  if(end==str){
    //check for matching level names
    for(i=0,found=0;i<sizeof(logLevels)/sizeof(logLevels[0]);i++){
      //get string length
      len=strlen(levelNames[i]);
      //check if the argument starts with the name
      if(!strncmp(levelNames[i],str,len)){
        //check if this is the end
        if(str[len]=='\0'){
          //match found
          found=1;
          //set log level
          level=logLevels[i];
          //done
          break;
        }else if(str[len]=='+'){
          level=strtoul(&str[len+1],&end,0);
          //check for suffix
          if(*end!=0){
            printf("Error : unknown sufix \"%s\" at end of log level \"%s\"\r\n",end,str);
            return -1;
          }
          //match found
          found=1;
          //add base level
          level+=logLevels[i];
          //done, return level
          return level;
        }
      }
    }
    if(!found){
      printf("Error : Could not parse log level \"%s\"\r\n",str);
      return -2;
    }
  }else{
    //check for suffix
    if(*end!=0){
      printf("Error : unknown sufix \"%s\" at end of log level\r\n",end);
      return -3;
    }
  }
  return level;
}

//replay errors from the Error log
int replayCmd(char **argv,unsigned short argc){
    unsigned short num=0;
    unsigned char level=0;
    unsigned long tmp;
    char *end;
    int ret,i;
    if(argc>=1){
        //parse number
        tmp=strtoul(argv[1],&end,10);
        if(end==argv[1]){
            //print error
            printf("Error parsing num \"%s\"\r\n",argv[1]);
            return -1;
        }
        if(*end){
            printf("Error : unknown suffix \"%s\" while parsing num \"%s\".\r\n",end,argv[1]);
            return -2;
        }
        //saturate
        if(tmp>USHRT_MAX){
            num=USHRT_MAX;
        }else{
             num=tmp;
        }
    }
    if(argc>=2){ 
      ret=parse_log_level(argv[2]);
      //check for error
      if(ret<0){
        return ret;
      }
      //set level
      level=ret;
    }
    //replay log
    error_log_replay(num,level);
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
  int ret,i;
  size_t len;
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
    ret=parse_log_level(argv[1]);
    //check for error
    if(ret<0){
      return ret;
    }
    //set level
    level=ret;
    //set log level
    set_error_level(level);
  }
  //print (new) log level
  printf("Log level = %u\r\n",get_error_level());
  return 0;
}
