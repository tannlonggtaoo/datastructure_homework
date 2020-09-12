#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
//���Ͷ���
#define MAX_NAMESIZE 40		//�豸������󳤶�
typedef int ID_Type;		//�豸ID����
typedef int VexID_Type;		//�������ӵ�ID���ͣ�����Ϊ�˿ɶ��ԣ�
enum Equip_Type{BUS,DEG1,DEG2};		//�豸����:�������ӵ㡢1�ˡ�����
//��ע����������vex��busָ���������ӵ㣬isoeָ���ڵ㣩
enum Equip_Status {OFF,ON,BROKEN,UNKNOWN};	//�豸״̬

//AVL�����
//���е�dataֻ������������ElemType֮һ
typedef struct Node
{
	 void *data;
	 int balance_factor;
	 Node *left;
	 Node *right;
}Node,*BiTree;

//������
typedef struct ChainNode
{
	ID_Type id;
	ChainNode *next;
}ChainNode;

//�ڽӶ��ر�(Adjacent Multilist)�߽��
typedef struct AMListNode
{
	bool mark;			//�������
	ID_Type id;			//��Ӧ�����豸
	struct ElemType_Vex *vex_A,*vex_B;
	AMListNode *link_A,*link_B;
}AMListNode;

//�������ӵ�����ݵĽṹ
typedef struct ElemType_Vex		
{
	Equip_Type type;
	VexID_Type id;
	ChainNode *Deg1_Equips;			//һ���豸����
	AMListNode *Deg2_Equips;		//�����豸���ڽӶ��ر�(Adjacent Multilist)
	int isoe;						//��Ӧ�Ľڵ㣨�ȵ�λ�㣩���
	bool visited;					//�Ƿ���ʹ������
}ElemType_Vex;
//���������ӵ���豸�����ݵĽṹ
typedef struct ElemType_Deg1	
{
	Equip_Type type;
	ID_Type id;
	ElemType_Vex *bus;				//���ӵĵ������ӵ�ĵ�ַ
	Equip_Status status;
	char name[MAX_NAMESIZE];
}ElemType_Deg1;
//˫�������ӵ��豸�����ݵĽṹ
typedef struct ElemType_Deg2	
{
	Equip_Type type;
	ID_Type id;
	ElemType_Vex *bus_A;		//�������ӵ�1��ַ
	ElemType_Vex *bus_B;		//�������ӵ�2��ַ
	Equip_Status status;
	float X;					//�翹
	char name[MAX_NAMESIZE];
}ElemType_Deg2;
//���ݿ�ṹ
typedef struct dataBase
{
	BiTree EquipTree;
	BiTree VexTree;
}DataBase;
//ƽ�����ӡ���ȹ�ϵ�Ķ���
#define EqualHigh 0
#define LeftHigh +1
#define RightHigh -1
#define Equal(a,b) ((a)==(b))
#define LessThan(a,b) ((a)<(b))
#define MoreThan(a,b) ((a)>(b))
//ĳ��������������
Status DestoryChain(ElemType_Vex* pVex);

//������AVLTree��������
//����
//ע�⣺��ֻ�����ٱ��������ݣ������������������ϵ����ݲ����Ķ�����Ӧ�ڽӽ�����ʱʹ��
Status DestoryBiTree(BiTree &T)
{
	if(T==NULL)return OK;
	DestoryBiTree(T->left);
	DestoryBiTree(T->right);
	switch (((ElemType_Deg1*)T->data)->type)
	{
	case BUS:
		DestoryChain(((ElemType_Vex*)T->data));
		break;
	case DEG1:
		free(T->data);
		break;
	case DEG2:
		free(T->data);
		break;
	}
	free(T);
	return OK;
}

//��ѯ���ֲ�ѯ�豸����ѯ�������ӵ����ࣩ
Node* FindAVL_VexID(BiTree &T,VexID_Type id)
{
	if(T==NULL)return NULL;
	if(Equal(((ElemType_Vex *)T->data)->id,id))return T;
	if(MoreThan(((ElemType_Vex *)T->data)->id,id))return FindAVL_VexID(T->left,id);
	if(LessThan(((ElemType_Vex *)T->data)->id,id))return FindAVL_VexID(T->right,id);
}
Node* FindAVL_ID(BiTree &T,ID_Type id)		//ע��idλ����һ���ģ���deg1/deg2����ν
{
	if(T==NULL)return NULL;
	if(Equal(((ElemType_Deg1 *)T->data)->id,id))return T;
	if(MoreThan(((ElemType_Deg1 *)T->data)->id,id))return FindAVL_VexID(T->left,id);
	if(LessThan(((ElemType_Deg1 *)T->data)->id,id))return FindAVL_VexID(T->right,id);
}
//����
void R_Rotate(BiTree &p)
{
	Node* lc=p->left;
	p->left=lc->right;
	lc->right=p;
	p=lc;
}
//����
void L_Rotate(BiTree &p)
{
	Node* rc=p->right;
	p->right=rc->left;
	rc->left=p;
	p=rc;
}

//�����ʵ��
//��ƽ������Ӻ���
void LeftBalance(BiTree &T)
{
	Node* lc=T->left;
	switch (lc->balance_factor)
	{
	case LeftHigh:
		T->balance_factor=EqualHigh;
		lc->balance_factor=EqualHigh;
		R_Rotate(T);
		break;
	case RightHigh:
		Node *rd=lc->right;
		switch (rd->balance_factor)
		{
		case LeftHigh:
			T->balance_factor=RightHigh;
			lc->balance_factor=EqualHigh;
			break;
		case EqualHigh:
			T->balance_factor=EqualHigh;
			lc->balance_factor=EqualHigh;
			break;
		case RightHigh:
			T->balance_factor=EqualHigh;
			lc->balance_factor=LeftHigh;
			break;
		}
		rd->balance_factor=EqualHigh;
		L_Rotate(T->left);
		R_Rotate(T);
	}
}
//��ƽ������Ӻ���
void RightBalance(BiTree &T)
{
	Node* rc=T->right;
	switch (rc->balance_factor)
	{
	case RightHigh:
		T->balance_factor=EqualHigh;
		rc->balance_factor=EqualHigh;
		L_Rotate(T);
		break;
	case LeftHigh:
		Node *ld=rc->left;
		switch (ld->balance_factor)
		{
		case RightHigh:
			T->balance_factor=LeftHigh;
			rc->balance_factor=EqualHigh;
			break;
		case EqualHigh:
			T->balance_factor=EqualHigh;
			rc->balance_factor=EqualHigh;
			break;
		case LeftHigh:
			T->balance_factor=EqualHigh;
			rc->balance_factor=RightHigh;
			break;
		}
		ld->balance_factor=EqualHigh;
		R_Rotate(T->right);
		L_Rotate(T);
	}
}
//����
template <typename ElemType>
bool InsertAVL(BiTree &T,ElemType *e,bool &taller)//Ԥ�ȹ���e�ṹ��
{
	if(T==NULL)
	{
		T=(Node*)malloc(sizeof(Node));
		T->data=e;		//ע�⣺�����ǵ�ַ������
		T->left=NULL;
		T->right=NULL;
		T->balance_factor=EqualHigh;
		taller=true;
	}
	else
	{
		if(Equal(e->id,((ElemType*)T->data)->id))
		{
			taller=false;
			return 0;
		}

		if(LessThan(e->id,((ElemType*)T->data)->id))
		{
			if(!InsertAVL(T->left,e,taller))
			{
				return 0;
			}

			if(taller)
			{
				switch(T->balance_factor)
				{
				case LeftHigh:
					LeftBalance(T);
					taller=false;
					break;
				case EqualHigh:
					T->balance_factor=LeftHigh;
					taller=true;
					break;
				case RightHigh:
					T->balance_factor=EqualHigh;
					taller=false;
					break;
				}
			}
		}
		else
		{
			if(!InsertAVL(T->right,e,taller))
			{
				return 0;
			}

			if(taller)
			{
				switch(T->balance_factor)
				{
				case LeftHigh:
					T->balance_factor=EqualHigh;
					taller=false;
					break;
				case EqualHigh:
					T->balance_factor=RightHigh;
					taller=true;
					break;
				case RightHigh:
					RightBalance(T);
					taller=false;
					break;
				}
			}
		}
	}
	return 1;
}

//ɾ����ʵ�֣�id��Ϊint��
//�ݹ鲿��
Status DeleteAVL_rec(BiTree &p,int id,bool &shorter,Node* &pdel,Node* &plast)
{
	//���ҵ�pdel��key��Ӧ�Ľ�㣬������pdel,Ȼ���������
	if(p!=NULL&&Equal(((ElemType_Deg1*)p->data)->id,id))	//λ����ͬ��ElemType���ĸ�������ν
	{
		pdel=p;
	}

	if(p==NULL)	//���׾�return
	{
		if(pdel==NULL)return ERROR;	//û�ҵ�key��Ӧ�Ľ��
		else
		{
			plast=p;	//����plast����ʾ��һ����㣨ÿ�η���ʱ��¼����һ��ģ����ӵģ���ַ���Ӷ������ж�balancefactor����
			return OK;
		}
	}

	//����
	//С�ڵ�������ߣ�����pleafʱ�����������������ֵ
	//�˴��ݹ�����
	if(MoreThan(id,((ElemType_Deg1*)p->data)->id))
	{
		if(ERROR==DeleteAVL_rec(p->right,id,shorter,pdel,plast))
			return ERROR;
	}
	else if(ERROR==DeleteAVL_rec(p->left,id,shorter,pdel,plast))
		return ERROR;

	//��һ������,ֱ�ӳ�ȥ
	if(plast==NULL)
	{
		plast=p;
		return OK;
	}

	//�ڶ�������ʱ����ɾ��������������ɾ�������������޷��ࣩ
	//�ȶ��嵼��ָ�� left=1,right=2,������=0
	int dir=0;
	if(pdel!=NULL)
	{
		if(pdel->left==NULL)
		{
			//Ŀ��ɾ����������Ϊ��
			if(p->left==pdel)
			{
				//           p
				//          /
				//        pdel
				//            \
				//   (NULL)     ...
				p->left=pdel->right;
				dir=1;
			}
			else
			{
				//        p
				//         \
				//        pdel
				//            \
				//    (NULL)    ...
				p->right=pdel->right;
				dir=2;
			}
			free(plast->data);		//�ȷ��ڲ㣬������
			free(plast);
			pdel=NULL;
		}
		else
		{
			//Ŀ��ɾ�����������ǿ�
				pdel->data=plast->data;		//Ҷ������ݸ��Ƶ�Ŀ��ɾ����
				if(p==pdel)
				{
					//           pdel(p)
					//            /
					//          plast
					//           /   
					//        ...    (NULL)
					p->left=plast->left;
					dir=1;
				}
				else
				{
					//         p
					//        /  \
					//     ...    plast
					//            /   
					//         ...    (NULL)
					p->right=plast->left;
					dir=2;
				}
				free(plast->data);
				free(plast);
				pdel=NULL;
		}
	}

	//����balance factor
	//���Ѿ�ƽ����ֹͣ������ֱ���˳�
	if(shorter==false)return OK;

	//������һ�����̣��У�����ӣ��Ҷ��ӣ�*��p->balancefactor���ֿ��ܣ�=6��֧
	//����shorter==1
	int bf_temp=0;	//�ݴ���
	if(plast==p->left||dir==1)
	{
		//����һ������ǵ�ǰ���p�������
		switch(p->balance_factor)
		{
		case EqualHigh:
			p->balance_factor=RightHigh;
			shorter=0;
			break;
		case LeftHigh:
			p->balance_factor=EqualHigh;
			shorter=0;
			break;
		case RightHigh:
			switch (p->right->balance_factor)
			{
			case EqualHigh:
				L_Rotate(p);
				p->balance_factor=LeftHigh;
				p->left->balance_factor=RightHigh;
				shorter=0;
				break;
			case LeftHigh:
				bf_temp=p->right->left->balance_factor;
				R_Rotate(p->right);
				L_Rotate(p);
				p->balance_factor=EqualHigh;
				shorter=true;
				switch (bf_temp)
				{
				case LeftHigh:
					p->left->balance_factor=EqualHigh;
					p->right->balance_factor=RightHigh;
					break;
				case EqualHigh:
					p->left->balance_factor=EqualHigh;
					p->right->balance_factor=EqualHigh;
					break;
				case RightHigh:
					p->left->balance_factor=LeftHigh;
					p->right->balance_factor=EqualHigh;
					break;
				}
				break;
			case RightHigh:
				L_Rotate(p);
				p->balance_factor=EqualHigh;
				p->left->balance_factor=EqualHigh;
				shorter=true;
			}
			break;
		}
	}
	else if(plast==p->left||dir==2)
	{
		//����p���Ҷ���
		switch(p->balance_factor)
		{
		case EqualHigh:
			p->balance_factor=LeftHigh;
			shorter=0;
			break;
		case RightHigh:
			p->balance_factor=EqualHigh;
			shorter=0;
			break;
		case LeftHigh:
			switch (p->left->balance_factor)
			{
			case EqualHigh:
				R_Rotate(p);
				p->balance_factor=RightHigh;
				p->right->balance_factor=LeftHigh;
				shorter=0;
				break;
			case RightHigh:
				bf_temp=p->left->right->balance_factor;
				L_Rotate(p->left);
				R_Rotate(p);
				p->balance_factor=EqualHigh;
				shorter=true;
				switch (bf_temp)
				{
				case RightHigh:
					p->right->balance_factor=EqualHigh;
					p->left->balance_factor=LeftHigh;
					break;
				case EqualHigh:
					p->left->balance_factor=EqualHigh;
					p->right->balance_factor=EqualHigh;
					break;
				case LeftHigh:
					p->right->balance_factor=RightHigh;
					p->left->balance_factor=EqualHigh;
					break;
				}
				break;
			case LeftHigh:
				R_Rotate(p);
				p->balance_factor=EqualHigh;
				p->right->balance_factor=EqualHigh;
				shorter=true;
				break;
			}
			break;
		}
	}
	plast=p;
	return OK;
}
//�ⲿ���ýӿ�
Status DeleteAVL(BiTree &T,int key)
{
	//ע�⣺����pdel��pleaf�Ĺ��̶��ǳ�������������searchmode֮��ָʾ
	//����Ҫ�ĳ�<=����>���ң���Ӧrec�е�һ��if

	//true��ʾ��̣����򲻱�
	bool shorter_flag=true;	

	//����ݹ��е�ͨ�ÿ�������pdelָ����ɾ���Ľ�㣬plast�ڵݹ��ջ��ָ����һ��Ԫ�أ������ĸ�����׷�������ģ�
	Node *pdel=NULL,*plast=NULL;	
	return DeleteAVL_rec(T,key,shorter_flag,pdel,plast);
}

//������AMList������(����)���֣�ֻ����VexTree��
//����
//��������
Status DestoryChain(ElemType_Vex* pVex)
{
	if(pVex->Deg1_Equips==NULL)return OK;		//֮������Ƿ������������������Ƿ�ɾ��bus��㣨�������ӵ㣩
	ChainNode* pLast=pVex->Deg1_Equips;
	ChainNode* pCur=pLast->next;
	pVex->Deg1_Equips=NULL;
	while(pCur!=NULL)
	{
		free(pLast);
		pLast=pCur;
		pCur=pCur->next;
	}
	free(pLast);
	return OK;
}
//����(���ڿ�ͷ)
Status InsertChain(ElemType_Vex *pVex,ID_Type id)
{
	ChainNode *pNewNode;
	if((pNewNode=(ChainNode*)malloc(sizeof(ChainNode)))==NULL)return ERROR;
	pNewNode->id=id;
	pNewNode->next=pVex->Deg1_Equips;
	pVex->Deg1_Equips=pNewNode;
	return OK;
}
//ɾ��
Status DeleteChain(ElemType_Vex *pVex,ID_Type id)
{
	if(pVex->Deg1_Equips==NULL)return ERROR;
	ChainNode* pLast=pVex->Deg1_Equips;
	if(Equal(pLast->id,id))
	{
		pVex->Deg1_Equips=pLast->next;
		free(pLast);
		return OK;
	}
	ChainNode* pCur=pLast->next;
	while(pCur!=NULL&&!Equal(pCur->id,id))
	{
		pCur=pCur->next;
		pLast=pLast->next;
	}
	if(pCur==NULL)return ERROR;
	//�ҵ���
	pLast->next=pCur->next;
	free(pCur);
	return OK;
}

//AMList
//����ߣ�ֱ�Ӳ��ڿ�ͷ
Status InsertAMList(ElemType_Vex *pVex_A,ElemType_Vex *pVex_B,ID_Type id)
{
	AMListNode *pNewNode;
	if((pNewNode=(AMListNode*)malloc(sizeof(AMListNode)))==NULL)return ERROR;
	pNewNode->id=id;
	pNewNode->vex_A=pVex_A;
	pNewNode->vex_B=pVex_B;
	pNewNode->link_A=pVex_A->Deg2_Equips;
	pNewNode->link_B=pVex_B->Deg2_Equips;
	pVex_A->Deg2_Equips=pNewNode;
	pVex_B->Deg2_Equips=pNewNode;
	return OK;
}

//ɾ����
Status DeleteAMList(ElemType_Vex *pVex_A,ElemType_Vex *pVex_B,ID_Type id)
{
	if(pVex_A->Deg2_Equips==NULL||pVex_B->Deg2_Equips==NULL)return ERROR;
	
	//��A���Ĳ���
	AMListNode *pLast=pVex_A->Deg2_Equips;
	if(Equal(pLast->id,id))
	{
		pVex_A->Deg2_Equips=pLast->link_A;
	}
	else 
	{
		AMListNode *pCur=pLast->link_A;
		while(pCur!=NULL&&!Equal(pCur->id,id))
		{
			pCur=pCur->link_A;
			pLast=pLast->link_A;
		}
		if(pCur==NULL)return ERROR;
		//�ҵ���
		pLast->link_A=pCur->link_A;
	}

	//��B���Ĳ���
	pLast=pVex_B->Deg2_Equips;
	if(Equal(pLast->id,id))
	{
		pVex_B->Deg2_Equips=pLast->link_B;
	}
	else
	{
		AMListNode *pCur=pLast->link_B;
		while(pCur!=NULL&&!Equal(pCur->id,id))
		{
			pCur=pCur->link_B;
			pLast=pLast->link_B;
		}
		if(pCur==NULL)return ERROR;
		//�ҵ���
		pLast->link_B=pCur->link_B;
		free(pCur);
		return OK;
	}
}

//�����Ƕ���ӿ�
//���루���أ�
Status Insert(DataBase &DB,ID_Type id,Equip_Status status,char* name,VexID_Type busid)
{
	bool taller=true;
	//�Ȳ�Vextree
	Node* temp;
	ElemType_Vex* pVex;
	if((temp=FindAVL_VexID(DB.VexTree,busid))==NULL)
	{
		//�½�Vextree�Ľ��
		if((pVex=(ElemType_Vex*)malloc(sizeof(ElemType_Vex)))==NULL)return ERROR;
		pVex->Deg1_Equips=NULL;
		pVex->Deg2_Equips=NULL;
		pVex->id=busid;
		pVex->type=BUS;
		pVex->isoe=0;
		InsertAVL(DB.VexTree,pVex,taller);
	}
	else pVex=(ElemType_Vex*)temp->data;
	//�ѻ����Ч��pVex����deg1�������
	InsertChain(pVex,id);
	//�ѻ����ȷ��pVex,����IDtree��������
	ElemType_Deg1 *pdata;
	if((pdata=(ElemType_Deg1*)malloc(sizeof(ElemType_Deg1)))==NULL)return ERROR;
	pdata->bus=pVex;
	pdata->id=id;
	strcpy(pdata->name,name);
	pdata->status=status;
	pdata->type=DEG1;
	//����IDtree
	InsertAVL(DB.EquipTree,pdata,taller);
	return OK;
}
Status Insert(DataBase &DB,ID_Type id,Equip_Status status,char* name,VexID_Type busid_A,VexID_Type busid_B,float X)
{
	bool taller=true;
	//�Ȳ�Vextree
	Node* temp;
	ElemType_Vex* pVex_A;
	if((temp=FindAVL_VexID(DB.VexTree,busid_A))==NULL)
	{
		//�½�Vextree�Ľ��
		if((pVex_A=(ElemType_Vex*)malloc(sizeof(ElemType_Vex)))==NULL)return ERROR;
		pVex_A->Deg1_Equips=NULL;
		pVex_A->Deg2_Equips=NULL;
		pVex_A->id=busid_A;
		pVex_A->type=BUS;
		pVex_A->isoe=0;
		InsertAVL(DB.VexTree,pVex_A,taller);
	}
	else pVex_A=(ElemType_Vex*)temp->data;

	ElemType_Vex* pVex_B;
	if((temp=FindAVL_VexID(DB.VexTree,busid_B))==NULL)
	{
		//�½�Vextree�Ľ��
		if((pVex_B=(ElemType_Vex*)malloc(sizeof(ElemType_Vex)))==NULL)return ERROR;
		pVex_B->Deg1_Equips=NULL;
		pVex_B->Deg2_Equips=NULL;
		pVex_B->id=busid_B;
		pVex_B->type=BUS;
		pVex_B->isoe=0;
		InsertAVL(DB.VexTree,pVex_B,taller);
	}
	else pVex_B=(ElemType_Vex*)temp->data;
	//�ѻ����Ч��pVex����deg2ͼ����
	InsertAMList(pVex_A,pVex_B,id);
	//�ѻ����ȷ��pVexA��B,����IDtree��������
	ElemType_Deg2 *pdata;
	if((pdata=(ElemType_Deg2*)malloc(sizeof(ElemType_Deg2)))==NULL)return ERROR;
	pdata->bus_A=pVex_A;
	pdata->bus_B=pVex_B;
	pdata->id=id;
	pdata->X=X;
	strcpy(pdata->name,name);
	pdata->status=status;
	pdata->type=DEG2;
	//����IDtree
	InsertAVL(DB.EquipTree,pdata,taller);
	return OK;
}

//ɾ��
Status Delete(DataBase &DB,ID_Type id)
{
	Node* pNode;	//ָ���豸���Ľ��
	if((pNode=FindAVL_ID(DB.EquipTree,id))==NULL)return ERROR;

	if(((ElemType_Deg1*)pNode->data)->type==BUS)return ERROR;	//λ��ͬ
	if(((ElemType_Deg1*)pNode->data)->type==DEG1)
	{
		ElemType_Vex *pVex=((ElemType_Deg1*)pNode->data)->bus;

		//������ɾ��ID��Ϣ
		DeleteChain(pVex,id);

		//���û���豸��������������ӵ㣬��ɾ��VexTree��������
		if(pVex->Deg1_Equips==NULL&&pVex->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex->id);

		//EquipTree��ɾ�����
		DeleteAVL(DB.EquipTree,id);
	}
	else
	{
		ElemType_Vex *pVex_A=((ElemType_Deg2*)pNode->data)->bus_A;
		ElemType_Vex *pVex_B=((ElemType_Deg2*)pNode->data)->bus_B;

		//���ر�ɾ���ߵ�ID��Ϣ
		DeleteAMList(pVex_A,pVex_B,id);

		//���û���豸��������������ӵ㣬��ɾ��VexTree��������
		if(pVex_A->Deg1_Equips==NULL&&pVex_A->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex_A->id);
		if(pVex_B->Deg1_Equips==NULL&&pVex_B->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex_B->id);

		//EquipTree��ɾ�����
		DeleteAVL(DB.EquipTree,id);
	}
	return OK;
}

//�޸ģ��򵥷�����ע��ID��ȻҲ�ǿ����޸ĵĶ��������޸ĵ�������
//(��ʱ��֧���޸����࣬Ҳ��֧�ְ���·�ĵ翹��Ϊ0��
Status Change(DataBase &DB,Node* pNode,ID_Type old_id,ID_Type id,Equip_Status status,char* name,VexID_Type busid)		//����һ���򵥵�ʵ��
{
	//У�飺��ȻĬ�����Ϊ����ֵ(Ĭ��ֵ�μ�texthandler.h)
	if(id==0)id=old_id;
	if(!strcmp(name,"DEFAULT"))strcpy(name,((ElemType_Deg1*)pNode->data)->name);
	if(busid==0)busid=((ElemType_Deg1*)pNode->data)->bus->id;
	if(status==UNKNOWN)status=((ElemType_Deg1*)pNode->data)->status;
	//change
	if(DeleteAVL(DB.EquipTree,old_id)==ERROR)return ERROR;
	if(Insert(DB,id,status,name,busid)==ERROR)return ERROR;
	return OK;
}
Status Change(DataBase &DB,Node* pNode,ID_Type old_id,ID_Type id,Equip_Status status,char* name,VexID_Type busid_A,VexID_Type busid_B,float X)
{
	//У��
	if(id==0)id=((ElemType_Deg2*)pNode->data)->id;
	if(!strcmp(name,"DEFAULT"))strcpy(name,((ElemType_Deg2*)pNode->data)->name);
	if(busid_A==0)busid_A=((ElemType_Deg2*)pNode->data)->bus_A->id;
	if(busid_B==-1)busid_B=((ElemType_Deg2*)pNode->data)->bus_B->id;
	if(status==UNKNOWN)status=((ElemType_Deg2*)pNode->data)->status;
	if(X<1e-6&&X>-1e-6)X=((ElemType_Deg2*)pNode->data)->X;
	//change
	if(DeleteAVL(DB.EquipTree,old_id)==ERROR)return ERROR;
	if(Insert(DB,id,status,name,busid_A,busid_B,X)==ERROR)return ERROR;
	return OK;
}