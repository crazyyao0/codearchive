#ifndef LIST_HEADER_INCLUDED
#define LIST_HEADER_INCLUDED
typedef struct list_head {
	struct list_head *next, *prev;
}list_head;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = { &name, &name }

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static _inline void __list_add(void * add,
	void * prev,
	void * next)
{
	((struct list_head*)next)->prev = (struct list_head*)add;
	((struct list_head*)add)->next = (struct list_head*)next;
	((struct list_head*)add)->prev = (struct list_head*)prev;
	((struct list_head*)prev)->next = (struct list_head*)add;
}

/*
 * Insert a new entry after the specified head..
 */
static _inline void list_add(void *add, void *head)
{
	__list_add(add, head, ((struct list_head*)head)->next);
}

/*
 * Insert a new entry at the tail
 */
static _inline void list_add_tail(void *add, void *head)
{
	__list_add(add, ((struct list_head*)head)->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static _inline void __list_del(void * prev, void * next)
{
	((struct list_head*)next)->prev = (struct list_head*)prev;
	((struct list_head*)prev)->next = (struct list_head*)next;
}

static _inline void list_del(void *entry)
{
	__list_del(((struct list_head*)entry)->prev, ((struct list_head*)entry)->next);
}

static _inline int list_empty(void *head)
{
	return ((struct list_head*)head)->next == (struct list_head*)head;
}

/*
 * Splice in "list" into "head"
 */
static _inline void list_splice(struct list_head *list, struct list_head *head)
{
	struct list_head *first = list->next;

	if (first != list) {
		struct list_head *last = list->prev;
		struct list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
