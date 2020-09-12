#pragma once
#include "const.h"
#include "database.h"
//表的最大长度和增量
#define MAXLEN 100
#define INC 20

//isoelec(节点)图和表
typedef struct ISOE_ChainNode
{
	ElemType_Vex* pVex;
	ISOE_ChainNode* next;
}ISOE_ChainNode;
typedef struct ISOE_AMListNode
{
	ElemType_Deg2* pwrline;		//指向相应的X非0的输电线
	int isoe_A,isoe_B;			//elec点（节点、等电位点）的序号
	ISOE_AMListNode *link_A,*link_B;
}ISOE_AMListNode;
typedef struct ISOE_Heads		//代表一个节点/等电位点
{
	int visited;				//true就是被访问过
	ISOE_ChainNode* ChainHead;
	ISOE_AMListNode* AMListHead;
}ISOE_Heads;
typedef struct ISOE_List
{
	int length;	//线性表长
	int listsize;		//存储容量
	ISOE_Heads* base;		//线性表头
}ISOE_List;

//对线性表的操作
Status InitList(ISOE_List &isoelist)
{
	ISOE_Heads* pTemp=(ISOE_Heads*)calloc(MAXLEN,sizeof(ISOE_Heads));
	if(pTemp==NULL)return ERROR;
	isoelist.base=pTemp;
	isoelist.listsize=MAXLEN;
	isoelist.length=0;
	return OK;
}
Status InsertList(ISOE_List &isoelist,ISOE_Heads heads)	//只在尾巴上插入
{
	if(isoelist.length>=isoelist.listsize)
	{
		//超出容量限制，则重新分配内存
		ISOE_Heads *newbase=(ISOE_Heads*)realloc(isoelist.base,(isoelist.listsize+INC)*sizeof(ISOE_Heads));
		if(!newbase)return ERROR;
		isoelist.base=newbase;
		isoelist.listsize+=INC;
	}
	isoelist.base[isoelist.length]=heads;
	isoelist.length++;
	return OK;
}
Status GetList(ISOE_List& L,int i,ISOE_Heads& record)		//根据指定的秩i获取一条记录
{
	if(i<1||i>L.length)return ERROR;
	record=L.base[i-1];
	return OK;
}
//对ISOE链表的操作
//插入(插在开头)
Status InsertISOE_Chain(ISOE_List &L,int idx,ElemType_Vex* pVex)
{
	ISOE_ChainNode *pNewNode;
	if((pNewNode=(ISOE_ChainNode*)malloc(sizeof(ISOE_ChainNode)))==NULL)return ERROR;
	pNewNode->pVex=pVex;
	pNewNode->next=L.base[idx-1].ChainHead;
	L.base[idx-1].ChainHead=pNewNode;
	return OK;
}
//对整个ISOE的删除
//遍历设备树
Status DeleteISOE_AMList(ISOE_List &L,int isoe_i,int isoe_j,ID_Type id);
 void FindPwrLine(ISOE_List &L,BiTree &pNode)
 {
	if(pNode==NULL)return;
	if(((ElemType_Deg2*)pNode->data)->type==DEG2)
	{
		if((((ElemType_Deg2*)pNode->data)->X>=1e-6||((ElemType_Deg2*)pNode->data)->X<=-1e-6)&&((ElemType_Deg2*)pNode->data)->status==ON)
		{
			//是二端设备、开着、有电抗,则删除
			DeleteISOE_AMList(L,((ElemType_Deg2*)(pNode->data))->bus_A->isoe,((ElemType_Deg2*)(pNode->data))->bus_B->isoe,((ElemType_Deg2*)(pNode->data))->id);
		}
	}
	//下两个
	FindPwrLine(L,pNode->left);
	FindPwrLine(L,pNode->right);
	return;
 }
Status DestoryList(ISOE_List& L,BiTree &EquipTree)
{
	ISOE_ChainNode *pNode,*ptemp;
	//先删链表
	for(int i=0;i<L.length;i++)
	{
		pNode=L.base[i].ChainHead;
		while(pNode!=NULL)
		{
			ptemp=pNode->next;
			free(pNode);
			pNode=ptemp;
		}
	}
	//再删边：在设备树中遍历开着的输电线路
	FindPwrLine(L,EquipTree);
	//然后放掉数组
	free(L.base);
	L.base=NULL;
	return OK;
}

//插入边（插在开头）
Status InsertISOE_AMList(ISOE_List &L,int isoe_A,int isoe_B,ElemType_Deg2 *pwrline)
{
	ISOE_AMListNode *pNewNode;
	if((pNewNode=(ISOE_AMListNode*)malloc(sizeof(ISOE_AMListNode)))==NULL)return ERROR;
	pNewNode->isoe_A=isoe_A;
	pNewNode->isoe_B=isoe_B;
	pNewNode->link_A=L.base[isoe_A-1].AMListHead;
	pNewNode->link_B=L.base[isoe_B-1].AMListHead;
	pNewNode->pwrline=pwrline;
	L.base[isoe_A-1].AMListHead=pNewNode;
	L.base[isoe_B-1].AMListHead=pNewNode;
	return OK;
}
//删除边
Status DeleteISOE_AMList(ISOE_List &L,int isoe_i,int isoe_j,ID_Type id)
{
	if(L.base[isoe_i-1].AMListHead==NULL||L.base[isoe_j-1].AMListHead==NULL)return ERROR;
	
	//对A链的操作
	ISOE_AMListNode *pLast=L.base[isoe_i-1].AMListHead;
	if(Equal(pLast->pwrline->id,id))
	{
		L.base[isoe_i-1].AMListHead=pLast->link_A;
	}
	else 
	{
		ISOE_AMListNode *pCur=pLast->link_A;
		while(pCur!=NULL&&!Equal(pCur->pwrline->id,id))
		{
			pCur=pCur->link_A;
			pLast=pLast->link_A;
		}
		if(pCur==NULL)return ERROR;
		//找到了
		pLast->link_A=pCur->link_A;
	}

	//对B链的操作
	pLast=L.base[isoe_j-1].AMListHead;
	if(Equal(pLast->pwrline->id,id))
	{
		L.base[isoe_j-1].AMListHead=pLast->link_B;
	}
	else
	{
		ISOE_AMListNode *pCur=pLast->link_B;
		while(pCur!=NULL&&!Equal(pCur->pwrline->id,id))
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

//update分3步：
//Vextree->isoelec graph
//isoelec graph->island graph
//isoelec graph->matrix
//以上都是为第一步作铺垫

/*
typedef struct RecordNode
{
	ElemType_Vex* pVex;
	RecordNode* next;
}RecordNode;

//遍历VexTree
Status traverseVexTree(Node* T,RecordNode* VexTable)
{
	if(T==NULL)return;
	//操作:插入
	RecordNode* pNode;
	if((pNode=(RecordNode*)malloc(sizeof(RecordNode)))==NULL)return ERROR;
	
	pNode->pVex=(ElemType_Vex*)T->data;

	if(VexTable==NULL)
	{
		VexTable=pNode;
		pNode->next=NULL;
	}
	else
	{
		pNode->next=VexTable->next;
		VexTable->next=pNode;
	}
	//向下遍历
	traverseVexTree(T->left,VexTable);
	traverseVexTree(T->right,VexTable);
}
*/

//存输电线的链表
typedef struct pwrList
{
	AMListNode* pPwrline;
	ElemType_Deg2* pEquip;
	pwrList* next;
}pwrList;

//递归把所有Node的visited置为false
void unvisited(Node* T)
{
	if(T==NULL)return;
	((ElemType_Vex*)T->data)->visited=false;
	unvisited(T->left);
	unvisited(T->right);
	return;
}

//递归把pVex的连通支找出来(注意！这里不支持自环！)
void visitVex(DataBase &DB,ElemType_Vex* pVex,int &cur_isoe_idx,ISOE_List &L,pwrList* &pwrlist)
{
	//T在第cur_isoe_idx个节点/连通支/等电位点/总线上，作相应处理
	//给vex点作标记
	pVex->isoe=cur_isoe_idx;
	//给目标isoelec图作改变
	InsertISOE_Chain(L,cur_isoe_idx,pVex);
	//visited改为true
	pVex->visited=true;

	//遍历多重表找下一个
	AMListNode* pEdge=pVex->Deg2_Equips;
	ElemType_Vex* pNextVex;
	while(pEdge!=NULL)
	{
		//先确定pEdge是否可用、是否是输电线路（是输电线路就存到链表（还没做）里）
		Node* pNode=FindAVL_ID(DB.EquipTree,pEdge->id);
		if(pNode==NULL)return;
		if(((ElemType_Deg2*)pNode->data)->X>=1e-6||((ElemType_Deg2*)pNode->data)->X<=-1e-6)
		{
			//X!=0,若status==ON,且对面的点还未访问（防重复访问）则存输电线路
			if(((ElemType_Deg2*)pNode->data)->status==ON)
			{
				ElemType_Vex* pOther=pEdge->vex_A!=pVex?pEdge->vex_A:pEdge->vex_B;		//pOther就是输电线对面的Vex结点
				//若该结点未访问，则加入输电线
				if(pOther->visited==false)
				{
					pwrList* newPwrLine;
					if((newPwrLine=(pwrList*)malloc(sizeof(pwrList)))==NULL)return;

					newPwrLine->pPwrline=pEdge;
					newPwrLine->pEquip=((ElemType_Deg2*)pNode->data);

					if(pwrlist==NULL)
					{
						pwrlist=newPwrLine;
						pwrlist->next=NULL;
					}
					else
					{
						newPwrLine->next=pwrlist->next;
						pwrlist->next=newPwrLine;
					}
				}
			}

		}
		else if(((ElemType_Deg2*)pNode->data)->status==ON)
		{
			//X==0且是闭合工作状态
			//找到pedge的另一个结点
			pNextVex=pEdge->vex_A!=pVex?pEdge->vex_A:pEdge->vex_B;
			//若该结点未访问，则visit该结点
			if(pNextVex->visited==false)
				visitVex(DB,pNextVex,cur_isoe_idx,L,pwrlist);
		}
		
		//下一条边
		pEdge=pEdge->vex_A==pVex?pEdge->link_A:pEdge->link_B;
	}
}

//在Vextree上递归遍历
void visit(DataBase &DB,Node* pNode,int &cur_isoe_idx,ISOE_List &L,pwrList* &pwrlist)
{
	if(pNode==NULL)return;
	//未遍历，处理一下
	if(((ElemType_Vex*)pNode->data)->visited==false)
	{
		cur_isoe_idx++;
		ISOE_Heads heads={NULL,NULL};
		InsertList(L,heads);
		//找到这个vex结点的连通支
		visitVex(DB,(ElemType_Vex*)pNode->data,cur_isoe_idx,L,pwrlist);
	}
	//若已遍历/未遍历但已处理，则下两个
	visit(DB,pNode->left,cur_isoe_idx,L,pwrlist);
	visit(DB,pNode->right,cur_isoe_idx,L,pwrlist);
	return;
}

//第一步：节点（等电势点/isoelec）图构造
Status Update_step1(dataBase &DB,ISOE_List &L)
{
	
	//标识当前正在探查第几个等电位点内含的Vex结点
	int cur_isoe_idx=0;		
	
	//遍历一遍VexTree，把搜索标志mark置0
	unvisited(DB.VexTree);

	//初始化目标图
	InitList(L);

	//初始化输电线暂存器
	pwrList* pwrlist=NULL;

	//开始深搜,对Vextree进行遍历
	visit(DB,DB.VexTree,cur_isoe_idx,L,pwrlist);

	//下一步：连输电线的边
	pwrList* pnext;
	if(pwrlist!=NULL)
	{
		//如果是NULL，则没有输电线，跳过连接步骤
		//否则就来到以下连接isoe步骤
		pnext=pwrlist;
		
		while(pnext!=NULL)
		{
		pnext=pnext->next;
		//连pwrlist对应的输电线
		InsertISOE_AMList(L,pwrlist->pPwrline->vex_A->isoe,pwrlist->pPwrline->vex_B->isoe,pwrlist->pEquip);
		free(pwrlist);
		pwrlist=pnext;
		}
	}
	//完成
	return OK;

}

//以下为第二阶段准备
typedef struct ISLAND_ChainNode
{
	int isoe_idx;
	ISLAND_ChainNode* next;
}ISLAND_ChainNode;

typedef struct ISLAND_ChainHead
{
	int isoc_num;
	ISLAND_ChainNode* pHead;
}ISLAND_ChainHead;

typedef struct ISLAND_List
{
	ISLAND_ChainHead* base;
	int listsize;
	int length;
}ISLAND_List;

//链表插入
Status InsertISLAND_Chain(ISLAND_List &IL,int island_idx,int isoe_idx)
{
	//分配内存
	ISLAND_ChainNode* pnewnode=(ISLAND_ChainNode*)malloc(sizeof(ISLAND_ChainNode));
	if(pnewnode==NULL)return ERROR;

	//插在第一个
	pnewnode->isoe_idx=isoe_idx;
	pnewnode->next=IL.base[island_idx-1].pHead;
	IL.base[island_idx-1].pHead=pnewnode;
	IL.base[island_idx-1].isoc_num++;
	return OK;
}

//线性表初始化
Status InitISLAND_List(ISLAND_List& IL) 
{
  // 构造一个空的顺序线性表L。
	IL.base = (ISLAND_ChainHead*)calloc(MAXLEN,sizeof(ISLAND_ChainHead));
	if (IL.base==NULL) return ERROR;        // 存储分配失败
	IL.length = 0;
	IL.listsize = MAXLEN;
	return OK;
} 

//线性表扩容
Status SizeupISLAND_List(ISLAND_List& IL)
{
	if(IL.length>=IL.listsize)
	{
		//超出容量限制，则重新分配内存
		ISLAND_ChainHead *newbase=(ISLAND_ChainHead*)realloc(IL.base,(IL.listsize+INC)*sizeof(ISLAND_ChainHead));
		if(!newbase)return ERROR;
		IL.base=newbase;
		IL.listsize+=INC;
	}
	IL.length++;
	return OK;
}

//线性表读
Status GetISLAND(ISLAND_List& IL,int i,ISLAND_ChainHead& record)		//根据指定的秩i获取一条记录
{
	if(i<1||i>IL.length)return ERROR;
	record=IL.base[i-1];
	return OK;
}

//销毁线性表
void DestoryISLAND(ISLAND_List &IL)
{
	ISLAND_ChainNode *pNode,*ptemp;
	for(int i=0;i<IL.length;i++)
	{
		pNode=IL.base[i].pHead;
		while(pNode!=NULL)
		{
			ptemp=pNode->next;
			free(pNode);
			pNode=ptemp;
		}
	}
	free(IL.base);
	IL.base=NULL;
	return;
}

//对当前节点/等电位点dfs把连通支找出来
Status visitISOE(ISOE_List &L,int isoe_idx,ISLAND_List &IL,int island_idx)
{
	int next_isoe_idx;
	//先把自己插进ISLAND

	InsertISLAND_Chain(IL,island_idx,isoe_idx);
	L.base[isoe_idx-1].visited=true;

	//遍历与该结点相邻的边,从而找到邻集，对邻集中每个点都递归一次
	ISOE_AMListNode* pEdge=L.base[isoe_idx-1].AMListHead;
	while(pEdge!=NULL)
	{
		//这条边对应的邻结点
		next_isoe_idx=pEdge->isoe_A!=isoe_idx?pEdge->isoe_A:pEdge->isoe_B;

		//递归一发
		if(L.base[next_isoe_idx-1].visited==false)
		{

			visitISOE(L,next_isoe_idx,IL,island_idx);
		}

		//下一条边
		pEdge=pEdge->isoe_A==isoe_idx?pEdge->link_A:pEdge->link_B;
	}
	//已访问则直接退出
	return OK;
}

//第二步：根据L生成拓扑岛/孤岛
Status Update_step2(ISLAND_List &IL,ISOE_List &L)
{
	//先把visited归0（calloc，新生成的L不用归）
	int isoe_idx;
	//初始化
	InitISLAND_List(IL);
	//然后遍历L各节点
	int island_idx=0;
	for(isoe_idx=1;isoe_idx<=L.length;isoe_idx++)
	{
		//访问过了就不再访问
		if(L.base[isoe_idx-1].visited==false)
		{
			//没访问过就开始以此点开始DPS
			island_idx++;
			SizeupISLAND_List(IL);
			//填充新开辟的List内存块和链表
			visitISOE(L,isoe_idx,IL,island_idx);
		}
	}

	//排序:数量少，则用最简单的冒泡（沉底）
	int i,j;
	int min_idx;
	ISLAND_ChainHead temp;
	for(i=0;i<IL.length;i++)
	{
		min_idx=i;
		for(j=i;j<IL.length;j++)
		{
			if(IL.base[j].isoc_num<IL.base[min_idx].isoc_num)
				min_idx=j;
		}
		if(min_idx!=i)
		{
			temp=IL.base[i];
			IL.base[i]=IL.base[min_idx];
			IL.base[min_idx]=temp;
		}
	}

	return OK;
}

//关于导纳矩阵
//结点结构体
typedef struct OLNode
{
	int i,j;
	float e;
	OLNode *right,*down;
}OLNode,* OLink;
//数组结构体
typedef struct
{
	OLink *rhead,*chead;	//r=row，c=column
	int n,tu;
}CrossList;
//初始化
Status InitCrossList(CrossList &M,int n)
{
	//暂时只考虑M是刚刚定义，内部为空的数组
	//如果M非空可能出现bug------------------------------------------------------
	M.n=n;
	M.tu=0;
	OLink *phead;
	//行、列头指针初始化
	phead=(OLink*)calloc(n,sizeof(OLink));
	if(phead==NULL)return ERROR;
	M.chead=phead;
	phead=(OLink*)calloc(n,sizeof(OLink));
	if(phead==NULL)return ERROR;
	M.rhead=phead;
}
//销毁矩阵
Status DestoryCrossList(CrossList &M)
{
	OLNode *pElem,*pNext;
	int i;
	//一行一行删除
	for(i=0;i<M.n;i++)
	{
		if(M.rhead[i]==NULL)continue;
		else
		{
			pElem=M.rhead[i];
			pNext=M.rhead[i]->right;
			while (pNext!=NULL)
			{
				free(pElem);
				pElem=pNext;
				pNext=pNext->right;
			}
			free(pElem);
		}
	}
	free(M.chead);
	free(M.rhead);
	return OK;
}
//初始化元素，对e=0不做响应
Status Insert_symm(CrossList &M,int i,int j,float e)
{
	//检查参数
	if(i<1||j<1||i>M.n||j>M.n)return ERROR;
	if(e==0)return OK;
	M.tu++;
	if(i<j)		//保证i>=j，从而只存储在下三角矩阵上，下同
	{
		int temp;
		temp=i;
		i=j;
		j=temp;
	}

	//先构造新结点
	OLink pnode,psearch;
	if(!(pnode=(OLNode*)calloc(1,sizeof(OLNode))))return ERROR;
	pnode->e=e;
	pnode->i=i;
	pnode->j=j;
	//设置新结点的down（以下改造自书上算法）
	if(M.chead[j-1]==NULL||M.chead[j-1]->i>i)
	{
		pnode->down=M.chead[j-1];
		M.chead[j-1]=pnode;
	}
	else
	{
		//直到psearch指向目标的前（上、左）一个结点
		for(psearch=M.chead[j-1];(psearch->down!=NULL)&&psearch->down->i<i;psearch=psearch->down){};
		//同一个i
		if(psearch->down!=NULL&&psearch->down->i==i)
		{
			psearch->down->e=e;
			M.tu--;
			free(pnode);
			return OK;
		}
		//不同的i
		pnode->down=psearch->down;
		psearch->down=pnode;
	}
	//设置新结点的right
	if(M.rhead[i-1]==NULL||M.rhead[i-1]->j>j)
	{
		pnode->right=M.rhead[i-1];
		M.rhead[i-1]=pnode;
	}
	else
	{
		for(psearch=M.rhead[i-1];(psearch->right!=NULL)&&psearch->right->j<j;psearch=psearch->right){};
		//不可能再出现同一个j
		//不同的j
		pnode->right=psearch->right;
		psearch->right=pnode;
	}
	return OK;
}
//取元素
Status Get_symm(CrossList &M,int i,int j,float &a)
{
	if(i<1||j<1||i>M.n||j>M.n)return ERROR;
	if(i<j)		//保证i>=j，从而只读取下三角矩阵
	{
		int temp;
		temp=i;
		i=j;
		j=temp;
	}
	OLink psearch;
	//和书上算法不同
	//重点关注每行/列第一个就是目标结点的情况。此时书上样例无效
	if(M.rhead[i-1]==NULL||M.rhead[i-1]->j>=j)
	{
		if(M.rhead[i-1]==NULL)
		{
			a=0;return OK;
		}
		if(M.rhead[i-1]->j==j)
		{
			a=M.rhead[i-1]->e;
			return OK;
		}
	}
	psearch=M.rhead[i-1];
	while((psearch->right!=NULL)&&psearch->right->j<=j)
	{
		if(psearch->right->j==j)
		{
			a=psearch->right->e;
			return OK;
		}
		psearch=psearch->right;
	}
	a=0;
	return OK;
}

//第三步：计算导纳矩阵
float CrossListATij(ISOE_List &L,int i,int j)
{
	float mat_elem=0.0f;
	//找到所有i-j的输电线（可能不止一条）
	ISOE_AMListNode* pEdge=L.base[i-1].AMListHead;
	while (pEdge!=NULL)
	{
		if(pEdge->isoe_A==j||pEdge->isoe_B==j)
		{
			mat_elem+=1/(pEdge->pwrline->X);
		}
		pEdge=pEdge->isoe_A==i?pEdge->link_A:pEdge->link_B;
	}
	return mat_elem;
}
Status Update_step3(ISOE_List &L,CrossList &M)
{
	float mat_elem;
	float mat_diag;
	InitCrossList(M,L.length);
	//先算非对角线元素
	for(int i=1;i<=L.length;i++)
	{
		mat_diag=0.0f;
		for(int j=1;j<=L.length;j++)
		{
			if(i==j)continue;
			mat_elem=CrossListATij(L,i,j);
			Insert_symm(M,i,j,mat_elem);
			mat_diag-=mat_elem;
		}
		//再算对角线元素
		Insert_symm(M,i,i,mat_diag);
	}
	return OK;
}

//求关节点留下的函数接口
void DFSArticul(ISOE_List &L,int v0,int &count,int* &low,int vexnum,bool &null_flag)
{
	int min;
	count++;
	L.base[v0-1].visited=count;
	min=count;
	ISOE_AMListNode* pEdge;
	for(pEdge=L.base[v0-1].AMListHead;pEdge!=NULL;pEdge=pEdge->isoe_A==v0?pEdge->link_A:pEdge->link_B)
	{
		int isoe_w=pEdge->isoe_A!=v0?pEdge->isoe_A:pEdge->isoe_B;
		if(L.base[isoe_w-1].visited==0)
		{
			DFSArticul(L,isoe_w,count,low,vexnum,null_flag);
			if(low[isoe_w-1]<min)
				min=low[isoe_w-1];
			if(low[isoe_w-1]>=L.base[v0-1].visited)
			{
				null_flag=false;
				printf("%d ",v0);
			}
		}
		else if(L.base[isoe_w-1].visited<min)
		{
			min=L.base[isoe_w-1].visited;
		}
	}
	low[v0-1]=min;
}
void FindArticul(ISOE_List &L,int vexnum,int isoe_root,bool &null_flag)
{
	int *low;
	if((low=(int*)calloc(vexnum,sizeof(int)))==NULL)return;

	int count=1;
	L.base[isoe_root-1].visited=1;
	for(int i=1;i<L.length;i++)
	{
		L.base[i].visited=0;
	}
	ISOE_AMListNode* pEdge=L.base[isoe_root-1].AMListHead;
	int v=pEdge->isoe_A!=isoe_root?pEdge->isoe_A:pEdge->isoe_B;
	DFSArticul(L,v,count,low,vexnum,null_flag);
	if(count<vexnum)
	{
		null_flag=false;
		printf("%d ",isoe_root);
		pEdge=pEdge->isoe_A==isoe_root?pEdge->link_A:pEdge->link_B;
		while (pEdge!=NULL)
		{
			v=pEdge->isoe_A!=isoe_root?pEdge->isoe_A:pEdge->isoe_B;
			pEdge=pEdge->isoe_A==isoe_root?pEdge->link_A:pEdge->link_B;

			if(L.base[v-1].visited==0)
				DFSArticul(L,v,count,low,vexnum,null_flag);
		}
	}
}

