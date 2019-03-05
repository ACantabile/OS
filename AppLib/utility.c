#define UTILITY_MAIN
#include "utility.h"
#include "string.h"

//------------------------------------------------------------------------------
//�����ֽںϳ�һ������
unsigned short char_hl_short(unsigned char hi,unsigned lo)
{
	union{
	unsigned char ch[2];
	unsigned short s;
	}u;
	u.ch[0]=lo;
	u.ch[1]=hi;
	return(u.s);
}
//------------------------------------------------------------------------------
//����һ���ֵĸߡ����ֽ�
/*
unsigned short short_xch_hl(unsigned short i)
{
	union{
	unsigned char ch[2];
	unsigned short s;
	}u;
	u.ch[0]=(i>>8) & 0xff;
	u.ch[1]=i & 0xff;
	return(u.s);
}
*/
//------------------------------------------------------------------------------
//��Դ����������n��short��Ŀ�Ļ�������b_xch=1,�����ߵ��ֽ�
void short_copy_xch(void *t,void *s,int n,unsigned char b_xch)
{
	int i;
	unsigned char *pt,*ps0;
	unsigned char ch;
	
	pt=(unsigned char *)t;
	ps0=(unsigned char *)s;
		
	if(b_xch)
	{
		for(i=0;i<n;i++)
		{
			ch=*ps0++;
			*pt++=*ps0++;
			*pt++=ch;
		}
	}
	else
	{
		n*=2;
		for(i=0;i<n;i++)
		{
			*pt++=*ps0++;
		}
	}
	
}
//------------------------------------------------------------------------------
//shortд�뻺����
unsigned char * short_wr_buf(unsigned char *buf,unsigned short s)
{
	*buf++ = s;
	*buf++ = s>>8;
	return(buf);
}
//����������short
unsigned short short_rd_buf(unsigned char *buf)
{
	return((unsigned short)(*buf) + ((unsigned short)*(buf+1)<<8));
}
//------------------------------------------------------------------------------
//short�����ߵ��ֽں�д�뻺����
unsigned char * short_wr_buf_xch(unsigned char *buf,unsigned short s)
{
	*buf++ = s>>8;
	*buf++ = s;
	return(buf);
}
//�����������ߵ��ֽڶ���short
unsigned short short_rd_buf_xch(unsigned char *buf)
{
	return(((unsigned short)(*buf)<<8) + (unsigned short)*(buf+1));
}
//------------------------------------------------------------------------------
void jump_to_app(unsigned int ApplicationAddress)
{
	Function_T Jump_To_Application;
	
	Jump_To_Application = (Function_T)(*(volatile unsigned int*)(ApplicationAddress + 4));
	__asm("MSR MSP, *(volatile unsigned int*) ApplicationAddress");		//ARM��ջ�ڸ�λ��ַ
	
	Jump_To_Application();
}
//------------------------------------------------------------------------------


//����λ��ַ
unsigned long * calc_bitadr(void *x,unsigned char b)
{
	unsigned long y;

	y=((unsigned long)x) & 0xf0000000;
	y |= 0x02000000;
	y |= ((unsigned long)x & 0x000FFFFF) << 5;
	y |= b <<2;
	return((unsigned long *)y);
}
//---------------------------------------------------------
/*
void delay_us_set_k(unsigned long us)
{
	unsigned long i;


	
	i=HAL_RCC_GetSysClockFreq();
	delay_k=i/3000000;
	delay_us(us);
}
*/

void delay_us(unsigned long us)
{
	unsigned long i;
	us=us*delay_k;

	if(us>3)
	{
		us-=3;
	}
	for(i=0;i<us;i++)	//ÿ��ѭ��3������
	{
		;
	}
}
//---------------------------------------------------------
	//�趨��ʱ���ʱ��ο���
//void Tout_CP_set(Tout_CP_T *p,Tout_ms_T val)
void left_ms_set(time_ms_T *p,time_ms_T val)
{
	*p = cnt_1ms+val;
}


unsigned long left_ms(time_ms_T *p)
{
	unsigned long i,j;
	i=cnt_1ms;
	
	j=*p-i;
	if(j & 0x80000000)
	{
		*p=i;
		return(0);
	}
	else
	{
		return(j);
	}
}
//-----------------------------------------------------------------------------------
//��json��ʽ�ַ���������Ҹ�ʽΪ"name":str2����"str1":"str2"����str2������buf
//���᣺>0�ɹ�,=0���ɹ�
int get_json_1fs(char *buf,int buf_size,char *s,char *name)
{
	char *p0,*p1,*pe;
	int n;
	//-----------------------------------------------------------
	//�ж��ַ�����ǰ���ǲ���{}
	n=strlen(s);				//���ַ�������
	if(n<7)return(GET_JSON_1FS_FAIL);			//{"A":1} һ��json����7���ַ�
	if(*s != '{')return(GET_JSON_1FS_FAIL);	//��һ���ַ�����'{'
	pe=s+n-1;					//ָ�����һ���ַ�
	if(*pe!='}')return(GET_JSON_1FS_FAIL);		//���һ���ַ�����'}'
	//-----------------------------------------------------------
	p0=strstr(s,name);			//���ַ�������ֶ���
	if(p0==NULL)return(GET_JSON_1FS_FAIL);		//û�ҵ�name
	if(*(p0-1)!='"')return(GET_JSON_1FS_FAIL);	//û�ҵ�nameǰ���'"'
	
	n=strlen(name);		//name�ĳ���
	
	p0+=n;				//ָ��name��"
	
	if(*p0 != '"')return(GET_JSON_1FS_FAIL);	//û�ҵ�"
	
	p0++;				//ָ��':'
	if(*p0 != ':')return(GET_JSON_1FS_FAIL);	//û�ҵ�':'
	
	p0++;	//����':'
	
	while(*p0==' ')
	{
		if(*p0==0)
		{
			return(GET_JSON_1FS_FAIL);
		}
		p0++;			//����':'֮��Ŀո�
	}
	
	if(*p0=='"')
	{				//�ַ�������
		p0++;		//����'"'
		p1=strstr(p0,"\"");	//����һ��'"'
		if(p1==NULL)return(GET_JSON_1FS_FAIL);	//�Ҳ���
		n=p1-p0;		//��¼�ַ�������
		
		p1++;		//ָ���ֶηָ����������
		if((*p1 !=',') && (*p1 != '}'))return(GET_JSON_1FS_FAIL);	//û�ҵ��ָ����������
	}
	else
	{				//��������
		p1=strstr(p0,",");
		if(p1==NULL)	//û���ҵ��ֶηָ���
		{
			p1=strstr(p0,"}");
			if(p1==NULL)return(GET_JSON_1FS_FAIL);	//û���ҵ���ֹ��
			if(*p1=='}')				//�ҵ���ֹ��
			{
				if(p1!=pe)return(GET_JSON_1FS_FAIL);	//��ֹ��λ�ò���ȷ
			}
		}
		n=p1-p0;
	}
	
	
	if(n>=buf_size)
	{
		return(GET_JSON_1FS_FAIL);	//�ַ���̫��
	}
	
	strncpy(buf,p0,n);
	buf[n]=0;
	return(n);
}
//-----------------------------------------------------------------------------------
int hexchar_btye(unsigned char ch)
{
	if(ch >='0' && ch<='9')
	{
		ch-='0';
	}
	else if(ch >= 'a' && ch <= 'f') 
	{
		ch-=('a'-10);
	}
	else if(ch >= 'A' && ch <= 'F')
	{
		ch-=('A'-10);
	}
	else
	{
		return(-1);
	}
	return(ch);
}

int str_to_hex(unsigned char *buf,unsigned int buf_size,char *s)
{
	int i,n,j;
	unsigned char ch;
	
	n=strlen(s);
	if(n & 0x01)
	{
		return(STR_TO_HEX_FAIL);
	}
	
	n/=2;
	
	if(n>buf_size)
	{
		return(STR_TO_HEX_FAIL);
	}
	
	for(i=0;i<n;i++)
	{
		j=hexchar_btye(*s++);
		if(j<0)
		{
			return(STR_TO_HEX_FAIL);
		}
		ch=j << 4;
		j=hexchar_btye(*s++);
		if(j<0)
		{
			return(STR_TO_HEX_FAIL);
		}
		ch |= j;
		*buf++ = ch;
	}
		
	return(n);
}

void hex_to_str(char *str_buf,unsigned char *hex_buf,unsigned int hex_n)
{
	int i;
	unsigned char ch,ch1;
	for(i=0;i<hex_n;i++)
	{
		ch1=*hex_buf++;
		ch =ch1>>4;
		if(ch<10)
		{
			ch+='0';
		}
		else
		{
			ch+=('A'-10);
		}
			
		*str_buf++ = ch;
		
		ch = ch1 & 0x0f;
		if(ch<10)
		{
			ch+='0';
		}
		else
		{
			ch+=('A'-10);
		}
			
		*str_buf++ = ch;
	}
}
//-------------------------------------------------------------------------
const unsigned char tb_month_days[]={
	0,
	31,		//1
	28,		//2
	31,		//3
	30,		//4
	31,		//5
	30,		//6
	31,		//7
	31,		//8
	30,		//9
	31,		//10
	30,		//11
	31};	//12
#define TIME_DATA_YEAR_ORG 1970
#define SECONDS_DAY (3600*24)
#define SECONDS_YEAR (3600*24*365)
#define DAYS_YEAR 365

int data_time_int(TIME_DATE_T *p)
{
	unsigned int i,j,k;
	unsigned char b_leap;
	unsigned int days;
	
	
	i=p->year;
	if(i < TIME_DATA_YEAR_ORG)
	{
		return(TIME_DATA_FAIL);
	}
	
	b_leap=(((i%400)==0) || ((i%4==0) && (i%100!=0)));	//��������Ƿ�����
	
	j=(i-TIME_DATA_YEAR_ORG);	//�Ѿ���ȥ������
	days=DAYS_YEAR*j;			//�Ѿ���ȥ������
	j/=4;	  					//�Ѿ���ȥ����������2000��պ�������
	days += j;					//�Ѿ���ȥ������
	
	i=p->month;
	
	if(i<1 || i>12)
	{
		return(TIME_DATA_FAIL);
	}
	
	k=0;
	for(j=1;j<i;j++)
	{
		k += (unsigned int)tb_month_days[j];
	}
	
	if(i >= 3 )
	{
		k+=b_leap;		//���ڵ���3��
	}
	days += k;			//�Ѿ���ȥ������
	
	
	i=p->day;
	if(i<1 || i>((unsigned int)tb_month_days[p->month]+b_leap))
	{
		return(TIME_DATA_FAIL);
	}
	
	days += (i-1);		//�Ѿ���ȥ������
	k = days * 24;		//�Ѿ���ȥ��Сʱ
	
	i = p->hour;
	if(i>23)
	{
		return(TIME_DATA_FAIL);
	}
	
	k += i;
	k *= 60;			//�Ѿ���ȥ�ķ���
	
	i = p->min;
	if(i>59)
	{
		return(TIME_DATA_FAIL);
	}
	
	k += i;
	k *= 60;			//�Ѿ���ȥ����
	
	i = p->sec;
	if(i>59)
	{
		return(TIME_DATA_FAIL);
	}
	
	k += i;				//�Ѿ���ȥ����
	
	p->sec1970 = k;

	return(TIME_DATA_SUCCESS);
}

void int_date_time(TIME_DATE_T *p)
{
	unsigned int i,j,k;
	unsigned int days;
	unsigned char b_leap;
	
	i=p->sec1970;
	
	days = i/SECONDS_DAY;		//��1970.1.1һ��������
	
	j=days/365;					//j=��1970��
	
	if((365*j + j/4) > days)	//����*365+�м������
	{
		j--;					//�������࣬��1��
	}
	k=1970+j;
	p->year=k;					//ȷ�����
	
	b_leap=(((k%400)==0) || ((k%4==0) && (k%100!=0)));	//��������Ƿ�����
	
	days -= (j*365 + j/4);		//ʣ�µ�����
	
	k=0;
	for(i=1;i<=12;i++)
	{
		j= k + (unsigned int)tb_month_days[i];
		if(b_leap && i==2)
		{
			j++;
		}
		if(j > days)
		{
			break;
		}
		k=j;
	}
	p->month = i;		//ȷ���·�

	p->day=days-k+1;	//ȷ����
	
	i=(p->sec1970) % SECONDS_DAY;
	
	p->hour = i / 3600;
	
	i %=3600;
	
	p->min = i/60;
	
	p->sec = i%60;
}
//-------------------------------------------------------------------------
//ch:�ַ���ָ�룬dada�����ݣ�i_nb������λ����dec_nb:С��λ����b_sign���з�����,b_inv0:��ʾ��Ч��
int Dword2Str(char *s,unsigned long data,unsigned char i_nb,unsigned char dec_nb,unsigned char b_sign,unsigned char b_inv0)
{
	int i,j,char_nb;
	//char ch[16];
	char *p;

	if(!i_nb)i_nb++;	
	char_nb=i_nb;	//����λ��
	if(dec_nb)
	{
		char_nb+=1+dec_nb;	//С��λ��+С����λ
	}

	if(b_sign)
	{
		if((long)data<0)
		{
			data=0-data;
		}
		else
		{
			b_sign=0;
		}
	}

	p=s+char_nb+1;	//+1������λ��
	*p--=0;

	for(i=0;i<char_nb;i++)
	{
		if(i!=0 && i==dec_nb)
		{
			*p--='.';
		}
		else
		{
			j=data%10;
			data/=10;
			*p--=j+'0';
		}
	}
	
	p++;
	if(!b_inv0 && i_nb)
	{		   	//��Ч�㲻��ʾ
		for(i=0;i<i_nb-1;i++)
		{
			if(*p=='0')
			{
				*p=' ';
			}
			else
			{
				break;
			}
			p++;
		}
	}
	
	p--;
	if(b_sign)
	{
		*p='-';
	}
	else
	{
		*p=' ';
	}

	strcpy(s,p);

	return(1);
}


//=========================================================================================
//16λ�����ն���������ת�����ַ���
void Short_BinStr(char *buf,unsigned short us)
{
	int i,j;
	
	j=0x8000;
	for(i=0;i<16;i++)
	{
		if(us & j)
		{
			*buf='1';
		}
		else
		{
			*buf='0';
		}
		buf++;
		j>>=1;
	}
	*buf=0;		//�ַ�����β
}
//=========================================================================================
void str2lwr(char *p)
{
	while(1)
	{
		if(*p==0)
		{
			break;
		}
		if(*p>='A' && *p<='Z')
		{
			*p-=('A'-'a');
		}
		p++;
	}
}

void str2upr(char *p)
{
	while(1)
	{
		if(*p==0)
		{
			break;
		}
		if(*p>='a' && *p<='z')
		{
			*p+=('A'-'a');
		}
		p++;
	}
}
//=========================================================================================
//�����ݳ�������ռ䣬�ɹ�����ָ��
unsigned char * DataPool_Get(unsigned int size)
{
	unsigned char *r=NULL;
	
	if(size > (g_p_data_pool->size-g_p_data_pool->used_size))
	{
		//g_p_data_pool->used_size=0xffffffff;
		while(1);
	}
	r=g_p_data_pool->buf + g_p_data_pool->used_size;
	g_p_data_pool->used_size += size;
	return(r);
}
//=========================================================================================
//�Ӻ������ݱ�������������ƥ��ĺ���ָ��
unsigned int Get_Func(FUNC_TB_T *func_tb,const char *func_name)
{
	while(func_tb->func_adr !=0)
	{
		if(strcmp(func_tb->func_name,func_name)==0)
		{
			return(func_tb->func_adr);
		}
		func_tb++;
	}
	return(0);
}
//=========================================================================================
int digchar_btye(unsigned char ch)
{
	if(ch >='0' && ch<='9')
	{
		ch-='0';
	}
	else
	{
		return(-1);
	}
	return(ch);
}

int asc2int_dft(char *p,int dft)
{
	unsigned char ch,b_neg=0;
	int i,j;
	unsigned int r;
	r=0;
	
	if(*p=='-')
	{
		p++;
		b_neg=1;
	}
	for(i=0;i<10;i++)
	{
		ch=*p++;
		if(ch==0)
		{
			break;
		}
		j=digchar_btye(ch);
		if(j<0)
		{
			return(dft);	//�Ƿ��ַ�����ʧ��
		}
		r=r*10+j;		//�Ϸ��ַ�
	}
	if(r>=0x80000000)
	{
		return(0);		//�������ͱ�ﷶΧ
	}
	if(b_neg)
	{
		r=-r;		//����
	}
	
	return(r);
}

S64_T asc2s64_dft(char *p,S64_T dft)
{
	unsigned char ch,b_neg=0;
	int i,j;
	U64_T r;
	r=0;
	
	if(*p=='-')
	{
		p++;
		b_neg=1;
	}
	for(i=0;i<19;i++)
	{
		ch=*p++;
		if(ch==0)
		{
			break;
		}
		j=digchar_btye(ch);
		if(j<0)
		{
			return(dft);	//�Ƿ��ַ�����ʧ��
		}
		r=r*10+j;		//�Ϸ��ַ�
	}
	if(r>=0x8000000000000000)
	{
		return(0);		//�������ͱ�ﷶΧ
	}
	if(b_neg)
	{
		r=-r;		//����
	}
	
	return(r);
}
//==============================================================================
int asc2int(char *p,int *resault)
{
	unsigned char ch,b_neg=0;
	int i,j;
	unsigned int r=0;
	
	if(*p=='-')
	{
		p++;
		b_neg=1;
	}
	for(i=0;i<10;i++)
	{
		ch=*p++;
		if(ch==0)
		{
			break;
		}
		j=digchar_btye(ch);
		if(j<0)
		{
			*resault=ASC2INT_FAIL;
			return(0);	//�Ƿ��ַ�����ʧ��
		}
		r=r*10+j;		//�Ϸ��ַ�
	}
	if(r>=0x80000000)
	{
		*resault=ASC2INT_FAIL;
		return(0);		//�������ͱ�ﷶΧ
	}
	if(b_neg)
	{
		r=-r;		//����
	}
	*resault=ASC2INT_SUCCESS;
	return(r);
}

S64_T asc2s64(char *p,int *resault)
{
	unsigned char ch,b_neg=0;
	int i,j;
	U64_T r;
	r=0;
	
	if(*p=='-')
	{
		p++;
		b_neg=1;
	}
	for(i=0;i<19;i++)
	{
		ch=*p++;
		if(ch==0)
		{
			break;
		}
		j=digchar_btye(ch);
		if(j<0)
		{
			*resault=ASC2INT_FAIL;
			return(0);	//�Ƿ��ַ�����ʧ��
		}
		r=r*10+j;		//�Ϸ��ַ�
	}
	if(r>=0x8000000000000000)
	{
		*resault=ASC2INT_FAIL;
		return(0);		//�������ͱ�ﷶΧ
	}
	if(b_neg)
	{
		r=-r;		//����
	}
	*resault=ASC2INT_SUCCESS;
	return(r);
}

