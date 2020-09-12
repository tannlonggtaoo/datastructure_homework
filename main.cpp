#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//常值文件
#include "const.h"
//数据库相关文件
#include "database.h"
//文本解析文件
#include "textHandler.h"
//图计算相关文件
#include "graph.h"

int main(void)
{

	//初始化DB
	DataBase DB={NULL,NULL};

	char buffer[BUF_SIZE+2];
	FILE *fpin;

	//取得datapath
	char DATA_PATH[MAX_PATH_LENGTH];
	printf("Please input the path of data file or input \"skip\" to skip this step:\n");
	scanf("%s",DATA_PATH);
	if(strcmp(DATA_PATH,"skip"))
	{
		//若不是skip
		//文件环境
		if((fpin=fopen(DATA_PATH,"r+"))==NULL)return 1;
		fgets(buffer,BUF_SIZE,fpin);		//读掉第一行（以后可以修改）

		//定义临时变量
		ID_Type id_tmp;
		Equip_Status status_tmp=ON;
		char name_tmp[MAX_NAMESIZE];
		VexID_Type bus_tmp;
		VexID_Type bus2_tmp=-1;
		float X_tmp=0.0f;

		//读入新的一行
		fgets(buffer,BUF_SIZE,fpin);

		while (!feof(fpin))
		{
			//初始化部分临时变量
			bus2_tmp=-1;	//标记读入的设备种类
			X_tmp=0.0f;

			//文本解析、插入
			DataRecord_handler(buffer,&id_tmp,&status_tmp,name_tmp,&bus_tmp,&bus2_tmp,&X_tmp);
			if(bus2_tmp<0)Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp);
			else Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp,bus2_tmp,X_tmp);

			//读入新的一行
			fgets(buffer,BUF_SIZE,fpin);
		}

		fclose(fpin);
		printf("\"");
		printf(DATA_PATH);
		printf("\" has been imported.\nInput \"help\" for more info.\nPlease input your command:\n");
	}
	else printf("Input \"help\" for more info.\nPlease input your command:\n");
	gets(buffer);

	

	//以下是命令行监听程序（主循环）（exit指令退出）
	char op_path[MAX_PATH_LENGTH];	
	ISLAND_List IL={NULL,0,0};
	ISOE_List L;
	CrossList M;
	while(true)
	{
		//表示不处于文件读取命令模式中（文件读取命令将不处理嵌套的execute指令）
		op_path[0]='\0';	
		//读入指令
		printf(">>");
		gets(buffer);
		//指令文本解析
		Command_handler(DB,buffer,op_path,L,IL,M);
		if(op_path[0]!='\0')	//即表示正在读文件中的指令或收到退出指令
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
	//销毁相关结构
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

//20200909 0029 建立DB测试 运行良好
//20200909 0902 DataRecord_handler测试 运行良好
//20200909 1800 指令正常
//20200909 2024 文件指令测试，正常
//20200909 2118 基础功能完全实现
//20200910 1146 update第一步实现，测试通过
//20200911 1201 新增销毁功能、添加注释