
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ARCbus.h>
#include "commandLib.h"


const SYM_ADDR busAddrSym[]= {{"LEDL",BUS_ADDR_LEDL},
                              {"ACDS",BUS_ADDR_ACDS},
                              {"COMM",BUS_ADDR_COMM},
                              {"IMG",BUS_ADDR_IMG},
                              {"CDH",BUS_ADDR_CDH},
                              {"GC",BUS_ADDR_GC},
                              {NULL,0}};
              
const SYM_ADDR magAddrSym[]= {{"X+",0x14},
                              {"X-",0x16},
                              {"Y+",0x26},
                              {"Y-",0x34},
                              {"Z+",0x25},
                              {"Z-",0x24},
                              {NULL,0}};
                                                           
const SYM_ADDR tempAddrSym[]={{"X+",0x48},
                              {"X-",0x4A},
                              {"Y+",0x49},
                              {"Y-",0x4C},
                              {"Z+",0x4E},
                              {"Z-",0x4D},
                              {"Tb",0x4F},
                              {NULL,0}};
                                                                           
//helper function to lookup addresses given a symbolic name
unsigned char I2C_addr_lookup(const char *str,const SYM_ADDR *syms){
  int i;
  //look for a match
  for(i=0;syms[i].name!=NULL && syms[i].addr!=0;i++){
    if(!strcmp(str,syms[i].name)){
      return syms[i].addr;
    }
  }
  //nothing found
  return 0xFF;
}      

//helper function to lookup symbolic name given an address
const char *I2C_addr_revlookup(char addr,const SYM_ADDR *syms){
  int i;
  //look for a match
  for(i=0;syms[i].name!=NULL && syms[i].addr!=0;i++){
    if(syms[i].addr==addr){
      return syms[i].name;
    }
  }
  //nothing found
  return NULL;
}

//helper function to parse I2C address
//if res is true reject reserved addresses
unsigned char getI2C_addr(const char *str,short res,const SYM_ADDR *syms){
  unsigned long addr;
  unsigned char tmp;
  int i;
  char *end;
  //attempt to parse a numeric address
  addr=strtol(str,&end,0);
  //check for errors
  if(end==str){
    //check for symbolic matches
    if(syms!=NULL){
      //lookup address
      addr=I2C_addr_lookup(str,syms);
      //check if match was found
      if(addr!=0xFF){
        //return match
        return addr;
      }
    }
    //not a known address, error
    printf("Error : could not parse address \"%s\".\r\n",str);
    return 0xFF;
  }
  if(*end!=0){
    printf("Error : unknown sufix \"%s\" at end of address\r\n",end);
    return 0xFF;
  }
  //check address length
  if(addr>0x7F){
    printf("Error : address 0x%04lX is not 7 bits.\r\n",addr);
    return 0xFF;
  }
  //check for reserved address
  tmp=0x78&addr;
  if((tmp==0x00 || tmp==0x78) && res){
    printf("Error : address 0x%02lX is reserved.\r\n",addr);
    return 0xFF;
  }
  //return address
  return addr;
}
