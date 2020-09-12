#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//��ֵ�ļ�
#include "const.h"
//���ݿ�����ļ�
#include "database.h"
//�ı������ļ�
#include "textHandler.h"
//ͼ��������ļ�
#include "graph.h"

int main(void)
{

	//��ʼ��DB
	DataBase DB={NULL,NULL};

	char buffer[BUF_SIZE+2];
	FILE *fpin;

	//ȡ��datapath
	char DATA_PATH[MAX_PATH_LENGTH];
	printf("Please input the path of data file or input \"skip\" to skip this step:\n");
	scanf("%s",DATA_PATH);
	if(strcmp(DATA_PATH,"skip"))
	{
		//������skip
		//�ļ�����
		if((fpin=fopen(DATA_PATH,"r+"))==NULL)return 1;
		fgets(buffer,BUF_SIZE,fpin);		//������һ�У��Ժ�����޸ģ�

		//������ʱ����
		ID_Type id_tmp;
		Equip_Status status_tmp=ON;
		char name_tmp[MAX_NAMESIZE];
		VexID_Type bus_tmp;
		VexID_Type bus2_tmp=-1;
		float X_tmp=0.0f;

		//�����µ�һ��
		fgets(buffer,BUF_SIZE,fpin);

		while (!feof(fpin))
		{
			//��ʼ��������ʱ����
			bus2_tmp=-1;	//��Ƕ�����豸����
			X_tmp=0.0f;

			//�ı�����������
			DataRecord_handler(buffer,&id_tmp,&status_tmp,name_tmp,&bus_tmp,&bus2_tmp,&X_tmp);
			if(bus2_tmp<0)Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp);
			else Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp,bus2_tmp,X_tmp);

			//�����µ�һ��
			fgets(buffer,BUF_SIZE,fpin);
		}

		fclose(fpin);
		printf("\"");
		printf(DATA_PATH);
		printf("\" has been imported.\nInput \"help\" for more info.\nPlease input your command:\n");
	}
	else printf("Input \"help\" for more info.\nPlease input your command:\n");
	gets(buffer);

	

	//�����������м���������ѭ������exitָ���˳���
	char op_path[MAX_PATH_LENGTH];	
	ISLAND_List IL={NULL,0,0};
	ISOE_List L;
	CrossList M;
	while(true)
	{
		//��ʾ�������ļ���ȡ����ģʽ�У��ļ���ȡ���������Ƕ�׵�executeָ�
		op_path[0]='\0';	
		//����ָ��
		printf(">>");
		gets(buffer);
		//ָ���ı�����
		Command_handler(DB,buffer,op_path,L,IL,M);
		if(op_path[0]!='\0')	//����ʾ���ڶ��ļ��е�ָ����յ��˳�ָ��
		{
			if(!strcmp(op_path,"exit"))break;
			else
			{
				if((fpin=fopen(op_path,"r+"))==NULL)
				{
					printf("ERROR:No such file\n");
					continue;
				}
				while(!feof(fpin))
				{
					fgets(buffer,BUF_SIZE,fpin);
					Command_handler(DB,buffer,op_path,L,IL,M);
				}
				fclose(fpin);
			}
		}
	}
	//������ؽṹ
	if(IL.base!=NULL)
	{
	DestoryCrossList(M);
	DestoryISLAND(IL);
	DestoryList(L,DB.EquipTree);
	}
	DestoryBiTree(DB.EquipTree);
	DestoryBiTree(DB.VexTree);
	return 0;
}

//20200909 0029 ����DB���� ��������
//20200909 0902 DataRecord_handler���� ��������
//20200909 1800 ָ������
//20200909 2024 �ļ�ָ����ԣ�����
//20200909 2118 ����������ȫʵ��
//20200910 1146 update��һ��ʵ�֣�����ͨ��
//20200911 1201 �������ٹ��ܡ����ע��