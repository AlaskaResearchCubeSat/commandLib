
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <msp430.h>

#include <ARCbus.h>
#include <UCA1_uart.h>
#include <Error.h>

#include "commandLib.h"

//reset a MSP430 on command
int restCmd(char **argv,unsigned short argc){
  unsigned char buff[10];
  unsigned char addr;
  unsigned short all=0;
  int resp;
  //force user to pass no arguments to prevent unwanted resets
  if(argc>1){
    puts("Error : too many arguments\r");
    return -1;
  }
  if(argc!=0){
    if(!strcmp(argv[1],"all")){
      all=1;
      addr=BUS_ADDR_GC;
    }else{
      //get address
      addr=getI2C_addr(argv[1],0,busAddrSym);
      if(addr==0xFF){
        return 1;
      }
    }
    //setup packet 
    BUS_cmd_init(buff,CMD_RESET);
    resp=BUS_cmd_tx(addr,buff,0,0,BUS_I2C_SEND_FOREGROUND);
    switch(resp){
      case 0:
        puts("Command Sent Sucussfully.\r");
      break;
      case ERR_TIMEOUT:
        puts("IIC timeout Error.\r");
      break;
    }
  }
  //reset if no arguments given or to reset all boards
  if(argc==0 || all){
    //wait for UART buffer to empty
    while(UCA1_CheckBusy());
    //write to WDTCTL without password causes PUC
    reset(ERR_LEV_INFO,ERR_SRC_CMD,CMD_ERR_RESET,0);
    //Never reached due to reset
    puts("Error : Reset Failed!\r");
  }
  return 0;
}


//print current time
int timeCmd(char **argv,unsigned short argc){
  //print time ticker value
  printf("time ticker = %li\r\n",get_ticker_time());
  return 0;
}

//transmit an arbitrary command over I2C
int txCmd(char **argv,unsigned short argc){
  unsigned char buff[10],*ptr,id;
  unsigned char addr;
  unsigned short len;
  unsigned int e;
  char *end;
  int i,resp,nack=BUS_CMD_FL_NACK;
  if(!strcmp(argv[1],"noNACK")){
    nack=0;
    //shift arguments
    argv[1]=argv[0];
    argv++;
    argc--;
  }
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
  addr=getI2C_addr(argv[1],0,busAddrSym);
  if(addr==0xFF){
    return 1;
  }
  //get packet ID
  id=strtol(argv[2],&end,0);
  if(end==argv[2]){
      printf("Error : could not parse element \"%s\".\r\n",argv[2]);
      return 2;
  }
  if(*end!=0){
    printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[2]);
    return 3;
  }
  //setup packet 
  ptr=BUS_cmd_init(buff,id);
  //pares arguments
  for(i=0;i<argc-2;i++){
    ptr[i]=strtol(argv[i+3],&end,0);
    if(end==argv[i+1]){
        printf("Error : could not parse element \"%s\".\r\n",argv[i+3]);
        return 2;
    }
    if(*end!=0){
      printf("Error : unknown sufix \"%s\" at end of element \"%s\"\r\n",end,argv[i+3]);
      return 3;
    }
  }
  len=i;
  resp=BUS_cmd_tx(addr,buff,len,nack,BUS_I2C_SEND_FOREGROUND);
  switch(resp){
    case RET_SUCCESS:
      printf("Command Sent Sucussfully.\r\n");
    break;
  }
  //check if an error occured
  if(resp<0){
    printf("Error : unable to send command\r\n");
  }
  printf("Resp = %i\r\n",resp);
  return 0;
}

//Send data over SPI
int spiCmd(char **argv,unsigned short argc){
  unsigned char addr;
  char *end;
  unsigned short crc;
  //static unsigned char rx[2048+2];
  unsigned char *rx=NULL;
  int resp,i,len=100;
  if(argc<1){
    printf("Error : too few arguments.\r\n");
    return 3;
  }
  //get address
  addr=getI2C_addr(argv[1],0,busAddrSym);
  if(addr==0xFF){
    return 1;
  }
  if(argc>=2){
    //Get packet length
    len=strtol(argv[2],&end,0);
    if(end==argv[2]){
        printf("Error : could not parse length \"%s\".\r\n",argv[2]);
        return 2;
    }
    if(*end!=0){
      printf("Error : unknown sufix \"%s\" at end of length \"%s\"\r\n",end,argv[2]);
      return 3;
    }    
    if(len+2>BUS_get_buffer_size()){
      printf("Error : length is too long. Maximum Length is %u\r\n",BUS_get_buffer_size());
      return 4;
    }
  }
  //get buffer, set a timeout of 2 secconds
  rx=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(rx==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //fill buffer with "random" data
  for(i=0;i<len;i++){
    rx[i]=i;
  }
  //send SPI data
  resp=BUS_SPI_txrx(addr,rx,rx,len);
  //TESTING: wait for transaction to fully complete
  while(UCB0STAT&UCBBUSY);
  //check return value
  if(resp==RET_SUCCESS){
      //print out data message
      printf("SPI data recived\r\n");
      //print out data
      for(i=0;i<len;i++){
        //printf("0x%02X ",rx[i]);
        printf("%03i ",rx[i]);
      }
      printf("\r\n");
  }else{
    printf("%s\r\n",BUS_error_str(resp));
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}

//connect to a remote board via async
int asyncProxyCmd(char **argv,unsigned short argc){
   char c;
   int err;
   CTL_EVENT_SET_t e=0,evt;
   unsigned char addr;
   if(argc>1){
    printf("Error : %s takes 0 or 1 arguments\r\n",argv[0]);
    return -1;
  }
  if(argc==1){
    addr=getI2C_addr(argv[1],0,busAddrSym);
    if(addr==0xFF){
      return -1;
    }
    //print out address
    printf("Using Address 0x%02X\r\n",addr);
    //try to open async connection
    if((err=async_open(addr))){
      //print error
      printf("Error : opening async\r\n%s\r\n",BUS_error_str(err));
      //return error
      return -2;
    }
    //Tell the user that async is open
    printf("async open use ^C to force quit\r\n");
    //setup events
    async_setup_events(&e,0,1<<0);
    UCA1_setup_events(&e,0,1<<1);
    async_setup_close_event(&e,1<<2);
    for(;;){
      //wait for event
      evt=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS,&e,0x07,CTL_TIMEOUT_NONE,0);
      //check for char from UART
      if(evt&(1<<1)){
        //get char
        c=UCA1_Getc();
        //check for ^C
        if(c==0x03){
          //close connection
          async_close();
          //print message
          printf("\r\nConnection terminated by user\r\n");
          //exit loop
          break;
        }
        //send char over async
        async_TxChar(c);
      }
      //check for char from async
      if(evt&(1<<0)){
        //get char from async
        c=async_Getc(); 
        //print char to UART
        UCA1_TxChar(c);
      }
      if(evt&(1<<2)){
        //print message
        printf("\r\nconnection closed remotely\r\n");
        //exit loop
        break;
      }
    }
    //stop monitoring events
    async_setup_events(NULL,0,0);
    UCA1_setup_events(NULL,0,0);
    async_setup_close_event(NULL,0);
  }else{
    if(async_close()){
      printf("Error : async_close() failed.\r\n");
    }
  }
}

//find devices on the ARCbus
int ARCsearch_Cmd(char **argv,unsigned short argc){
  //data for dummy command
  unsigned char buff[BUS_I2C_CRC_LEN+BUS_I2C_HDR_LEN],*ptr,*end;
  int i,ret,found=0;
  //setup bogus command
  //TODO: perhaps there should be a PING command that is sort of a no-operation command
  ptr=BUS_cmd_init(buff,7);
  //loop through all I2C addresses and send a command to see if there is a device at that address
  for(i=0;i<=0x7F;i++){
    //send command
    ret=BUS_cmd_tx(i,buff,0,0,BUS_I2C_SEND_FOREGROUND);
    if(ret==RET_SUCCESS){
      printf("Device Found at ADDR = 0x%02X\r\n",i);
      found++;
    }else if(ret!=ERR_I2C_NACK){
      printf("Error sending to addr 0x%02X : %s\r\n",i,BUS_error_str(ret));
    }
  }
  if(found==0){
    printf("No devices found on the ARCbus\r\n");
  }else{
    printf("%i %s found on the ARCbus\r\n",found,found==1?"device":"devices");
  }
  return 0;
}

