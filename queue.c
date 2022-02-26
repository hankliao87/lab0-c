#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

#ifndef min
#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head == NULL)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;

    element_t *elem, *safe;
    list_for_each_entry_safe (elem, safe, l, list) {
        list_del(&elem->list);
        // free(&elem->list);
        q_release_element(elem);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *elem = malloc(sizeof(element_t));
    if (elem == NULL)
        return false;

    size_t length = strlen(s) + 1;
    elem->value = malloc(sizeof(char) * length);
    strncpy(elem->value, s, length);
    list_add(&elem->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *elem = malloc(sizeof(element_t));
    if (elem == NULL)
        return false;

    size_t length = strlen(s) + 1;
    elem->value = malloc(sizeof(char) * length);
    strncpy(elem->value, s, length);
    list_add_tail(&elem->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *elem = list_first_entry(head, element_t, list);
    list_del_init(&elem->list);
    if (sp != NULL) {
        strncpy(sp, elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return elem;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *elem = list_last_entry(head, element_t, list);
    list_del_init(&elem->list);
    if (sp != NULL) {
        strncpy(sp, elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return elem;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *node;
    list_for_each (node, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head == NULL || list_empty(head))
        return false;

    struct list_head *slow = head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;

    element_t *elem = list_entry(slow, element_t, list);
    list_del(&elem->list);
    q_release_element(elem);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head == NULL)
        return false;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        bool remove_dup = false;
        if (safe == head)
            break;
        element_t *elem1 = list_entry(node, element_t, list);
        element_t *elem2 = list_entry(safe, element_t, list);
        size_t len_value = min(sizeof(elem1->value), sizeof(elem2->value));
        while (strncmp(elem1->value, elem2->value, len_value) == 0) {
            remove_dup = true;
            list_del(&elem1->list);
            q_release_element(elem1);
            node = safe;
            safe = node->next;
            if (safe == head)
                break;
            elem1 = list_entry(node, element_t, list);
            elem2 = list_entry(safe, element_t, list);
            len_value = min(sizeof(elem1->value), sizeof(elem2->value));
        }
        if (remove_dup) {
            remove_dup = false;
            elem1 = list_entry(node, element_t, list);
            list_del(&elem1->list);
            q_release_element(elem1);
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    struct list_head **ptr = &head->next, *tmp = NULL;
    while (*ptr != head && (*ptr)->next != head) {
        tmp = *ptr;
        *ptr = (*ptr)->next;

        tmp->prev->next = *ptr;
        (*ptr)->next->prev = tmp;

        tmp->next = (*ptr)->next;
        (*ptr)->next = tmp;
        (*ptr)->prev = tmp->prev;
        tmp->prev = (*ptr);

        ptr = &(*ptr)->next->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 * TODO:
 * https://hackmd.io/@sysprog/c-linked-list#Linux-%E6%A0%B8%E5%BF%83%E7%9A%84-list_sort-%E5%AF%A6%E4%BD%9C
 */
struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    if (!l1)
        return l2;
    if (!l2)
        return l1;

    struct list_head *head = NULL, **ptr = &head, **node;

    for (node = NULL; l1 && l2; *node = (*node)->next) {
        element_t *elem1 = list_entry(l1, element_t, list);
        element_t *elem2 = list_entry(l2, element_t, list);
        size_t len_value = min(sizeof(elem1->value), sizeof(elem2->value));

        node = (strncmp(elem1->value, elem2->value, len_value) < 0) ? &l1 : &l2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (l2 == NULL) ? l1 : l2;

    return head;
}

struct list_head *q_sort_helper(struct list_head *head)
{
    if (head == NULL || head->next == NULL)
        return head;

    struct list_head *slow = head, *fast = head->next;

    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }

    struct list_head *mid = slow->next;
    slow->next = NULL;

    struct list_head *l1 = q_sort_helper(head);
    struct list_head *l2 = q_sort_helper(mid);

    return merge(l1, l2);
}

void q_sort(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    head->prev->next = NULL;

    struct list_head *node = q_sort_helper(head->next);
    head->next = node;

    struct list_head *ptr = head;
    while (node != NULL) {
        node->prev = ptr;
        ptr = node;
        node = node->next;
    }
    ptr->next = head;
    head->prev = ptr;
}
