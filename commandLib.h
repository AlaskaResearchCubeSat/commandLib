#ifndef __COMMAND_LIB_H
#define __COMMAND_LIB_H

//structure for symbolic names
typedef struct{
  //symbolic name
  const char *name;
  //numeric address
  unsigned char addr;
}SYM_ADDR;

//define error constants
#define ERR_SRC_CMD             ERR_SRC_SUBSYSTEM
#define CMD_ERR_RESET           1

//symbolic names for I2C addresses
extern const SYM_ADDR busAddrSym[];
extern const SYM_ADDR magAddrSym[];
extern const SYM_ADDR tempAddrSym[];

//SD card commands
int mmcInitChkCmd(char**argv,unsigned short argc);
int mmc_write(char **argv, unsigned short argc);
int mmc_read(char **argv, unsigned short argc);
int mmc_dump(char **argv, unsigned short argc);
int mmcdat_Cmd(char **argv, unsigned short argc);
int mmc_cardSize(char **argv, unsigned short argc);
int mmc_eraseCmd(char **argv, unsigned short argc);
int mmc_TstCmd(char **argv, unsigned short argc);
int mmc_multiWTstCmd(char **argv, unsigned short argc);
int mmc_multiRTstCmd(char **argv, unsigned short argc);
int mmc_reinit(char **argv, unsigned short argc);
int mmcDMA_Cmd(char **argv, unsigned short argc);
int mmcreg_Cmd(char**argv,unsigned short argc);
int mmcdread_Cmd(char **argv, unsigned short argc);

#define MMC_INIT_CHECK_COMMAND        {"mmcinitchk","\r\n\t""Check if the SD card is initialized",mmcInitChkCmd}
#define MMC_WRITE_COMMAND             {"mmcw","[data,..]\r\n\t""write data to mmc card.",mmc_write}
#define MMC_READ_COMMAND              {"mmcr","\r\n\t""read string from mmc card.",mmc_read}
#define MMC_DUMP_COMMAND              {"mmcdump","[sector]\r\n\t""dump a sector from MMC card formatted as text.",mmc_dump}
#define MMC_DAT_COMMAND               {"mmcdat","[sector]\r\n\t""read a sector from MMC card and send out raw data. Best used with accompanying Matlab function.",mmcdat_Cmd}
#define MMC_CARD_SIZE_COMMAND         {"mmcsize","\r\n\t""get card size.",mmc_cardSize}
#define MMC_ERASE_COMMAND             {"mmce","start end\r\n\t""erase sectors from start to end",mmc_eraseCmd}
#define MMC_TEST_COMMAND              {"mmctst","start end [seed]\r\n\t""Test by writing to blocks from start to end.",mmc_TstCmd}
#define MMC_MW_TEST_COMMAND           {"mmcmw","start end [single|multi]\r\n\t""Multi block write test.",mmc_multiWTstCmd}
#define MMC_MR_TEST_COMMAND           {"mmcmr","start end [single|multi]\r\n\t""Multi block read test.",mmc_multiRTstCmd}
#define MMC_INIT_COMMAND              {"mmcinit","\r\n\t""initialize the mmc card the mmc card.",mmc_reinit}
#define MMC_DMA_COMMAND               {"mmcDMA","\r\n\t""Check if DMA is enabled.",mmcDMA_Cmd}
#define MMC_REG_COMMAND               {"mmcreg","[CID|CSD]\r\n\t""Read SD card registers.",mmcreg_Cmd}
#define MMC_DREAD_COMMAND             {"mmcdread","sector size\r\n\t""Read data from terminal into sector",mmcdread_Cmd}

//all SD card commands
//MMC_DREAD is not included because it requires __getchar to be defined
#define MMC_COMMANDS                  MMC_INIT_CHECK_COMMAND,MMC_WRITE_COMMAND,MMC_READ_COMMAND,MMC_DUMP_COMMAND,MMC_DAT_COMMAND,MMC_CARD_SIZE_COMMAND,MMC_ERASE_COMMAND,MMC_TEST_COMMAND,MMC_MW_TEST_COMMAND,MMC_MR_TEST_COMMAND,MMC_INIT_COMMAND,MMC_DMA_COMMAND,MMC_REG_COMMAND

//ARCbus commands
int restCmd(char **argv,unsigned short argc);
int timeCmd(char **argv,unsigned short argc);
int txCmd(char **argv,unsigned short argc);
int spiCmd(char **argv,unsigned short argc);
int asyncProxyCmd(char **argv,unsigned short argc);
int ARCsearch_Cmd(char **argv,unsigned short argc);

#define ARC_RESET_COMMAND           {"reset","\r\n\t""reset the msp430.",restCmd}
#define ARC_TIME_COMMAND            {"time","\r\n\t""Return current time.",timeCmd}
#define ARC_TX_COMMAND              {"tx"," [noACK] [noNACK] addr ID [[data0] [data1]...]\r\n\t""send data over I2C to an address",txCmd}
#define ARC_SPI_COMMAND             {"SPI","addr [len]\r\n\t""Send data using SPI.",spiCmd}
#define ARC_ASYNC_PROXY_COMMAND     {"async","[addr]\r\n\t""Open connection if address given. otherwise close connection.",asyncProxyCmd}
#define ARC_SEARCH_COMMAND          {"search","\r\n\t""Find devices on the bus",ARCsearch_Cmd}
#define ARC_SPI_DREAD               {"SPIdread","addr sector len""\r\n\t""Write data to SD card on another board",SPIdread_Cmd}
            
#define ARC_COMMANDS                 ARC_RESET_COMMAND,ARC_TIME_COMMAND,ARC_TX_COMMAND,ARC_SPI_COMMAND,ARC_SEARCH_COMMAND

//Crossworks Tasking Library commands
int priorityCmd(char **argv,unsigned short argc);
int timesliceCmd(char **argv,unsigned short argc);
int statsCmd(char **argv,unsigned short argc);
int stackCmd(char **argv,unsigned short argc);

#define CTL_PRIORITY_COMMAND        {"priority"," task [priority]\r\n\t""Get/set task priority.",priorityCmd}
#define CTL_TIMESLICE_COMMAND       {"timeslice"," [period]\r\n\t""Get/set ctl_timeslice_period.",timesliceCmd}
#define CTL_STATS_COMMAND           {"stats","\r\n\t""Print task status",statsCmd}
#define CTL_STACK_COMMAND           {"stack","\r\n\t""Print task stack status",stackCmd}
        
#define CTL_COMMANDS                CTL_PRIORITY_COMMAND,CTL_TIMESLICE_COMMAND,CTL_STATS_COMMAND,CTL_STACK_COMMAND

//ErrorLib commands
int replayCmd(char **argv,unsigned short argc);
int clearErrCmd(char **argv,unsigned short argc);
int logCmd(char **argv,unsigned short argc);

#define CLEAR_ERROR_COMMAND         {"clear","\r\n\t""Clear all saved errors on the SD card",clearErrCmd}
#define REPLAY_ERROR_COMMAND        {"replay","\r\n\t""Replay errors from log",replayCmd}
#define ERROR_LOG_LEVEL_COMMAND     {"log","[level]\r\n\t""get/set log level",logCmd}

#define ERROR_COMMANDS              CLEAR_ERROR_COMMAND,REPLAY_ERROR_COMMAND,ERROR_LOG_LEVEL_COMMAND

//I2C commands
int I2C_txCmd(char **argv,unsigned short argc);
int I2C_rxCmd(char **argv,unsigned short argc);
int I2C_txrxCmd(char **argv,unsigned short argc);
int I2C_scan_Cmd(char **argv,unsigned short argc);
        
#define I2C_TX_COMMAND              {"I2Ctx"," addr [[data0] [data1]...]\r\n\t""send data over I2C to an address",I2C_txCmd}
#define I2C_RX_COMMAND              {"I2Crx"," addr num\r\n\t""get num bytes of data over I2C from an addresed slave",I2C_rxCmd}
#define I2C_TXRX_COMMAND            {"I2Ctxrx"," addr num [[data0] [data1]...]\r\n\t""send data over I2C then recive num bytes",I2C_txrxCmd}
#define I2C_SCAN_COMMAND            {"I2Cscan","\r\nScan I2C bus for devices",I2C_scan_Cmd}
                
#define I2C_COMMANDS                I2C_TX_COMMAND,I2C_RX_COMMAND,I2C_TXRX_COMMAND,I2C_SCAN_COMMAND

//helper functions

//helper function to get an I2C address from a string
unsigned char getI2C_addr(const char *str,short res,const SYM_ADDR *syms);

//helper function to lookup addresses given a symbolic name
unsigned char I2C_addr_lookup(const char *str,const SYM_ADDR *syms);

//helper function to lookup symbolic name given an address
const char *I2C_addr_revlookup(unsigned char addr,const SYM_ADDR *syms);

#endif
  