/*
 * listof.h
 */

#ifndef __LISTOF_H__
#define __LISTOF_H__

/* type name for a list of TYPE */
#define LIST_TYPE(TYPE)	struct list_of_ ## TYPE

/*
 * define list type
 * for the head of the list, prev points to the tail (=> fast append)
 * for the tail of the list, next is NULL
 */
#define DEFINE_LIST_OF(TYPE)	\
LIST_TYPE(TYPE)			\
{				\
	LIST_TYPE(TYPE) *prev;	\
	LIST_TYPE(TYPE) *next;	\
	TYPE item;		\
}

/*
 * declare list type
 * need to do DEFINE_LIST_OF(); first
 * variable should be a ptr, eg: LIST_OF(int) *int_list_head;
 */
#define LIST_OF(TYPE)	LIST_TYPE(TYPE)

/*
 * eg usage:
 * LIST_OF(T) *head = NULL;
 * LIST_TYPE(T) *new = alloc(sizeof(LIST_TYPE(T)));
 * [set new->item.xxx]
 * LIST_APPEND(&head, new);
 */
#define LIST_APPEND(PHEAD, ITEM)			\
do							\
{							\
	/* is it the first item */			\
	if((*(PHEAD)) == NULL)				\
	{						\
		*(PHEAD) = ITEM;			\
		/* tail is also the head */		\
		(ITEM)->prev = ITEM;			\
		(ITEM)->next = NULL;			\
	}						\
	else						\
	{						\
		/* add it to the tail */		\
		(ITEM)->prev = (*(PHEAD))->prev;	\
		(ITEM)->next = NULL;			\
		(*(PHEAD))->prev->next = ITEM;		\
		(*(PHEAD))->prev = ITEM;		\
	}						\
}							\
while(0)

/*
 * make ITEM the new head of the list
 */
#define LIST_PREPEND(PHEAD, ITEM)			\
do							\
{							\
	/* is it the first item */			\
	if((*(PHEAD)) == NULL)				\
	{						\
		*(PHEAD) = ITEM;			\
		/* tail is also the head */		\
		(ITEM)->prev = ITEM;			\
		(ITEM)->next = NULL;			\
	}						\
	else						\
	{						\
		/* make it the new head */		\
		(ITEM)->prev = (*(PHEAD))->prev;	\
		(ITEM)->next = *(PHEAD);		\
		(*(PHEAD))->prev = ITEM;		\
		*(PHEAD) = ITEM;			\
	}						\
}							\
while(0)

/*
 * remove the given LIST_TYPE from the list
 * doesn't check that ITEM is actually on the list
 * doesn't check that *PHEAD is not NULL
 */
#define LIST_REMOVE(PHEAD, ITEM)				\
do								\
{								\
	/* is ITEM the head and the tail */			\
	if(*(PHEAD) == ITEM && (ITEM)->next == NULL)		\
	{							\
		*(PHEAD) = NULL;				\
	}							\
	/* is ITEM just the head */				\
	else if(*(PHEAD) == ITEM)				\
	{							\
		/* make new head->prev point to the tail */	\
		(*(PHEAD))->next->prev = (*(PHEAD))->prev;	\
		*(PHEAD) = (*(PHEAD))->next;			\
	}							\
	/* is ITEM just the tail */				\
	else if((ITEM)->next == NULL)				\
	{							\
		/* make head->prev point to the new tail */	\
		(*(PHEAD))->prev = (ITEM)->prev;		\
		/* unlink ITEM from the new tail */		\
		(ITEM)->prev->next = NULL;			\
	}							\
	else							\
	{							\
		/* unlink ITEM from the previous one */		\
		(ITEM)->prev->next = (ITEM)->next;		\
		/* unlink ITEM from the next one */		\
		(ITEM)->next->prev = (ITEM)->prev;		\
	}							\
}								\
while(0)

/*
 * inserts ITEM into the list after item AFTER
 * ITEM must not currently be on any list
 */
#define LIST_INSERT_AFTER(PHEAD, ITEM, AFTER)	\
do						\
{						\
	/* is AFTER currently the tail */	\
	if((AFTER)->next == NULL)		\
	{					\
		/* make ITEM the tail */	\
		(*(PHEAD))->prev = ITEM;	\
		/* insert ITEM */		\
		(ITEM)->next = (AFTER)->next;	\
		(ITEM)->prev = AFTER;		\
		(AFTER)->next = ITEM;		\
	}					\
	else					\
	{					\
		/* insert ITEM */		\
		(AFTER)->next->prev = ITEM;	\
		(ITEM)->next = (AFTER)->next;	\
		(ITEM)->prev = AFTER;		\
		(AFTER)->next = ITEM;		\
	}					\
}						\
while(0)

/*
 * inserts ITEM into the list before item BEFORE
 * ITEM must not currently be on any list
 */
#define LIST_INSERT_BEFORE(PHEAD, ITEM, BEFORE)	\
do						\
{						\
	/* is BEFORE currently the head */	\
	if(*(PHEAD) == BEFORE)			\
	{					\
		/* make ITEM the new head */	\
		*(PHEAD) = ITEM;		\
		/* insert ITEM */		\
		(ITEM)->next = BEFORE;		\
		(ITEM)->prev = (BEFORE)->prev;	\
		(BEFORE)->prev = ITEM;		\
	}					\
	else					\
	{					\
		/* insert ITEM */		\
		(BEFORE)->prev->next = ITEM;	\
		(ITEM)->next = BEFORE;		\
		(ITEM)->prev = (BEFORE)->prev;	\
		(BEFORE)->prev = ITEM;		\
	}					\
}						\
while(0)

/*
 * appends the second list to the tail of the first
 */
#define LIST_CONCAT(PHEAD1, TYPE, HEAD2)			\
do								\
{								\
	if(*(PHEAD1) == NULL)					\
	{							\
		*(PHEAD1) = HEAD2;				\
	}							\
	else if(HEAD2 != NULL)					\
	{							\
		LIST_TYPE(TYPE) *old_tail = (*(PHEAD1))->prev;	\
		/* set tail of *PHEAD1 to tail of HEAD2 */	\
		(*(PHEAD1))->prev = (HEAD2)->prev;		\
		/* attach HEAD2 to old tail of *PHEAD1 */	\
		old_tail->next = HEAD2;				\
		(HEAD2)->prev = old_tail;			\
	}							\
}								\
while(0)

/*
 * removes the head of the list
 * calls FREE_ITEM on removed->item
 * calls FREE_LIST on removed
 */
#define LIST_FREE_HEAD_ITEM(PHEAD, TYPE, FREE_ITEM, FREE_LIST)	\
do								\
{								\
	LIST_TYPE(TYPE) *removed = *(PHEAD);			\
	if(removed != NULL)					\
	{							\
		FREE_ITEM(removed->item);			\
		*(PHEAD) = removed->next;			\
		if(*(PHEAD) != NULL)				\
			(*(PHEAD))->prev = removed->prev;	\
		FREE_LIST(removed);				\
	}							\
}								\
while(0)

/*
 * removes the head of the list
 * calls FREE_LIST on the removed head
 */
#define LIST_FREE_HEAD(PHEAD, TYPE, FREE_LIST)			\
do								\
{								\
	LIST_TYPE(TYPE) *removed = *(PHEAD);			\
	if(removed != NULL)					\
	{							\
		*(PHEAD) = removed->next;			\
		if(*(PHEAD) != NULL)				\
			(*(PHEAD))->prev = removed->prev;	\
		FREE_LIST(removed);				\
	}							\
}								\
while(0)

/*
 * calls FREE_ITEM on each LIST_TYPE().item
 * calls FREE_LIST on each LIST_TYPE()
 * sets *PHEAD to NULL
 */
#define LIST_FREE_ITEMS(PHEAD, TYPE, FREE_ITEM, FREE_LIST)	\
do								\
{								\
	LIST_TYPE(TYPE) *head, *next;				\
	head = *(PHEAD);					\
	while(head)						\
	{							\
		next = head->next;				\
		FREE_ITEM(&head->item);				\
		FREE_LIST(head);				\
		head = next;					\
	}							\
	*(PHEAD) = NULL;					\
}								\
while(0)

/*
 * calls FREE_LIST on each LIST_TYPE()
 * sets *PHEAD to NULL
 */
#define LIST_FREE(PHEAD, TYPE, FREE_LIST)	\
do						\
{						\
	LIST_TYPE(TYPE) *head, *next;		\
	head = *(PHEAD);			\
	while(head)				\
	{					\
		next = head->next;		\
		FREE_LIST(head);		\
		head = next;			\
	}					\
	*(PHEAD) = NULL;			\
}						\
while(0)

/* common lists */
DEFINE_LIST_OF(int);

#endif	/* __LISTOF_H__ */

