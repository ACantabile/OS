#ifndef __CHKSUMS_H__
#define __CHKSUMS_H__

//--------------------------------------------------------------------------------------------------
//单字节加法校验
extern unsigned int chksum_1byte_add(void *buf,unsigned int n,unsigned int chksum_org);
//--------------------------------------------------------------------------------------------------
//Modbus的CRC16检验
extern unsigned short modbus_crc(void *puchMsg, unsigned short usDataLen);
extern unsigned short modbus_crc_org(unsigned short CRC_ORG,unsigned char *p,unsigned short length);

//快速CRC，但是需要消耗512字节的表格
extern unsigned short ModBus_FastCRC(void *puchMsg, unsigned short usDataLen);
extern unsigned short ModBus_FastCRC_ORG(unsigned short CRC_ORG,void *puchMsg, unsigned short usDataLen);
//--------------------------------------------------------------------------------------------------

#endif
