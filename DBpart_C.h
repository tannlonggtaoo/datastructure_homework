#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
//类型定义
#define MAX_NAMESIZE 40		//设备名称最大长度
typedef int ID_Type;		//设备ID类型
typedef int VexID_Type;		//电气连接点ID类型（纯粹为了可读性）
enum Equip_Type{BUS,DEG1,DEG2};		//设备类型:电气连接点、1端、二端
//（注：本程序中vex或bus指代电气连接点，isoe指代节点）
enum Equip_Status {OFF,ON,BROKEN,UNKNOWN};	//设备状态

//AVL树结点
//其中的data只能是以上三种ElemType之一
typedef struct Node
{
	 void *data;
	 int balance_factor;
	 Node *left;
	 Node *right;
}Node,*BiTree;

//链表结点
typedef struct ChainNode
{
	ID_Type id;
	ChainNode *next;
}ChainNode;

//邻接多重表(Adjacent Multilist)边结点
typedef struct AMListNode
{
	bool mark;			//搜索标记
	ID_Type id;			//对应二端设备
	struct ElemType_Vex *vex_A,*vex_B;
	AMListNode *link_A,*link_B;
}AMListNode;

//电气连接点的数据的结构
typedef struct ElemType_Vex		
{
	Equip_Type type;
	VexID_Type id;
	ChainNode *Deg1_Equips;			//一端设备链表
	AMListNode *Deg2_Equips;		//二端设备的邻接多重表(Adjacent Multilist)
	int isoe;						//对应的节点（等电位点）序号
	bool visited;					//是否访问过结点标记
}ElemType_Vex;
//单电气连接点的设备的数据的结构
typedef struct ElemType_Deg1	
{
	Equip_Type type;
	ID_Type id;
	ElemType_Vex *bus;				//连接的电气连接点的地址
	Equip_Status status;
	char name[MAX_NAMESIZE];
}ElemType_Deg1;
//双电气连接点设备的数据的结构
typedef struct ElemType_Deg2	
{
	Equip_Type type;
	ID_Type id;
	ElemType_Vex *bus_A;		//电气连接点1地址
	ElemType_Vex *bus_B;		//电气连接点2地址
	Equip_Status status;
	float X;					//电抗
	char name[MAX_NAMESIZE];
}ElemType_Deg2;
//数据库结构
typedef struct dataBase
{
	BiTree EquipTree;
	BiTree VexTree;
}DataBase;
//平衡因子、相等关系的定义
#define EqualHigh 0
#define LeftHigh +1
#define RightHigh -1
#define Equal(a,b) ((a)==(b))
#define LessThan(a,b) ((a)<(b))
#define MoreThan(a,b) ((a)>(b))
//某两个函数的声明
Status DestoryChain(ElemType_Vex* pVex);

//以下是AVLTree函数部分
//销毁
//注意：这只会销毁本树的数据，对于外链到其他树上的数据不作改动；故应在接近结束时使用
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

//查询（分查询设备、查询电气连接点两类）
Node* FindAVL_VexID(BiTree &T,VexID_Type id)
{
	if(T==NULL)return NULL;
	if(Equal(((ElemType_Vex *)T->data)->id,id))return T;
	if(MoreThan(((ElemType_Vex *)T->data)->id,id))return FindAVL_VexID(T->left,id);
	if(LessThan(((ElemType_Vex *)T->data)->id,id))return FindAVL_VexID(T->right,id);
}
Node* FindAVL_ID(BiTree &T,ID_Type id)		//注：id位域是一样的，故deg1/deg2无所谓
{
	if(T==NULL)return NULL;
	if(Equal(((ElemType_Deg1 *)T->data)->id,id))return T;
	if(MoreThan(((ElemType_Deg1 *)T->data)->id,id))return FindAVL_VexID(T->left,id);
	if(LessThan(((ElemType_Deg1 *)T->data)->id,id))return FindAVL_VexID(T->right,id);
}
//右旋
void R_Rotate(BiTree &p)
{
	Node* lc=p->left;
	p->left=lc->right;
	lc->right=p;
	p=lc;
}
//左旋
void L_Rotate(BiTree &p)
{
	Node* rc=p->right;
	p->right=rc->left;
	rc->left=p;
	p=rc;
}

//插入的实现
//左平衡调节子函数
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
//右平衡调节子函数
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
//插入
template <typename ElemType>
bool InsertAVL(BiTree &T,ElemType *e,bool &taller)//预先构造e结构体
{
	if(T==NULL)
	{
		T=(Node*)malloc(sizeof(Node));
		T->data=e;		//注意：这里是地址操作！
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

//删除的实现（id设为int）
//递归部分
Status DeleteAVL_rec(BiTree &p,int id,bool &shorter,Node* &pdel,Node* &plast)
{
	//若找到pdel即key对应的结点，则设置pdel,然后继续向下
	if(p!=NULL&&Equal(((ElemType_Deg1*)p->data)->id,id))	//位域相同，ElemType是哪个都无所谓
	{
		pdel=p;
	}

	if(p==NULL)	//触底就return
	{
		if(pdel==NULL)return ERROR;	//没找到key对应的结点
		else
		{
			plast=p;	//设置plast，表示上一个结点（每次返回时记录上面一层的（儿子的）地址）从而便于判断balancefactor增减
			return OK;
		}
	}

	//向下
	//小于等于走左边，即找pleaf时总是找左子树的最大值
	//此处递归出入口
	if(MoreThan(id,((ElemType_Deg1*)p->data)->id))
	{
		if(ERROR==DeleteAVL_rec(p->right,id,shorter,pdel,plast))
			return ERROR;
	}
	else if(ERROR==DeleteAVL_rec(p->left,id,shorter,pdel,plast))
		return ERROR;

	//第一次跳出,直接出去
	if(plast==NULL)
	{
		plast=p;
		return OK;
	}

	//第二次跳出时附加删除结点操作（按照删除点左子树有无分类）
	//先定义导向指标 left=1,right=2,不导向=0
	int dir=0;
	if(pdel!=NULL)
	{
		if(pdel->left==NULL)
		{
			//目标删除点左子树为空
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
			free(plast->data);		//先放内层，后放外层
			free(plast);
			pdel=NULL;
		}
		else
		{
			//目标删除点左子树非空
				pdel->data=plast->data;		//叶结点数据复制到目标删除点
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

	//调整balance factor
	//若已经平衡则停止调整，直接退出
	if(shorter==false)return OK;

	//以下是一般流程，有（左儿子，右儿子）*（p->balancefactor三种可能）=6分支
	//以下shorter==1
	int bf_temp=0;	//暂存器
	if(plast==p->left||dir==1)
	{
		//若上一个结点是当前结点p的左儿子
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
		//若是p的右儿子
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
//外部调用接口
Status DeleteAVL(BiTree &T,int key)
{
	//注意：搜索pdel和pleaf的过程都是常规搜索！不用searchmode之类指示
	//不过要改成<=走左，>走右，对应rec中第一个if

	//true表示变短，否则不变
	bool shorter_flag=true;	

	//定义递归中的通用控制量：pdel指向欲删除的结点，plast在递归出栈中指向上一个元素（即从哪个儿子追溯上来的）
	Node *pdel=NULL,*plast=NULL;	
	return DeleteAVL_rec(T,key,shorter_flag,pdel,plast);
}

//以下是AMList和链表(无序)部分（只用于VexTree）
//链表
//销毁链表
Status DestoryChain(ElemType_Vex* pVex)
{
	if(pVex->Deg1_Equips==NULL)return OK;		//之后根据是否两个表都空了来决定是否删除bus结点（电气连接点）
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
//插入(插在开头)
Status InsertChain(ElemType_Vex *pVex,ID_Type id)
{
	ChainNode *pNewNode;
	if((pNewNode=(ChainNode*)malloc(sizeof(ChainNode)))==NULL)return ERROR;
	pNewNode->id=id;
	pNewNode->next=pVex->Deg1_Equips;
	pVex->Deg1_Equips=pNewNode;
	return OK;
}
//删除
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
	//找到了
	pLast->next=pCur->next;
	free(pCur);
	return OK;
}

//AMList
//插入边，直接插在开头
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

//删除边
Status DeleteAMList(ElemType_Vex *pVex_A,ElemType_Vex *pVex_B,ID_Type id)
{
	if(pVex_A->Deg2_Equips==NULL||pVex_B->Deg2_Equips==NULL)return ERROR;
	
	//对A链的操作
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
		//找到了
		pLast->link_A=pCur->link_A;
	}

	//对B链的操作
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
		//找到了
		pLast->link_B=pCur->link_B;
		free(pCur);
		return OK;
	}
}

//以下是对外接口
//插入（重载）
Status Insert(DataBase &DB,ID_Type id,Equip_Status status,char* name,VexID_Type busid)
{
	bool taller=true;
	//先插Vextree
	Node* temp;
	ElemType_Vex* pVex;
	if((temp=FindAVL_VexID(DB.VexTree,busid))==NULL)
	{
		//新建Vextree的结点
		if((pVex=(ElemType_Vex*)malloc(sizeof(ElemType_Vex)))==NULL)return ERROR;
		pVex->Deg1_Equips=NULL;
		pVex->Deg2_Equips=NULL;
		pVex->id=busid;
		pVex->type=BUS;
		pVex->isoe=0;
		InsertAVL(DB.VexTree,pVex,taller);
	}
	else pVex=(ElemType_Vex*)temp->data;
	//已获得有效的pVex，向deg1链表插入
	InsertChain(pVex,id);
	//已获得正确的pVex,构建IDtree的数据域
	ElemType_Deg1 *pdata;
	if((pdata=(ElemType_Deg1*)malloc(sizeof(ElemType_Deg1)))==NULL)return ERROR;
	pdata->bus=pVex;
	pdata->id=id;
	strcpy(pdata->name,name);
	pdata->status=status;
	pdata->type=DEG1;
	//插入IDtree
	InsertAVL(DB.EquipTree,pdata,taller);
	return OK;
}
Status Insert(DataBase &DB,ID_Type id,Equip_Status status,char* name,VexID_Type busid_A,VexID_Type busid_B,float X)
{
	bool taller=true;
	//先插Vextree
	Node* temp;
	ElemType_Vex* pVex_A;
	if((temp=FindAVL_VexID(DB.VexTree,busid_A))==NULL)
	{
		//新建Vextree的结点
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
		//新建Vextree的结点
		if((pVex_B=(ElemType_Vex*)malloc(sizeof(ElemType_Vex)))==NULL)return ERROR;
		pVex_B->Deg1_Equips=NULL;
		pVex_B->Deg2_Equips=NULL;
		pVex_B->id=busid_B;
		pVex_B->type=BUS;
		pVex_B->isoe=0;
		InsertAVL(DB.VexTree,pVex_B,taller);
	}
	else pVex_B=(ElemType_Vex*)temp->data;
	//已获得有效的pVex，向deg2图插入
	InsertAMList(pVex_A,pVex_B,id);
	//已获得正确的pVexA和B,构建IDtree的数据域
	ElemType_Deg2 *pdata;
	if((pdata=(ElemType_Deg2*)malloc(sizeof(ElemType_Deg2)))==NULL)return ERROR;
	pdata->bus_A=pVex_A;
	pdata->bus_B=pVex_B;
	pdata->id=id;
	pdata->X=X;
	strcpy(pdata->name,name);
	pdata->status=status;
	pdata->type=DEG2;
	//插入IDtree
	InsertAVL(DB.EquipTree,pdata,taller);
	return OK;
}

//删除
Status Delete(DataBase &DB,ID_Type id)
{
	Node* pNode;	//指向设备树的结点
	if((pNode=FindAVL_ID(DB.EquipTree,id))==NULL)return ERROR;

	if(((ElemType_Deg1*)pNode->data)->type==BUS)return ERROR;	//位域同
	if(((ElemType_Deg1*)pNode->data)->type==DEG1)
	{
		ElemType_Vex *pVex=((ElemType_Deg1*)pNode->data)->bus;

		//链表中删除ID信息
		DeleteChain(pVex,id);

		//如果没有设备附在这个电气连接点，则删除VexTree中这个结点
		if(pVex->Deg1_Equips==NULL&&pVex->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex->id);

		//EquipTree中删除结点
		DeleteAVL(DB.EquipTree,id);
	}
	else
	{
		ElemType_Vex *pVex_A=((ElemType_Deg2*)pNode->data)->bus_A;
		ElemType_Vex *pVex_B=((ElemType_Deg2*)pNode->data)->bus_B;

		//多重表删除边的ID信息
		DeleteAMList(pVex_A,pVex_B,id);

		//如果没有设备附在这个电气连接点，则删除VexTree中这个结点
		if(pVex_A->Deg1_Equips==NULL&&pVex_A->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex_A->id);
		if(pVex_B->Deg1_Equips==NULL&&pVex_B->Deg2_Equips==NULL)DeleteAVL(DB.VexTree,pVex_B->id);

		//EquipTree中删除结点
		DeleteAVL(DB.EquipTree,id);
	}
	return OK;
}

//修改（简单方案，注意ID当然也是可以修改的对象，这是修改的最坏情况）
//(暂时不支持修改种类，也不支持把线路的电抗改为0）
Status Change(DataBase &DB,Node* pNode,ID_Type old_id,ID_Type id,Equip_Status status,char* name,VexID_Type busid)		//先用一个简单的实现
{
	//校验：仍然默认则改为已有值(默认值参见texthandler.h)
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
	//校验
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