/**
  ******************************************************************************
  * @file    newlist.c
  * @author  MingLiang.Lu
  * @version V1.1.0
  * @date    16-November-2014
  * @brief   for use to sim3U1xxx devices families.
  * @descriptions: This file is writen based on linkerlist.c MingLiang.Lu,
						 10-october-2014 and feel free to let me know 
						 if you have any questions,you can send e-mail to  
						 linux.lucian@gmail.com or www.potevio.com
  ******************************************************************************
  */

#include "newList.h"

#define		FLASE	0
#define		TRUE	1

/*
 *	@brief :�������Ϊ ��ͷ�ڵ� �� û��ͷ�ڵ� ���֡�ʹ����ͷ�ڵ���Ϊ���ӣ��Ƚ�ֱ�ӣ�����������⡣
 */
OS_NEWTASKLIST *pNewHead = NULL; //���������õ�����

/*
 *	@brief :step1 --> ������Ҫһ��ͷ���Ǳ�Ҫ������;������ͼ�������ϵ�Ŀ¼��ǩһ��
 *					  				����ARM�����������ҵ���Ƭ�ڴ棬�����ڶ������ʱ����;�����ԡ�
 *
 */
 
elemType createNewList( void )
{
	pNewHead = (OS_NEWTASKLIST *)malloc(sizeof(OS_NEWTASKLIST));
	
	if(NULL == pNewHead){
		return FLASE;
	}else{
		Initialize_a_node(pNewHead);
		pNewHead->next = NULL;
		return TRUE;
	}
}

/*
 *	@brief :����һ���ڵ�,ʵ��ʹ���о������ǵ�һ֡����
 *
 *	D:\github\vc 6.0\sim3U1xx\linker.c(44) : warning C4715: 'add_a_new_node' : not all control paths return a value
 */

elemType add_a_new_node( elemType *src,elemType type,elemShort len,elemType stat )
{
	unsigned char *pSize = NULL;
	OS_NEWTASKLIST* newNode = NULL;
	
	if(NULL == pNewHead){
		return FLASE;
	}

	
	pSize = (unsigned char *)malloc(len*sizeof(unsigned char));
	if(pSize == NULL){
		return FLASE;
	}
	newNode = (OS_NEWTASKLIST*)malloc(sizeof(OS_NEWTASKLIST));
	if(NULL == newNode){
		return FLASE;
	}else{		
		Initialize_a_node(newNode);	
		newNode->data = pSize;	
		newNode->nPriority = type;			
		newNode->dataLen = len;
		newNode->NodeStat = stat;	
		memcpy(newNode->data,src,len);
		newNode->next = NULL;	
	}
	//������������в���ڵ�
	sort_list_from_priority(newNode);

	return TRUE;	
}

/*
 *	@brief : ɾ��һ���ڵ�
 *
 */
elemType  remove_a_old_node( void )
{    	
	 OS_NEWTASKLIST* p = pNewHead->next;
   OS_NEWTASKLIST* q = pNewHead;  
	
   if(NULL == pNewHead){              
      return FLASE;
   }



   while(NULL != p){
      if(DATA_SEND == p->NodeStat){
         free(p->data);
         p->data = NULL;
         if(p->next == NULL){
            q->next = NULL;
         }else{
            q->next = p->next;         //�� ��ɾ���Ľڵ� �ĺ�һ���ڵ��������ֵ��ֵ����ɾ���ڵ��ǰһ���ڵ��ָ����
         }  
         free(p);                      //�ǵ�һ��Ҫ�ͷŵ���������ڴ棬�������ڴ����Ϊ�ô��������±�ʹ��
         p = NULL;                     //����Ҳ����Ҫ��Ϊ���Ͻ�������ͷŵ�p��ָ��ı�־λ����Ҫ������ΪNULL,�Է�ֹ����	
      }else{
         q = p;
         p = p->next;
      }
   }

   return TRUE;
}


/*
 *	@brief : ��ʼ��һ���ڵ㣬������ĳ�Ա�����ʼֵ
 *
 */
void   Initialize_a_node(OS_NEWTASKLIST* node)
{
	node->data = NULL;
	node->dataLen = 0;
	node->NodeStat = 0;	
	node->nPriority = 0;	
}


/*
 *	@brief : �ж�һ�������ǲ��ǿյģ���;�����б���������û�����ݣ���ֱ�ӷ���Ѳ������
 *
 */
elemType  Is_this_EmptyList( void )
{
	if(NULL == pNewHead->next){
		return TRUE;
	}else{
		return FLASE;
	}
}


/*
 *	@brief : �������ȼ������µĽڵ�
 *
 */
void sort_list_from_priority(OS_NEWTASKLIST* node)
{
	OS_NEWTASKLIST* p = NULL;
	OS_NEWTASKLIST* q = NULL; 
	
	p = pNewHead->next;
	q = pNewHead;
	
	while( NULL != p){		
		if(p->nPriority > node->nPriority){
			//�������ǰ�ڵ�����ȼ���Ҫ����ĵͣ���ô�Ͱѵ�ǰ�ڵ����һ���ڵ�ĵ�ַ��node,����node�������ǰ�ڵ�
			node->next = q->next;
			q->next = node;
			return;
		}
		q = p;
		p = p->next;		
	}
	//�����ǰ����û�б�node�����ȼ��͵ģ���ô��ֻ�ü���β��
	q->next = node;
	node->next = NULL;	
}

//------------------------------------------end of file----------------------------------
