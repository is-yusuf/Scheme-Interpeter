#include <stdbool.h>
#include "item.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "linkedlist.h"
#include <assert.h>
#include <string.h>
#include "talloc.h"

// Takes no arguments and returns a new NULL_TYPE item node.
Item *makeNull()
{
    Item *item = talloc(sizeof(Item));
    item->type = NULL_TYPE;
    return item;
}

// Takes a car and cdr and creates a cons type item node with the car and cdr.
Item *cons(Item *newCar, Item *newCdr)
{
    Item *item = talloc(sizeof(Item));
    item->type = CONS_TYPE;
    item->c.car = newCar;
    item->c.cdr = newCdr;
    return item;
}

// Takes an int,double,or str Item node and prints its content followed by a ->
void print(Item *current)
{
    switch (current->type)
    {
    case INT_TYPE:
        printf("%d", current->i);
        printf("->");
        return;
    case DOUBLE_TYPE:
        printf("%f", current->d);
        printf("->");
        break;
    case STR_TYPE:
        printf("%s", current->s);
        printf("->");
        break;
    default:
        break;
    }
}

// Takes the head node of a linkedlist and prints the content as item1 -> item 2 and so on
void display(Item *list)
{
    if (list == NULL)
    {
        return;
    }
    Item *current = list;
    while (!isNull(current))
    {

        if (current->type == CONS_TYPE)
        {
            print(car(current));
            if (current->c.cdr != NULL)
            {
                if (cdr(current) == NULL)
                {
                    return;
                }
                current = cdr(current);
            }
            else
            {
                return;
            }
        }
        else
        {
            print(current);
            return;
        }
    }
}

// Takes an two pointers to Items destination and source and copies the source to destination without pointing to source
void copy(Item *destination, Item *source)
{
    switch (source->type)
    {
    case INT_TYPE:
        destination->type = INT_TYPE;
        destination->i = source->i;
        break;
    case DOUBLE_TYPE:
        destination->type = DOUBLE_TYPE;
        destination->d = source->d;
        break;
    case STR_TYPE:
        destination->type = STR_TYPE;
        destination->s = talloc(sizeof(source->s));
        strcpy(destination->s, source->s);
        break;
    default:
        break;
    }
}

// Takes a pointers to Item head node of a linkedlist and returns a pointer to the reversed linkedlist with no pointers pointing to the original values
Item *reverse(Item *list)
{
    assert(list != NULL);
    if (isNull(list))
    {
        Item *newList;
        newList = makeNull();
        return newList;
    }
    assert(list->type == CONS_TYPE);

    Item *current = list;
    Item *prev = talloc(sizeof(Item));
    prev->type = CONS_TYPE;

    prev->c.cdr = makeNull();
    prev->c.car = NULL;
    int i = 0;
    while (!isNull(current))
    {
        Item *toBeAdded = talloc(sizeof(Item));
        if (current->type == CONS_TYPE)
        {
            toBeAdded = car(current);
            //  copy(toBeAdded, car(current));

            if (prev->c.car != NULL)
            {
                Item *nextprev = talloc(sizeof(Item));
                nextprev->type = CONS_TYPE;
                nextprev->c.cdr = prev;
                prev = nextprev;
            }
            prev->c.car = toBeAdded;
            current = cdr(current);
        }
        else
        {
            toBeAdded = current;
            // copy(toBeAdded, current);
            prev->c.cdr = toBeAdded;
            return prev;
        }
    }
    return prev;
}

// Takes an pointer to Item of type cons_type and returns a pointer to its car. Discontinues the program of invalid input
Item *car(Item *list)
{
    assert(list->type == CONS_TYPE);
    return list->c.car;
}

// Takes an pointer to Item of type cons_type and returns a pointer to its cdr. Discontinues the program of invalid input
Item *cdr(Item *list)
{
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Takes a pointer to Item and returns true if it'n null type and false otherwise
bool isNull(Item *item)
{
    assert(item != NULL);
    return item->type == NULL_TYPE;
}

// Takes a pointer to Item head and returns the length of the linkedlist
int length(Item *list)
{
    assert(list != NULL);

    Item *current = list;
    int i = 0;

    while (!isNull(current))
    {
        if (current->type == CONS_TYPE)
        {
            if (cdr(current) == NULL)
            {
                return i + 1;
            }
            i++;
            current = cdr(current);
        }
        else
        {
            return i + 1;
        }
    }
    return i;
}
