#define PR_KEY_MAIN
#include "Pr_key.h"


void Pr_Key_Init(void)
{
	//��GPIO���г�ʼ����
}


void Pr_key8(void)		//�������ÿ10-20mS����һ��
{
	unsigned char input;
	//--------------------------------------------------
	//��ȡ����ԭʼ�˿���Ϣ
	input=0;
	if(!B_KEY0_IN)		//�͵�ƽ��Ч
	{
		input |= 0x01;
	}
	
	if(!B_KEY1_IN)
	{
		input |= 0x02;
	}
	
	if(!B_KEY2_IN)
	{
		input |= 0x04;
	}
	
	if(!B_KEY3_IN)
	{
		input |= 0x08;
	}
	
	if(!B_KEY4_IN)
	{
		input |= 0x10;
	}
	
	if(!B_KEY5_IN)
	{
		input |= 0x20;
	}
	
	if(!B_KEY6_IN)
	{
		input |= 0x40;
	}
	
	if(!B_KEY7_IN)
	{
		input |= 0x80;
	}
	//--------------------------------------------------
	key8_oper(&g_key,input);	//�԰������д���
	//--------------------------------------------------
}


//�����ʹ��
void xxxxx(void)
{
	unsigned char key_code;

	key_code=get_key8(&g_key);

	switch(key_code)
	{
		case KEY_CODE0:
			//����0�Ĳ���
			break;
		case KEY_CODE1:
			break;
		case KEY_CODE2:
			break;
		case KEY_CODE3:
			break;
		case KEY_CODE4:
			break;
		case KEY_CODE5:
			break;
		case KEY_CODE6:
			break;
		case KEY_CODE7:
			break;
		default:
			break;
	}
}
