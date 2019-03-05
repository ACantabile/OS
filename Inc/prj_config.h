#ifndef __PRJ_CONFIG_H__
#define __PRJ_CONFIG_H__

#define HVER "AZ01"
#define SVER "224"
//----------------------------------------------------------------------------------
//#define HARDWARE_FIRM
#define HARDWARE_DTU

#define DATA_POOL_SIZE 10240	//定义数据池大小
//----------------------------------------------------------------------------------
#ifdef HARDWARE_FIRM
	#define NET_BUF_SIZE 1024
	#define SCOMM_COM_RX_BUF_SIZE 1280
	#define SCOMM_COM_TX_BUF_SIZE 1280
#endif

#ifdef HARDWARE_DTU
	//#define NET_BUF_SIZE 512
	#define MQTT_BUF_SIZE 512
	#define SCOMM_COM_RX_BUF_SIZE (512+32)
	#define SCOMM_COM_TX_BUF_SIZE (512+32)
#endif

#define IAP_COMM_BUF_SIZE (2048+32)
#define FLASH_BUF_SIZE IAP_COMM_BUF_SIZE	//flash缓冲与HR共用，取大一些用于串口IAP
#define FLASK_BUF_N	2048	//满2K进行一次编程
#define MODBUS_HR_NB (IAP_COMM_BUF_SIZE/2)
//--------------------------
#define BOOTLOAD
//#define HARDWARE_LZ_DTU
//#define USE_A6
#define USE_SIM800C
	#ifdef USE_SIM800C
		//#define SIM800C_100
	#endif
//#define USE_M6312

#ifdef USE_A6
	#define USART_N		5		//定义串口个数
	#define AUTH_DATA_ADR 0x801f800
#endif

#ifdef USE_SIM800C
	#define USART_N		3		//定义串口个数
	#define AUTH_DATA_ADR 0x800f800
#endif

#ifdef HARDWARE_LZ_DTU
	#define USART_N		3		//定义串口个数
	#define AUTH_DATA_ADR 0x801f800
#endif
//--------------------------
//#define MANUAL_CTRL
//#define USE_DOG		//发行定义


#define MQTT_POINT_N	1
#define KEEP_ALIVE_TIMS	60
//------------------------------------------------------------------------
#define FIREM_REMAINDER REMAINDER2
//------------------------------------------------------------------------
#define APP_OFS		0x8000
#define IAP_ID_ADR_OFS  0x7f0
#define PSD_ADR_BAK_OFS	0x7f4
#define IAP_ID_ADR_SECTION_ADR  (Flash_Base_Adr + APP_OFS)
#define IAP_ID_ADR (Flash_Base_Adr + APP_OFS + IAP_ID_ADR_OFS)
#define STM32_PSD_ADR_BAK (Flash_Base_Adr + APP_OFS + PSD_ADR_BAK_OFS)

//#define STM32_PSD_ADR_ORG		0x8000600
#define STM32_PSD_CRC_ADR		0x8000602
#define STM32_PSD_ADR			0x8000604

//------------------------------------------------------------------------
#define STM32_FUNC_ADR_ORG		0x8000610
#define IAP_ID_ORG				0x5678
#define IAP_ID					(IAP_ID_ORG+1)

//------------------------------------------------------------------------
#endif
