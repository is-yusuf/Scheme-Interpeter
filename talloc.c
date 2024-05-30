#include <stdlib.h>
#include "item.h"
#include <stdio.h>

#ifndef TALLOC_H
#define TALLOC_H

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
int iniitalized = 0;
Item *head;

// Takes a car and cdr and creates a cons type item node with the car and cdr.
Item *local_cons(Item *newCar, Item *newCdr)
{
    Item *item = malloc(sizeof(Item));
    item->type = CONS_TYPE;
    item->c.car = newCar;
    item->c.cdr = newCdr;
    return item;
}

// Identical to malloc, takes a size and returns a pointer to allocated memory of that size with the difference it has an underlying garbage collector to free memory after execution
void *talloc(size_t size)
{
    if (!iniitalized)
    {
        iniitalized = 1;
        head = malloc(sizeof(Item));
        head->type = CONS_TYPE;
        Item *item = malloc(sizeof(Item));
        item->type = NULL_TYPE;
        head->c.cdr = item;
        head->c.car = NULL;
    }
    Item *item = malloc(sizeof(Item));
    item->type = PTR_TYPE;
    item->p = malloc(size);
    head = local_cons(item, head);
    return item->p;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree()
{
    int i = 0;
    while (head != NULL && head->type != NULL_TYPE)
    {
        i++;
        if (head->c.car != NULL)
        {
            free(head->c.car->p);
            free(head->c.car);
        }
        Item *temp = head->c.cdr;
        free(head);
        head = temp;
    }
    free(head);
    iniitalized = 0;
}

// Takes a status code and frees the allocated memory before exiting with the status code given
void texit(int status)
{
    tfree();
    exit(status);
}

#endif
