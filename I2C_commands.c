#include <stdlib.h>
#include <stdio.h>
#include <i2c.h>
#include "commandLib.h"


int I2C_scan_Cmd(char **argv,unsigned short argc){
  int i,res,found=0;
  unsigned char rx[3];
  const char *name;
  for(i=0;i<0x7F;i++){
    res=i2c_rx(i,rx,3);
    if(res>=0){
      //device found, see if it is one that we recognize
      if((name=I2C_addr_revlookup(i,magAddrSym))){
        printf("Device Found at : %i (Mag %s)\r\n",i,name);
      }else if((name=I2C_addr_revlookup(i,tempAddrSym))){
        printf("Device Found at : %i (Temp %s)\r\n",i,name);
      }else{
        printf("Device Found at : %i\r\n",i);
      }
      //count the number of found devices
      found++;
    }else if(res!=I2C_ERR_NACK){
        //There was an error, abort scan
        printf("Error Encountered \"%s\". Aborting at %i\r\n",I2C_error_str(res),i);
        return 1;
    }
  }
  //check if devices were found
  if(!found){
    printf("Scan Complete, No Devices found\r\n");
  }else{
    printf("Scan Complete, %i Devices found\r\n",found);
  }
  return 0;
}
    
    
    
//transmit command over I2C
int I2C_txCmd(char **argv,unsigned short argc){
  unsigned char buff[10];
  unsigned char addr;
  unsigned short len;
  short res;
  char *end;
  int i;
  //check number of arguments
  if(argc<2){
    printf("Error : too few arguments.\r\n");
    return 1;
  }
  if(argc>sizeof(buff)){
    printf("Error : too many arguments.\r\n");
    return 2;
  }
  //get address
  addr=getI2C_addr(argv[1],0,NULL);
  if(addr==0xFF){
    return 1;
  }
  //get data to transmit
  for(i=0;i<argc-1;i++){
    buff[i]=strtol(argv[i+2],&end,0);
    if(end==argv[i+2]){
        printf("Error : could not parse element \"%s\".\r\n",argv[i+2]);
        return 2;
    }
    if(*end!=0){
      printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[i+2]);
      return 3;
    }
  }
  //save number of bytes read
  len=i;
  //send data
  res=i2c_tx(addr,buff,len);
  printf("Complete : %s\r\n",I2C_error_str(res));
  return 0;
}

int I2C_rxCmd(char **argv,unsigned short argc){
  unsigned char buff[10];
  unsigned char addr;
  unsigned short len;
  short res;
  char *end;
  int i;
  //check number of arguments
  if(argc<2){
    printf("Error : too few arguments.\r\n");
    return 1;
  }
  if(argc>2){
    printf("Error : too many arguments.\r\n");
    return 2;
  }
  //get address
  addr=getI2C_addr(argv[1],0,NULL);
  if(addr==0xFF){
    return 1;
  }
  //get number of bytes to read
  len=strtol(argv[2],&end,0);
  if(end==argv[2]){
      printf("Error : could not parse element \"%s\".\r\n",argv[2]);
      return 2;
  }
  if(*end!=0){
    printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[2]);
    return 3;
  }
  //send data
  res=i2c_rx(addr,buff,len);
  if(res<=0){
    printf("Error : %s\r\n",I2C_error_str(res));
  }else{
    for(i=0;i<len;i++){
      printf("0x%X ",buff[i]);
    }
    puts("\r");
  }
  return 0;
}

int I2C_txrxCmd(char **argv,unsigned short argc){
  unsigned char tx[10],rx[10];
  unsigned char addr;
  unsigned short rxlen,txlen;
  short res;
  char *end;
  int i;
  //check number of arguments
  if(argc<3){
    printf("Error : too few arguments.\r\n");
    return 1;
  }
  if(argc>sizeof(tx)){
    printf("Error : too many arguments.\r\n");
    return 2;
  }
  //get address
  addr=getI2C_addr(argv[1],0,NULL);
  if(addr==0xFF){
    return 1;
  }
  //get number of bytes to read
  rxlen=strtol(argv[2],&end,0);
  if(end==argv[2]){
      printf("Error : could not parse element \"%s\".\r\n",argv[2]);
      return 2;
  }
  if(*end!=0){
    printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[2]);
    return 3;
  }
  //get data to transmit
  for(i=0;i<argc-2;i++){
    tx[i]=strtol(argv[i+3],&end,0);
    if(end==argv[i+3]){
        printf("Error : could not parse element \"%s\".\r\n",argv[i+3]);
        return 2;
    }
    if(*end!=0){
      printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[i+3]);
      return 3;
    }
  }
  //get tx length
  txlen=i;
  //send data
  res=i2c_txrx(addr,tx,txlen,rx,rxlen);
  if(res>0){
    //no error, print out data
    puts("Commands sent successfully\r");
    for(i=0;i<rxlen;i++){
      printf("0x%X ",rx[i]);
    }
    puts("\r");
  }else{
    printf("Error : %s\r\n",I2C_error_str(res));
  }
  return 0;
}
      