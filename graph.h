#pragma once
#include "const.h"
#include "database.h"
//�����󳤶Ⱥ�����
#define MAXLEN 100
#define INC 20

//isoelec(�ڵ�)ͼ�ͱ�
typedef struct ISOE_ChainNode
{
	ElemType_Vex* pVex;
	ISOE_ChainNode* next;
}ISOE_ChainNode;
typedef struct ISOE_AMListNode
{
	ElemType_Deg2* pwrline;		//ָ����Ӧ��X��0�������
	int isoe_A,isoe_B;			//elec�㣨�ڵ㡢�ȵ�λ�㣩�����
	ISOE_AMListNode *link_A,*link_B;
}ISOE_AMListNode;
typedef struct ISOE_Heads		//����һ���ڵ�/�ȵ�λ��
{
	int visited;				//true���Ǳ����ʹ�
	ISOE_ChainNode* ChainHead;
	ISOE_AMListNode* AMListHead;
}ISOE_Heads;
typedef struct ISOE_List
{
	int length;	//���Ա�
	int listsize;		//�洢����
	ISOE_Heads* base;		//���Ա�ͷ
}ISOE_List;

//�����Ա�Ĳ���
Status InitList(ISOE_List &isoelist)
{
	ISOE_Heads* pTemp=(ISOE_Heads*)calloc(MAXLEN,sizeof(ISOE_Heads));
	if(pTemp==NULL)return ERROR;
	isoelist.base=pTemp;
	isoelist.listsize=MAXLEN;
	isoelist.length=0;
	return OK;
}
Status InsertList(ISOE_List &isoelist,ISOE_Heads heads)	//ֻ��β���ϲ���
{
	if(isoelist.length>=isoelist.listsize)
	{
		//�����������ƣ������·����ڴ�
		ISOE_Heads *newbase=(ISOE_Heads*)realloc(isoelist.base,(isoelist.listsize+INC)*sizeof(ISOE_Heads));
		if(!newbase)return ERROR;
		isoelist.base=newbase;
		isoelist.listsize+=INC;
	}
	isoelist.base[isoelist.length]=heads;
	isoelist.length++;
	return OK;
}
Status GetList(ISOE_List& L,int i,ISOE_Heads& record)		//����ָ������i��ȡһ����¼
{
	if(i<1||i>L.length)return ERROR;
	record=L.base[i-1];
	return OK;
}
//��ISOE����Ĳ���
//����(���ڿ�ͷ)
Status InsertISOE_Chain(ISOE_List &L,int idx,ElemType_Vex* pVex)
{
	ISOE_ChainNode *pNewNode;
	if((pNewNode=(ISOE_ChainNode*)malloc(sizeof(ISOE_ChainNode)))==NULL)return ERROR;
	pNewNode->pVex=pVex;
	pNewNode->next=L.base[idx-1].ChainHead;
	L.base[idx-1].ChainHead=pNewNode;
	return OK;
}
//������ISOE��ɾ��
//�����豸��
Status DeleteISOE_AMList(ISOE_List &L,int isoe_i,int isoe_j,ID_Type id);
 void FindPwrLine(ISOE_List &L,BiTree &pNode)
 {
	if(pNode==NULL)return;
	if(((ElemType_Deg2*)pNode->data)->type==DEG2)
	{
		if((((ElemType_Deg2*)pNode->data)->X>=1e-6||((ElemType_Deg2*)pNode->data)->X<=-1e-6)&&((ElemType_Deg2*)pNode->data)->status==ON)
		{
			//�Ƕ����豸�����š��е翹,��ɾ��
			DeleteISOE_AMList(L,((ElemType_Deg2*)(pNode->data))->bus_A->isoe,((ElemType_Deg2*)(pNode->data))->bus_B->isoe,((ElemType_Deg2*)(pNode->data))->id);
		}
	}
	//������
	FindPwrLine(L,pNode->left);
	FindPwrLine(L,pNode->right);
	return;
 }
Status DestoryList(ISOE_List& L,BiTree &EquipTree)
{
	ISOE_ChainNode *pNode,*ptemp;
	//��ɾ����
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
	//��ɾ�ߣ����豸���б������ŵ������·
	FindPwrLine(L,EquipTree);
	//Ȼ��ŵ�����
	free(L.base);
	L.base=NULL;
	return OK;
}

//����ߣ����ڿ�ͷ��
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
//ɾ����
Status DeleteISOE_AMList(ISOE_List &L,int isoe_i,int isoe_j,ID_Type id)
{
	if(L.base[isoe_i-1].AMListHead==NULL||L.base[isoe_j-1].AMListHead==NULL)return ERROR;
	
	//��A���Ĳ���
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
		//�ҵ���
		pLast->link_A=pCur->link_A;
	}

	//��B���Ĳ���
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
		//�ҵ���
		pLast->link_B=pCur->link_B;
		free(pCur);
		return OK;
	}
}

//update��3����
//Vextree->isoelec graph
//isoelec graph->island graph
//isoelec graph->matrix
//���϶���Ϊ��һ�����̵�

/*
typedef struct RecordNode
{
	ElemType_Vex* pVex;
	RecordNode* next;
}RecordNode;

//����VexTree
Status traverseVexTree(Node* T,RecordNode* VexTable)
{
	if(T==NULL)return;
	//����:����
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
	//���±���
	traverseVexTree(T->left,VexTable);
	traverseVexTree(T->right,VexTable);
}
*/

//������ߵ�����
typedef struct pwrList
{
	AMListNode* pPwrline;
	ElemType_Deg2* pEquip;
	pwrList* next;
}pwrList;

//�ݹ������Node��visited��Ϊfalse
void unvisited(Node* T)
{
	if(T==NULL)return;
	((ElemType_Vex*)T->data)->visited=false;
	unvisited(T->left);
	unvisited(T->right);
	return;
}

//�ݹ��pVex����֧ͨ�ҳ���(ע�⣡���ﲻ֧���Ի���)
void visitVex(DataBase &DB,ElemType_Vex* pVex,int &cur_isoe_idx,ISOE_List &L,pwrList* &pwrlist)
{
	//T�ڵ�cur_isoe_idx���ڵ�/��֧ͨ/�ȵ�λ��/�����ϣ�����Ӧ����
	//��vex�������
	pVex->isoe=cur_isoe_idx;
	//��Ŀ��isoelecͼ���ı�
	InsertISOE_Chain(L,cur_isoe_idx,pVex);
	//visited��Ϊtrue
	pVex->visited=true;

	//�������ر�����һ��
	AMListNode* pEdge=pVex->Deg2_Equips;
	ElemType_Vex* pNextVex;
	while(pEdge!=NULL)
	{
		//��ȷ��pEdge�Ƿ���á��Ƿ��������·���������·�ʹ浽������û�����
		Node* pNode=FindAVL_ID(DB.EquipTree,pEdge->id);
		if(pNode==NULL)return;
		if(((ElemType_Deg2*)pNode->data)->X>=1e-6||((ElemType_Deg2*)pNode->data)->X<=-1e-6)
		{
			//X!=0,��status==ON,�Ҷ���ĵ㻹δ���ʣ����ظ����ʣ���������·
			if(((ElemType_Deg2*)pNode->data)->status==ON)
			{
				ElemType_Vex* pOther=pEdge->vex_A!=pVex?pEdge->vex_A:pEdge->vex_B;		//pOther��������߶����Vex���
				//���ý��δ���ʣ�����������
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
			//X==0���ǱպϹ���״̬
			//�ҵ�pedge����һ�����
			pNextVex=pEdge->vex_A!=pVex?pEdge->vex_A:pEdge->vex_B;
			//���ý��δ���ʣ���visit�ý��
			if(pNextVex->visited==false)
				visitVex(DB,pNextVex,cur_isoe_idx,L,pwrlist);
		}
		
		//��һ����
		pEdge=pEdge->vex_A==pVex?pEdge->link_A:pEdge->link_B;
	}
}

//��Vextree�ϵݹ����
void visit(DataBase &DB,Node* pNode,int &cur_isoe_idx,ISOE_List &L,pwrList* &pwrlist)
{
	if(pNode==NULL)return;
	//δ����������һ��
	if(((ElemType_Vex*)pNode->data)->visited==false)
	{
		cur_isoe_idx++;
		ISOE_Heads heads={NULL,NULL};
		InsertList(L,heads);
		//�ҵ����vex������֧ͨ
		visitVex(DB,(ElemType_Vex*)pNode->data,cur_isoe_idx,L,pwrlist);
	}
	//���ѱ���/δ�������Ѵ�����������
	visit(DB,pNode->left,cur_isoe_idx,L,pwrlist);
	visit(DB,pNode->right,cur_isoe_idx,L,pwrlist);
	return;
}

//��һ�����ڵ㣨�ȵ��Ƶ�/isoelec��ͼ����
Status Update_step1(dataBase &DB,ISOE_List &L)
{
	
	//��ʶ��ǰ����̽��ڼ����ȵ�λ���ں���Vex���
	int cur_isoe_idx=0;		
	
	//����һ��VexTree����������־mark��0
	unvisited(DB.VexTree);

	//��ʼ��Ŀ��ͼ
	InitList(L);

	//��ʼ��������ݴ���
	pwrList* pwrlist=NULL;

	//��ʼ����,��Vextree���б���
	visit(DB,DB.VexTree,cur_isoe_idx,L,pwrlist);

	//��һ����������ߵı�
	pwrList* pnext;
	if(pwrlist!=NULL)
	{
		//�����NULL����û������ߣ��������Ӳ���
		//�����������������isoe����
		pnext=pwrlist;
		
		while(pnext!=NULL)
		{
		pnext=pnext->next;
		//��pwrlist��Ӧ�������
		InsertISOE_AMList(L,pwrlist->pPwrline->vex_A->isoe,pwrlist->pPwrline->vex_B->isoe,pwrlist->pEquip);
		free(pwrlist);
		pwrlist=pnext;
		}
	}
	//���
	return OK;

}

//����Ϊ�ڶ��׶�׼��
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

//�������
Status InsertISLAND_Chain(ISLAND_List &IL,int island_idx,int isoe_idx)
{
	//�����ڴ�
	ISLAND_ChainNode* pnewnode=(ISLAND_ChainNode*)malloc(sizeof(ISLAND_ChainNode));
	if(pnewnode==NULL)return ERROR;

	//���ڵ�һ��
	pnewnode->isoe_idx=isoe_idx;
	pnewnode->next=IL.base[island_idx-1].pHead;
	IL.base[island_idx-1].pHead=pnewnode;
	IL.base[island_idx-1].isoc_num++;
	return OK;
}

//���Ա��ʼ��
Status InitISLAND_List(ISLAND_List& IL) 
{
  // ����һ���յ�˳�����Ա�L��
	IL.base = (ISLAND_ChainHead*)calloc(MAXLEN,sizeof(ISLAND_ChainHead));
	if (IL.base==NULL) return ERROR;        // �洢����ʧ��
	IL.length = 0;
	IL.listsize = MAXLEN;
	return OK;
} 

//���Ա�����
Status SizeupISLAND_List(ISLAND_List& IL)
{
	if(IL.length>=IL.listsize)
	{
		//�����������ƣ������·����ڴ�
		ISLAND_ChainHead *newbase=(ISLAND_ChainHead*)realloc(IL.base,(IL.listsize+INC)*sizeof(ISLAND_ChainHead));
		if(!newbase)return ERROR;
		IL.base=newbase;
		IL.listsize+=INC;
	}
	IL.length++;
	return OK;
}

//���Ա��
Status GetISLAND(ISLAND_List& IL,int i,ISLAND_ChainHead& record)		//����ָ������i��ȡһ����¼
{
	if(i<1||i>IL.length)return ERROR;
	record=IL.base[i-1];
	return OK;
}

//�������Ա�
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

//�Ե�ǰ�ڵ�/�ȵ�λ��dfs����֧ͨ�ҳ���
Status visitISOE(ISOE_List &L,int isoe_idx,ISLAND_List &IL,int island_idx)
{
	int next_isoe_idx;
	//�Ȱ��Լ����ISLAND

	InsertISLAND_Chain(IL,island_idx,isoe_idx);
	L.base[isoe_idx-1].visited=true;

	//������ý�����ڵı�,�Ӷ��ҵ��ڼ������ڼ���ÿ���㶼�ݹ�һ��
	ISOE_AMListNode* pEdge=L.base[isoe_idx-1].AMListHead;
	while(pEdge!=NULL)
	{
		//�����߶�Ӧ���ڽ��
		next_isoe_idx=pEdge->isoe_A!=isoe_idx?pEdge->isoe_A:pEdge->isoe_B;

		//�ݹ�һ��
		if(L.base[next_isoe_idx-1].visited==false)
		{

			visitISOE(L,next_isoe_idx,IL,island_idx);
		}

		//��һ����
		pEdge=pEdge->isoe_A==isoe_idx?pEdge->link_A:pEdge->link_B;
	}
	//�ѷ�����ֱ���˳�
	return OK;
}

//�ڶ���������L�������˵�/�µ�
Status Update_step2(ISLAND_List &IL,ISOE_List &L)
{
	//�Ȱ�visited��0��calloc�������ɵ�L���ù飩
	int isoe_idx;
	//��ʼ��
	InitISLAND_List(IL);
	//Ȼ�����L���ڵ�
	int island_idx=0;
	for(isoe_idx=1;isoe_idx<=L.length;isoe_idx++)
	{
		//���ʹ��˾Ͳ��ٷ���
		if(L.base[isoe_idx-1].visited==false)
		{
			//û���ʹ��Ϳ�ʼ�Դ˵㿪ʼDPS
			island_idx++;
			SizeupISLAND_List(IL);
			//����¿��ٵ�List�ڴ�������
			visitISOE(L,isoe_idx,IL,island_idx);
		}
	}

	//����:�����٣�������򵥵�ð�ݣ����ף�
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

//���ڵ��ɾ���
//���ṹ��
typedef struct OLNode
{
	int i,j;
	float e;
	OLNode *right,*down;
}OLNode,* OLink;
//����ṹ��
typedef struct
{
	OLink *rhead,*chead;	//r=row��c=column
	int n,tu;
}CrossList;
//��ʼ��
Status InitCrossList(CrossList &M,int n)
{
	//��ʱֻ����M�Ǹոն��壬�ڲ�Ϊ�յ�����
	//���M�ǿտ��ܳ���bug------------------------------------------------------
	M.n=n;
	M.tu=0;
	OLink *phead;
	//�С���ͷָ���ʼ��
	phead=(OLink*)calloc(n,sizeof(OLink));
	if(phead==NULL)return ERROR;
	M.chead=phead;
	phead=(OLink*)calloc(n,sizeof(OLink));
	if(phead==NULL)return ERROR;
	M.rhead=phead;
}
//���پ���
Status DestoryCrossList(CrossList &M)
{
	OLNode *pElem,*pNext;
	int i;
	//һ��һ��ɾ��
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
//��ʼ��Ԫ�أ���e=0������Ӧ
Status Insert_symm(CrossList &M,int i,int j,float e)
{
	//������
	if(i<1||j<1||i>M.n||j>M.n)return ERROR;
	if(e==0)return OK;
	M.tu++;
	if(i<j)		//��֤i>=j���Ӷ�ֻ�洢�������Ǿ����ϣ���ͬ
	{
		int temp;
		temp=i;
		i=j;
		j=temp;
	}

	//�ȹ����½��
	OLink pnode,psearch;
	if(!(pnode=(OLNode*)calloc(1,sizeof(OLNode))))return ERROR;
	pnode->e=e;
	pnode->i=i;
	pnode->j=j;
	//�����½���down�����¸����������㷨��
	if(M.chead[j-1]==NULL||M.chead[j-1]->i>i)
	{
		pnode->down=M.chead[j-1];
		M.chead[j-1]=pnode;
	}
	else
	{
		//ֱ��psearchָ��Ŀ���ǰ���ϡ���һ�����
		for(psearch=M.chead[j-1];(psearch->down!=NULL)&&psearch->down->i<i;psearch=psearch->down){};
		//ͬһ��i
		if(psearch->down!=NULL&&psearch->down->i==i)
		{
			psearch->down->e=e;
			M.tu--;
			free(pnode);
			return OK;
		}
		//��ͬ��i
		pnode->down=psearch->down;
		psearch->down=pnode;
	}
	//�����½���right
	if(M.rhead[i-1]==NULL||M.rhead[i-1]->j>j)
	{
		pnode->right=M.rhead[i-1];
		M.rhead[i-1]=pnode;
	}
	else
	{
		for(psearch=M.rhead[i-1];(psearch->right!=NULL)&&psearch->right->j<j;psearch=psearch->right){};
		//�������ٳ���ͬһ��j
		//��ͬ��j
		pnode->right=psearch->right;
		psearch->right=pnode;
	}
	return OK;
}
//ȡԪ��
Status Get_symm(CrossList &M,int i,int j,float &a)
{
	if(i<1||j<1||i>M.n||j>M.n)return ERROR;
	if(i<j)		//��֤i>=j���Ӷ�ֻ��ȡ�����Ǿ���
	{
		int temp;
		temp=i;
		i=j;
		j=temp;
	}
	OLink psearch;
	//�������㷨��ͬ
	//�ص��עÿ��/�е�һ������Ŀ������������ʱ����������Ч
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

//�����������㵼�ɾ���
float CrossListATij(ISOE_List &L,int i,int j)
{
	float mat_elem=0.0f;
	//�ҵ�����i-j������ߣ����ܲ�ֹһ����
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
	//����ǶԽ���Ԫ��
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
		//����Խ���Ԫ��
		Insert_symm(M,i,i,mat_diag);
	}
	return OK;
}

//��ؽڵ����µĺ����ӿ�
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

