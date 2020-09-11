#pragma once
//文本解析函数
//strtok不支持中文，则手动实现
#include <stdio.h>
#include <string.h>
#include "const.h"
#include "DBpart_C.h"
#include "graph.h"
//减少调用函数的开销
#define nextTok(buf,tok)		\
	{                           \
	while(buf[tok]!='\0')tok++; \
	while(buf[tok]=='\0')tok++; \
	}
#define strMatch(buf,tok,str) (!strcmp(&buf[tok],str))
#define preProcess(buf,tok)													\
	{                                                                       \
	for(tok=0;buf[tok]!='\0';tok++)											\
		{                                                                   \
		if(buf[tok]=='\t'||buf[tok]==' '||buf[tok]=='\n')                   \
		buf[tok]='\0';														\
		}                                                                   \
	buffer[token+1]='$';													\
	token=0;																\
}
//对[域][值][域][值]...格式的串进行处理
Status DataRecord_handler(char *buffer,ID_Type* id_tmp,Equip_Status* status_tmp,char* name_tmp,VexID_Type* bus_tmp,VexID_Type* bus2_tmp,float* X_tmp)
{
	int token;
	preProcess(buffer,token);
	while(buffer[token]=='\0')token++;	//移到第一个非零元
	while(buffer[token]!='$')
	{
		switch (buffer[token])		//比调用函数的开销小
		{
		case 'F':
		case 'A':
			if(strMatch(buffer,token,"FROM")||strMatch(buffer,token,"AT"))
			{
				nextTok(buffer,token);
				sscanf(&buffer[token],"%d",bus_tmp);
				break;
			}
		case 'T':
			if(strMatch(buffer,token,"TO"))
			{
				nextTok(buffer,token);
				sscanf(&buffer[token],"%d",bus2_tmp);
				break;
			}
		case 'S':
			if(strMatch(buffer,token,"STATUS"))
			{
				nextTok(buffer,token);
				sscanf(&buffer[token],"%d",status_tmp);
				break;
			}
		case 'x':
			if(strMatch(buffer,token,"x"))
			{
				nextTok(buffer,token);
				sscanf(&buffer[token],"%f",X_tmp);
				break;
			}
		case 'I':
			if(buffer[token+1]='D')
			{
				sscanf(&buffer[token],"ID=%d",id_tmp);
				nextTok(buffer,token);
				sscanf(&buffer[token],"%s",name_tmp);
				break;
			}
		default:
			if(1==sscanf(&buffer[token],"%d",id_tmp))
			{
				nextTok(buffer,token);
				sscanf(&buffer[token],"%s",name_tmp);
				break;
			}
			//未在上面break，则未匹配成功
			else return ERROR;
		}
		nextTok(buffer,token);
	}
	return OK;
}

//为节点的重边定制的缓存区
typedef struct {
	ID_Type* base;	// 线性表首地址
	int listsize;		// 分配的存储空间
	int length;		// 线性表长度
}BusBuf;
Status InitBusBuf(BusBuf& L) {
  // 构造一个空的顺序线性表L。
	L.base = (ID_Type*)malloc(BUSBUF_SIZE*sizeof(ID_Type));
	if (L.base==NULL) return ERROR;        // 存储分配失败
	L.length = 0;
	L.listsize = BUSBUF_SIZE;
	return OK;
} 
Status InsertBusBuf(BusBuf& L,ID_Type id)	//只在尾巴上插入
{
	if(L.length>=L.listsize)
	{
		//超出容量限制，则重新分配内存
		ID_Type *newbase=(ID_Type*)realloc(L.base,(L.listsize+BUSBUF_SIZE)*sizeof(ID_Type));
		if(!newbase)return ERROR;
		L.base=newbase;
		L.listsize+=BUSBUF_INC;
	}
	L.base[L.length]=id;
	L.length++;
	return OK;
}
Status DeleteBusBuf(BusBuf& L,int i)		//根据指定的秩i删除一条记录
{
	if(i<1||i>L.length||L.length==0)return ERROR;
	int idx;
	for(idx=i-1;idx<=L.length-1;idx++)
	{
		L.base[idx]=L.base[idx+1];
	}
	L.length--;
	return OK;
}
bool inBusBuf(BusBuf &L,ID_Type id_to_find)
{
	for (int i = 1; i <= L.length; i++)
	{
		if(id_to_find==L.base[i-1])
		{
			DeleteBusBuf(L,i);
			return true;
		}
	}
	return false;
}
Status DestoryBusBuf(BusBuf& L)	//归还空间
{
	free(L.base);
	L.base=NULL;
	return OK;
}

//命令识别处理
Status Command_handler(DataBase &DB,char* buffer,char* path,ISOE_List &L,ISLAND_List &IL,CrossList &M)
{
	int token=0;
	//定义临时变量
	ID_Type id_tmp=0,id_old=0;
	Equip_Status status_tmp=UNKNOWN;
	char name_tmp[MAX_NAMESIZE]="DEFAULT";
	VexID_Type bus_tmp=0;
	VexID_Type bus2_tmp=-1;
	float X_tmp=0.0f;

	//返回状态
	Status cmdStatus;
	//移到第一个非零元
	while(buffer[token]=='\0'||buffer[token]==' '||buffer[token]=='\t')token++;	

	switch (buffer[token])
	{
	case 'g':
		preProcess(buffer,token);
		if(strMatch(buffer,token,"get"))
		{
			nextTok(buffer,token);
			switch (buffer[token])
			{
			case 'a':
				if(strMatch(buffer,token,"artic"))
				{
					for(int idx=1;idx<=IL.length;idx++)
					{
						if(IL.base[idx-1].isoc_num>2)		//单点、二点非割点
						{
							FindArticul(L,IL.base[idx-1].isoc_num,IL.base[idx-1].pHead->isoe_idx);
						}
					}
					printf("\n");
					cmdStatus=OK;
					break;
				}
				else cmdStatus=ERROR;
				break;
			case 'i':
				if(strMatch(buffer,token,"island"))
				{
					int island_idx;
					nextTok(buffer,token);
					sscanf(&buffer[token],"%d",&island_idx);
					ISLAND_ChainHead head;
					GetISLAND(IL,island_idx,head);
					ISLAND_ChainNode* pNode=head.pHead;
					while(pNode!=NULL)
					{
						printf("%d ",pNode->isoe_idx);
						pNode=pNode->next;
					}
					printf("\n");
					cmdStatus=OK;break;
				}
				else cmdStatus=ERROR;break;
			case 'b':
				if(strMatch(buffer,token,"bus"))
				{
					ID_Type id;
					nextTok(buffer,token);
					sscanf(&buffer[token],"%d",&id);
					Node* pNode=FindAVL_ID(DB.EquipTree,id);
					if(pNode==NULL){cmdStatus=ERROR;break;}
					switch(((ElemType_Deg1*)pNode->data)->type)
					{
					case DEG1:
						printf("%d\n",((ElemType_Deg1*)pNode->data)->bus->isoe);
						cmdStatus=OK;break;
					case DEG2:
						if(((ElemType_Deg2*)pNode->data)->bus_A->isoe!=((ElemType_Deg2*)pNode->data)->bus_B->isoe)
							printf("%d %d\n",
							((ElemType_Deg2*)pNode->data)->bus_A->isoe,((ElemType_Deg2*)pNode->data)->bus_B->isoe);
						else
							printf("%d\n",((ElemType_Deg2*)pNode->data)->bus_A->isoe);

						cmdStatus=OK;break;
					default:cmdStatus=ERROR;break;
					}
				}
				else cmdStatus=ERROR;break;
			case 'e':
				if(strMatch(buffer,token,"equip"))
				{
					int isoe_idx;
					nextTok(buffer,token);
					sscanf(&buffer[token],"%d",&isoe_idx);
					ISOE_Heads heads;
					GetList(L,isoe_idx,heads);
					ISOE_ChainNode *pNode=heads.ChainHead;
					ChainNode* pChain;
					AMListNode* pAMList;
					//构造重边缓存
					BusBuf L;
					InitBusBuf(L);
					while(pNode!=NULL)
					{
						pChain=((ElemType_Vex*)pNode->pVex)->Deg1_Equips;
						pAMList=((ElemType_Vex*)pNode->pVex)->Deg2_Equips;
						while(pChain!=NULL)
						{
							printf("%d\n",pChain->id);
							pChain=pChain->next;
						}
						while(pAMList!=NULL)
						{
							if(!inBusBuf(L,pAMList->id))
							{
								printf("%d\n",pAMList->id);
								InsertBusBuf(L,pAMList->id);
							}
							if(pAMList->vex_A==((ElemType_Vex*)pNode->pVex))
							{
								pAMList=pAMList->link_A;
							}
							else if(pAMList->vex_B==((ElemType_Vex*)pNode->pVex))
							{
								pAMList=pAMList->link_B;
							}
						}
						pNode=pNode->next;
					}
					DestoryBusBuf(L);
					cmdStatus=OK;
					break;
				}
				else cmdStatus=ERROR;break;
			case 'l':
				if(strMatch(buffer,token,"line"))
				{
					int isoe_i,isoe_j;
					nextTok(buffer,token);
					sscanf(&buffer[token],"%d",&isoe_i);
					nextTok(buffer,token);
					sscanf(&buffer[token],"%d",&isoe_j);
					//到isoe图找答案,顺着i找j
					ISOE_AMListNode* pEdge=L.base[isoe_i-1].AMListHead;
					bool none_flag=true;
					while (pEdge!=NULL)
					{
						if(pEdge->isoe_A==isoe_j||pEdge->isoe_B==isoe_j)
						{
							none_flag=false;
							printf("%d ",pEdge->pwrline->id);
						}
						pEdge=pEdge->isoe_A==isoe_i?pEdge->link_A:pEdge->link_B;
					}
					if(none_flag)printf("NULL");
					printf("\n");
					cmdStatus=OK;break;
				}
				else cmdStatus=ERROR;break;
			case 'Y':
				nextTok(buffer,token);
				int isoe_i,isoe_j;
				sscanf(&buffer[token],"%d",&isoe_i);
				nextTok(buffer,token);
				sscanf(&buffer[token],"%d",&isoe_j);
				float mat_elem;
				Get_symm(M,isoe_i,isoe_j,mat_elem);
				printf("%f\n",mat_elem);
				cmdStatus=OK;break;
			default:cmdStatus=ERROR;break;
			}
		}
		else cmdStatus=ERROR;
		break;
	case 'u':
		preProcess(buffer,token);
		while(buffer[token]=='\0'||buffer[token]==' '||buffer[token]=='\t')token++;	
		if(strMatch(buffer,token,"update"))
		{
			nextTok(buffer,token);
			if(strMatch(buffer,token,"model"))
			{
			if(IL.base!=NULL)
			{
			//step0,把旧的结构删除
			DestoryCrossList(M);
			DestoryISLAND(IL);
			DestoryList(L);
			}
			//step1
			Update_step1(DB,L);
			//step2,构造ISLAND表
			Update_step2(IL,L);
			//step3,导纳矩阵
			Update_step3(L,M);
			//输出信息
			printf("Bus:\t%d\nIsland:\t%d\nIsland bus number:\t",L.length,IL.length);
			for(int m=0;m<IL.length;m++)
			{
				printf("%d\t",IL.base[m].isoc_num);
			}
			printf("\n");
			cmdStatus=OK;
			}
			else cmdStatus=ERROR;
		}
		else cmdStatus=ERROR;
		break;
	case 'i':
		buffer[token+6]='\0';	//ID和名字必须隔开
		if(strMatch(buffer,token,"insert"))
		{
			//移到记录部分(已知insert长度了)
			token+=6;
			//假装恢复信息
			buffer[token]=' ';
			//信息处理
			if(DataRecord_handler(&buffer[token],&id_tmp,&status_tmp,name_tmp,&bus_tmp,&bus2_tmp,&X_tmp)==ERROR)
				{
					cmdStatus=ERROR;
					break;
				}
			if(bus2_tmp<0)
			{
				cmdStatus=Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp);
				if(cmdStatus==OK)printf("OK\n");
			}
			else
			{
				cmdStatus=Insert(DB,id_tmp,status_tmp,name_tmp,bus_tmp,bus2_tmp,X_tmp);
				if(cmdStatus==OK)printf("OK\n");
			}
		}
		else cmdStatus=ERROR;
		break;
	case 'd':
		preProcess(buffer,token);
		if(strMatch(buffer,token,"delete"))
		{
			nextTok(buffer,token);
			if(1!=sscanf(&buffer[token],"ID=%d",&id_tmp))
				{
					cmdStatus=ERROR;
					break;
				}
			cmdStatus=Delete(DB,id_tmp);
			if(cmdStatus==OK)printf("OK\n");
		}
		else cmdStatus=ERROR;
		break;
	case 'c':
		buffer[token+6]='\0';	//ID和名字必须隔开
		if(strMatch(buffer,token,"change"))
		{
			//移到记录部分(已知insert长度了)
			token+=6;
			//假装恢复信息
			buffer[token]=' ';
			token++;
			//信息处理
			int token_beforeID=token;
			while(buffer[token]!=' '&&buffer[token]!='\t'&&buffer[token]!='\0')token++;
			buffer[token]='\0';
			token++;
			if(1!=sscanf(&buffer[token_beforeID],"ID=%d",&id_old))
				{
					cmdStatus=ERROR;
					break;
				}
			if(ERROR==DataRecord_handler(&buffer[token],&id_tmp,&status_tmp,name_tmp,&bus_tmp,&bus2_tmp,&X_tmp))
				{
					cmdStatus=ERROR;
					break;
				}
			//找结点
			Node* pNode;
			if((pNode=FindAVL_ID(DB.EquipTree,id_old))==NULL)
				{
					cmdStatus=ERROR;
					break;
				}
			if(((ElemType_Deg1*)pNode->data)->type==DEG1)
			{
				cmdStatus=Change(DB,pNode,id_old,id_tmp,status_tmp,name_tmp,bus_tmp);
				if(cmdStatus==OK)printf("OK\n");
			}
			else
			{
				cmdStatus=Change(DB,pNode,id_old,id_tmp,status_tmp,name_tmp,bus_tmp,bus2_tmp,X_tmp);
				if(cmdStatus==OK)printf("OK\n");
			}
		}
		else cmdStatus=ERROR;
		break;
	case 'p':
		preProcess(buffer,token);
		if(strMatch(buffer,token,"print"))
		{
			nextTok(buffer,token);
			if(1!=sscanf(&buffer[token],"ID=%d",&id_tmp))
				{
					cmdStatus=ERROR;
					break;
				}

			//临时变量
			Node* pNode;
			ElemType_Deg1* pdeg1;
			ElemType_Deg2* pdeg2;
			if((pNode=FindAVL_ID(DB.EquipTree,id_tmp))==NULL)
				{
					cmdStatus=ERROR;
					break;
				}
			switch (((ElemType_Deg1*)pNode->data)->type)	//同前，任意一种类型均可
			{
			case DEG1:
				pdeg1=((ElemType_Deg1*)pNode->data);

				printf("ID=%d %s AT %d STATUS %d\n",
					pdeg1->id,pdeg1->name,pdeg1->bus->id,pdeg1->status);

				break;
			case DEG2:
				pdeg2=((ElemType_Deg2*)pNode->data);
				if(pdeg2->X<=1e-6&&pdeg2->X>=-1e-6)		//即X==0,浮点数性质
				{
					printf("ID=%d %s FROM %d TO %d STATUS %d\n",
						pdeg2->id,pdeg2->name,pdeg2->bus_A->id,pdeg2->bus_B->id,pdeg2->status);
					cmdStatus=OK;
				}
				else
				{
					printf("ID=%d %s FROM %d TO %d STATUS %d x %f\n",
						pdeg2->id,pdeg2->name,pdeg2->bus_A->id,pdeg2->bus_B->id,pdeg2->status,pdeg2->X);
					cmdStatus=OK;
				}
				break;
			default:cmdStatus=ERROR;break;
			}
		}
		else cmdStatus=ERROR;
		break;
	case 'f':
		preProcess(buffer,token);
		if(strMatch(buffer,token,"find"))
		{
			nextTok(buffer,token);
			VexID_Type vexid;
			Node* pVex;
			if(1!=sscanf(&buffer[token],"fromto=%d",&vexid))
				{
					cmdStatus=ERROR;
					break;
				}
			if((pVex=FindAVL_VexID(DB.VexTree,vexid))==NULL)
				{
					cmdStatus=ERROR;
					break;
				}

			ChainNode* pChain=((ElemType_Vex*)pVex->data)->Deg1_Equips;
			AMListNode* pAMList=((ElemType_Vex*)pVex->data)->Deg2_Equips;
			while(pChain!=NULL)
			{
				printf("%d\n",pChain->id);
				pChain=pChain->next;
			}
			while(pAMList!=NULL)
			{
				printf("%d\n",pAMList->id);
				if(pAMList->vex_A==(ElemType_Vex*)pVex->data)
				{
					pAMList=pAMList->link_A;
				}
				else if(pAMList->vex_B==(ElemType_Vex*)pVex->data)
				{
					pAMList=pAMList->link_B;
				}
			}
			cmdStatus=OK;
		}
		else cmdStatus=ERROR;
		break;
	case 'e':
		preProcess(buffer,token);
		if(path[0]!='\0')
			{
				cmdStatus=ERROR;
				break;
			}
		else if(strMatch(buffer,token,"execute"))
		{
			nextTok(buffer,token);
			sscanf(&buffer[token],"%s",path);
			cmdStatus=OK;
			break;
		}
		else if(strMatch(buffer,token,"exit"))
		{
			strcpy(path,"exit");
			cmdStatus=OK;
			break;
		}
		else cmdStatus=OK;
		break;
	}
	switch (cmdStatus)
	{
	case OK:
		break;
	case ERROR:
		printf("ERROR\n");
		break;
	default:
		printf("UNKNOWN RESULT\n");		//调试用
		break;
	}
	return cmdStatus;
}