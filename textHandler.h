#pragma once
//�ı���������
//strtok��֧�����ģ����ֶ�ʵ��
#include <stdio.h>
#include <string.h>
#include "const.h"
#include "DBpart_C.h"
#include "graph.h"
//���ٵ��ú����Ŀ���
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
//��[��][ֵ][��][ֵ]...��ʽ�Ĵ����д���
Status DataRecord_handler(char *buffer,ID_Type* id_tmp,Equip_Status* status_tmp,char* name_tmp,VexID_Type* bus_tmp,VexID_Type* bus2_tmp,float* X_tmp)
{
	int token;
	preProcess(buffer,token);
	while(buffer[token]=='\0')token++;	//�Ƶ���һ������Ԫ
	while(buffer[token]!='$')
	{
		switch (buffer[token])		//�ȵ��ú����Ŀ���С
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
			//δ������break����δƥ��ɹ�
			else return ERROR;
		}
		nextTok(buffer,token);
	}
	return OK;
}

//Ϊ�ڵ���ر߶��ƵĻ�����
typedef struct {
	ID_Type* base;	// ���Ա��׵�ַ
	int listsize;		// ����Ĵ洢�ռ�
	int length;		// ���Ա���
}BusBuf;
Status InitBusBuf(BusBuf& L) {
  // ����һ���յ�˳�����Ա�L��
	L.base = (ID_Type*)malloc(BUSBUF_SIZE*sizeof(ID_Type));
	if (L.base==NULL) return ERROR;        // �洢����ʧ��
	L.length = 0;
	L.listsize = BUSBUF_SIZE;
	return OK;
} 
Status InsertBusBuf(BusBuf& L,ID_Type id)	//ֻ��β���ϲ���
{
	if(L.length>=L.listsize)
	{
		//�����������ƣ������·����ڴ�
		ID_Type *newbase=(ID_Type*)realloc(L.base,(L.listsize+BUSBUF_SIZE)*sizeof(ID_Type));
		if(!newbase)return ERROR;
		L.base=newbase;
		L.listsize+=BUSBUF_INC;
	}
	L.base[L.length]=id;
	L.length++;
	return OK;
}
Status DeleteBusBuf(BusBuf& L,int i)		//����ָ������iɾ��һ����¼
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
Status DestoryBusBuf(BusBuf& L)	//�黹�ռ�
{
	free(L.base);
	L.base=NULL;
	return OK;
}

//����ʶ����
Status Command_handler(DataBase &DB,char* buffer,char* path,ISOE_List &L,ISLAND_List &IL,CrossList &M)
{
	int token=0;
	//������ʱ����
	ID_Type id_tmp=0,id_old=0;
	Equip_Status status_tmp=UNKNOWN;
	char name_tmp[MAX_NAMESIZE]="DEFAULT";
	VexID_Type bus_tmp=0;
	VexID_Type bus2_tmp=-1;
	float X_tmp=0.0f;

	//����״̬
	Status cmdStatus;
	//�Ƶ���һ������Ԫ
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
						if(IL.base[idx-1].isoc_num>2)		//���㡢����Ǹ��
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
					//�����ر߻���
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
					//��isoeͼ�Ҵ�,˳��i��j
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
			//step0,�ѾɵĽṹɾ��
			DestoryCrossList(M);
			DestoryISLAND(IL);
			DestoryList(L);
			}
			//step1
			Update_step1(DB,L);
			//step2,����ISLAND��
			Update_step2(IL,L);
			//step3,���ɾ���
			Update_step3(L,M);
			//�����Ϣ
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
		buffer[token+6]='\0';	//ID�����ֱ������
		if(strMatch(buffer,token,"insert"))
		{
			//�Ƶ���¼����(��֪insert������)
			token+=6;
			//��װ�ָ���Ϣ
			buffer[token]=' ';
			//��Ϣ����
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
		buffer[token+6]='\0';	//ID�����ֱ������
		if(strMatch(buffer,token,"change"))
		{
			//�Ƶ���¼����(��֪insert������)
			token+=6;
			//��װ�ָ���Ϣ
			buffer[token]=' ';
			token++;
			//��Ϣ����
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
			//�ҽ��
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

			//��ʱ����
			Node* pNode;
			ElemType_Deg1* pdeg1;
			ElemType_Deg2* pdeg2;
			if((pNode=FindAVL_ID(DB.EquipTree,id_tmp))==NULL)
				{
					cmdStatus=ERROR;
					break;
				}
			switch (((ElemType_Deg1*)pNode->data)->type)	//ͬǰ������һ�����;���
			{
			case DEG1:
				pdeg1=((ElemType_Deg1*)pNode->data);

				printf("ID=%d %s AT %d STATUS %d\n",
					pdeg1->id,pdeg1->name,pdeg1->bus->id,pdeg1->status);

				break;
			case DEG2:
				pdeg2=((ElemType_Deg2*)pNode->data);
				if(pdeg2->X<=1e-6&&pdeg2->X>=-1e-6)		//��X==0,����������
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
		printf("UNKNOWN RESULT\n");		//������
		break;
	}
	return cmdStatus;
}