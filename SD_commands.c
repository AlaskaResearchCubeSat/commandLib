#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <ARCbus.h>
#include <SDlib.h>
#include "commandLib.h"

//define printf formats
#define HEXOUT_STR    "%02X "


//check if the SD card has been initialized
int mmcInitChkCmd(char**argv,unsigned short argc){
  int resp;
  resp=mmc_is_init();
  if(resp==MMC_SUCCESS){
    printf("Card Initialized\r\n");
  }else{
    printf("Card Not Initialized\r\n%s\r\n",SD_error_str(resp));
  }
  return 0;
}

//write a string to the SD card
int mmc_write(char **argv, unsigned short argc){
  //pointer to buffer, pointer inside buffer, pointer to string
  unsigned char *buffer=NULL,*ptr=NULL,*string;
  //response from block write
  int resp;
  int i ;
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //clear all bytes in buffer
  memset(buffer,0,512);
  //concatenate arguments into one big string with spaces in between
  for(ptr=buffer,i=1; i<=argc; i++){
    string=(unsigned char*)argv[i];
    while(*string!=0){
      *ptr++=*string++;
    }
    *ptr++=' ';
  }
  //Terminate string
  *(ptr-1)=0;
  //write data
  resp=mmcWriteBlock(0,buffer);
  //check if write was successful
  if(resp==MMC_SUCCESS){
    printf("data written to memeory\r\n");
  }else{
    printf("resp = 0x%04X\r\n%s\r\n",resp,SD_error_str(resp));
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}

int mmc_read(char **argv, unsigned short argc){
  char *ptr=NULL,*buffer=NULL; 
  int resp;
   //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //init buffer
  memset(buffer,0,513);
  //read from SD card
  resp=mmcReadBlock(0,(unsigned char*)buffer);
  //check for error
  if(resp!=MMC_SUCCESS){
    //print error from SD card
    printf("%s\r\n",SD_error_str(resp));
    return 1;
  }
  //force termination after last byte
  buffer[512]=0;
  //check for non printable chars
  for(ptr=(char*)buffer;ptr!=0;ptr++){
    //check for non printable chars
    if(!isprint(*ptr)){
      //check for null
      if(*ptr){
        //not null, non printable char encountered
        printf("String prematurely terminated due to a non printable character.\r\n");
      }
      //terminate string
      *ptr=0;
      //exit loop
      break;
    }
  }
  //print out the string
  printf("String From SD card:\r\n\'%s\'\r\n",buffer);
  //free buffer
  BUS_free_buffer();
  return 0;
}

int mmc_dump(char **argv, unsigned short argc){
  int resp; 
  char *buffer=NULL;
  unsigned long sector=0;
  int i;
  //check if sector given
  if(argc!=0){
    //read sector
    if(1!=sscanf(argv[1],"%lu",&sector)){
      //print error
      printf("Error parsing sector \"%s\"\r\n",argv[1]);
      return -1;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //read from SD card
  resp=mmcReadBlock(sector,(unsigned char*)buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));
  //print out buffer
  for(i=0;i<512/16;i++){
    printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
    buffer[i*16],buffer[i*16+1],buffer[i*16+2],buffer[i*16+3],buffer[i*16+4],buffer[i*16+5],buffer[i*16+6],buffer[i*16+7],buffer[i*16+8],buffer[i*16+9],buffer[i*16+10],
    buffer[i*16+11],buffer[i*16+12],buffer[i*16+13],buffer[i*16+14],buffer[i*16+15]);
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}

//prototype for __putchar. why is this not in stdio.h?
int __putchar(int ch);

int mmcdat_Cmd(char **argv, unsigned short argc){
  int resp; 
  char *buffer=NULL;
  unsigned long sector=0;
  unsigned int check;
  int i;
  //check if sector given
  if(argc!=0){
    //read sector
    if(1!=sscanf(argv[1],"%lu",&sector)){
      //print error
      printf("Error parsing sector \"%s\"\r\n",argv[1]);
      return -1;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //read from SD card
  resp=mmcReadBlock(sector,(unsigned char*)buffer);
  //check if command was successful
  if(resp){
      printf("%s\r\n",SD_error_str(resp));
      //free buffer
      BUS_free_buffer();
      //return
      return resp;
  }
  //print sector 
  printf("Sending MMC block %lu\r\n",sector);
  //initialize check
  check=0;
  //print out buffer
  for(i=0;i<512;i++){
    putchar(buffer[i]);
    check=check+buffer[i];
  }
  printf("Check =  %u\r\n",check);
  //free buffer
  BUS_free_buffer();
  return 0;
}

//read card size
int mmc_cardSize(char **argv, unsigned short argc){
  unsigned long size;
  unsigned char CSD[16];
  int resp;
  resp=mmcReadReg(0x40|9,CSD);
  size=mmcGetCardSize(CSD);
  if(resp==MMC_SUCCESS){
    printf("card size = %luKB\r\n",size);
  }else{
    printf("%s\r\n",SD_error_str(resp));
  }
  return 0;
}

int mmc_eraseCmd(char **argv, unsigned short argc){
  unsigned long start,end;
  int resp;
  //check arguments
  if(argc!=2){
    printf("Error : %s requiors two arguments\r\n",argv[0]);
    return 1;
  }
  //clear errno
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  //check for error
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return 2;
  }
  printf("Erasing from %lu to %lu\r\n",start,end);
  //send erase command
  resp=mmcErase(start,end);
  printf("%s\r\n",SD_error_str(resp));
  return 0;
}

//data types for TstCmd
enum{DAT_LFSR=0,DAT_COUNT};

//return next value in data sequence
char dat_next(char v,int type){
  switch(type){
    //next value in the LFSR sequence x^8 + x^6 + x^5 + x^4 + 1
    case DAT_LFSR:
      //code taken from: http://en.wikipedia.org/wiki/Linear_feedback_shift_register#Galois_LFSRs
      return (v>>1)^(-(v&1)&0xB8);
    //count up by one
    case DAT_COUNT:
      return v+1;
    //unknown type return zero
    default:
      printf("Error : unknown type\r\n");
      return 0;
  }
}

//write LFSR pattern onto SD card sectors and read it back
int mmc_TstCmd(char **argv, unsigned short argc){
  int resp;
  char seed,*buffer=NULL;
  short lfsr;
  int j,count,tc,dat=DAT_LFSR,have_seed=0;
  unsigned long i,start,end;
  if(argc<2){
    printf("Error : Too few arguments\r\n");
    return 1;
  }
  //clear errno
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(argc>=3){
    //other arguments are optional
    for(i=3;i<=argc;i++){
      if(!strcmp(argv[i],"LFSR")){
        dat=DAT_LFSR;
      }else if(!strcmp(argv[i],"count")){
        dat=DAT_COUNT;
      }else if(!strncmp("seed=",argv[i],sizeof("seed"))){
        //parse seed
        seed=atoi(argv[i]+sizeof("seed"));
        have_seed=1;
      }else{
        printf("Error : unknown argument \"%s\".\r\n",argv[i]);
        return 3;
      }
    }
  }
  if(!have_seed){
    //seed LFSR from TAR
    seed=TAR;   //not concerned about correct value so don't worry about diffrent clocks
    //make sure seed is not zero
    if(seed==0){
      seed=1;
    }
    printf("seed = %i\r\n",(unsigned short)seed);
  }
  //check for error
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return 2;
  }
  //get buffer, set a timeout of 2 seconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //write to sectors
  for(i=start,lfsr=seed;i<=end;i++){
    //fill with psudo random data
    for(j=0;j<512;j++){
      buffer[j]=lfsr;
      //get next in sequence
      lfsr=dat_next(lfsr,dat);
    }
    //write data
    resp=mmcWriteBlock(i,(unsigned char*)buffer);
    if(resp!=MMC_SUCCESS){
      printf("Error : write failure for sector %i\r\nresp = 0x%04X\r\n%s\r\n",i,resp,SD_error_str(resp));
      //free buffer
      BUS_free_buffer();
      return -1;
    }
  }
  //read back sectors and check for correctness
  for(i=start,lfsr=seed,tc=0;i<=end;i++){
    //clear block data
    memset(buffer,0,512);
    //read data from card
    resp=mmcReadBlock(i,(unsigned char*)buffer);
    if(resp!=MMC_SUCCESS){
      printf("Error : read failure for sector %i\r\nresp = 0x%04X\r\n%s\r\n",i,resp,SD_error_str(resp));
      //free buffer
      BUS_free_buffer();
      return -1;
    }
    //compare to psudo random data
    for(j=0,count=0;j<512;j++){
      //print out the first few bytes
      if(buffer[j]!=lfsr){
        count++;
      }
      //get next in sequence
      lfsr=dat_next(lfsr,dat);
    }
    if(count!=0){
      printf("%i errors found in sector %i\r\n",count,i);
      tc+=count;
    }
  }
  if(tc==0){
    printf("All sectors read susussfully!\r\n");
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}

int mmc_multiWTstCmd(char **argv, unsigned short argc){
  unsigned char *ptr;
  int stat;
  unsigned long i,start,end;
  unsigned short multi=1;
  if(argc<2){
    printf("Error : too few arguments\r\n");
    return -1;
  }
  if(argc>3){
    printf("Error : too many arguments\r\n");
    return -2;
  }
  //get start and end
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return 2;
  }
  if(argc>2){
    if(!strcmp("single",argv[3])){
      multi=0;
    }else if(!strcmp("multi",argv[3])){
      multi=1;
    }else{
      //unknown argument
      printf("Error : unknown argument \"%s\".\r\n",argv[3]);
      return -3;
    }
  }
  if(!multi){
    //write each block in sequence
    for(i=start,ptr=NULL;i<end;i++,ptr+=512){
      if((stat=mmcWriteBlock(i,ptr))!=MMC_SUCCESS){
        printf("Error writing block %li. Aborting.\r\n",i);
        printf("%s\r\n",SD_error_str(stat));
        return 1;
      }
    }
  }else{
    //write all blocks with one command
    if((stat=mmcWriteMultiBlock(start,NULL,end-start))!=MMC_SUCCESS){
      printf("Error with write. %i\r\n",stat);
      printf("%s\r\n",SD_error_str(stat));
      return 1;
    }
  }
  printf("Data written sucussfully\r\n");
  return 0;
}

int mmc_multiRTstCmd(char **argv, unsigned short argc){
  unsigned char *ptr,*buffer;
  int resp;
  unsigned long i,start,end;
  unsigned short multi=1;
  if(argc<2){
    printf("Error : too few arguments\r\n");
    return -1;
  }
  if(argc>3){
    printf("Error : too many arguments\r\n");
    return -2;
  }
  //get start and end
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return -4;
  }
  if((end-start)*512>BUS_get_buffer_size()){
    printf("Error : data size too large for buffer.\r\n");
    return -5;
  }
  if(argc>2){
    if(!strcmp("single",argv[3])){
      multi=0;
    }else if(!strcmp("multi",argv[3])){
      multi=1;
    }else{
      //unknown argument
      printf("Error : unknown argument \"%s\".\r\n",argv[3]);
      return -3;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  if(!multi){
    //write each block in sequence
    for(i=start,ptr=buffer;i<end;i++,ptr+=512){
      if((resp=mmcReadBlock(i,ptr))!=MMC_SUCCESS){
        printf("Error reading block %li. Aborting.\r\n",i);     
        printf("%s\r\n",SD_error_str(resp));
        //free buffer
        BUS_free_buffer();
        return 1;   
      }
    }
  }else{
    //write all blocks with one command
    if((resp=mmcReadBlocks(start,end-start,buffer))!=MMC_SUCCESS){
      printf("Error with read.\r\n");
      printf("resp = 0x%04X\r\n%s\r\n",resp,SD_error_str(resp));
      //free buffer
      BUS_free_buffer();
      return 1;   
    }
  }
  //print out buffer
  for(i=0;i<(end-start)*512/16;i++){
    printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
    buffer[i*16],buffer[i*16+1],buffer[i*16+2],buffer[i*16+3],buffer[i*16+4],buffer[i*16+5],buffer[i*16+6],buffer[i*16+7],buffer[i*16+8],buffer[i*16+9],buffer[i*16+10],
    buffer[i*16+11],buffer[i*16+12],buffer[i*16+13],buffer[i*16+14],buffer[i*16+15]);
  }
  printf("Data read sucussfully\r\n");
  //free buffer
  BUS_free_buffer();
  return 0;
}

int mmc_reinit(char **argv, unsigned short argc){
  int resp;
  //setup the SD card
  resp=mmcReInit_card();
  //print status
  printf("%s\r\n",SD_error_str(resp));
  return 0;
}

//check if SD card SPI uses DMA
int mmcDMA_Cmd(char **argv, unsigned short argc){
  if(SD_DMA_is_enabled()){
    printf("DMA is enabled\r\n");
  }else{
    printf("DMA is not enabled\r\n");
  }
  return 0;
}

//read a register from the SD card
int mmcreg_Cmd(char**argv,unsigned short argc){
  unsigned char dat[16],reg;
  int resp;
  int i;
  //check number of arguments
  if(argc!=1){
    printf("Error : %s requires one argument\r\n",argv[0]);
    return -1;
  }
  //check register to read
  if(!strcmp("CSD",argv[1])){
    reg=0x40|9;
  }else if(!strcmp("CID",argv[1])){
    reg=0x40|10;
  }else{
    printf("Error : invalid register \"%s\"\r\n",argv[1]);
    return -2;
  }
  //read register
  resp=mmcReadReg(reg,dat);
  //check for success
  if(resp!=MMC_SUCCESS){
    printf("%s\r\n",SD_error_str(resp));
    return -3;
  }
  //print out contents
  for(i=0;i<16;i++){
    printf("%02X",dat[i]);
  }
  //add new line
  printf("\r\n");
  //return
  return 0;
}

int mmcdread_Cmd(char **argv, unsigned short argc){
  int resp; 
  char *buffer=NULL;
  unsigned long sector;
  unsigned int length,written;
  unsigned short check;
  //buffer size is not a multiple of 512, so find a size that is
  const unsigned short buffsize=512*(BUS_get_buffer_size()/512);
  int i;
  //check for arguments
  if(argc!=2){
    printf("Error : 2 arguments required but %i given.\r\n",argc);
  }else{
    //read sector
    if(1!=sscanf(argv[1],"%lu",&sector)){
      //print error
      printf("Error parsing sector \"%s\"\r\n",argv[1]);
      return -1;
    }
    //read size
    if(1!=sscanf(argv[2],"%u",&length)){
      //print error
      printf("Error parsing length \"%s\"\r\n",argv[2]);
      return -1;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //print sector 
  printf("Starting at MMC block %lu\r\n",sector);
  for(;;){
      for(i=0,check=0;i<length*512 && i<buffsize;i++){
          buffer[i]=getchar();
          check=check+buffer[i];
          //printf("i = %i\r\n",i);
          if(i%32==31){
              printf("%u\r\n",check);
              check=0;
          }
      }
      if(length==1){
        //number of blocks written
        written=1;
        //write single block
        resp=mmcWriteBlock(sector,(unsigned char*)buffer);
      }else{
        written=((length<(buffsize/512))?length:(buffsize/512));
        //write multiple blocks
        resp=mmcWriteMultiBlock(sector,(unsigned char*)buffer,written);
      }
      //printf("%s\r\n",SD_error_str(resp));
      //check if command was successful
      if(resp){
          printf("%s\r\n",SD_error_str(resp));
          //free buffer
          BUS_free_buffer();
          //return
         return resp;
      }
      //printf("Length = %u written %u\r\n",length,written);
      if(written>=length){
          //complete
          break;
      }
      length-=written;
      //printf("Length = %u written %u\r\n",length,written);
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}
