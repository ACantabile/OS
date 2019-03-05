#ifndef __CHKSUMS_H__
#define __CHKSUMS_H__

//--------------------------------------------------------------------------------------------------
//���ֽڼӷ�У��
extern unsigned int chksum_1byte_add(void *buf,unsigned int n,unsigned int chksum_org);
//--------------------------------------------------------------------------------------------------
//Modbus��CRC16����
extern unsigned short modbus_crc(void *puchMsg, unsigned short usDataLen);
extern unsigned short modbus_crc_org(unsigned short CRC_ORG,unsigned char *p,unsigned short length);

//����CRC��������Ҫ����512�ֽڵı��
extern unsigned short ModBus_FastCRC(void *puchMsg, unsigned short usDataLen);
extern unsigned short ModBus_FastCRC_ORG(unsigned short CRC_ORG,void *puchMsg, unsigned short usDataLen);
//--------------------------------------------------------------------------------------------------

#endif
