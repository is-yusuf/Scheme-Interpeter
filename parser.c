#include "tokenizer.h"

#include <stdlib.h>

#include <stdio.h>

#include "talloc.h"

#include "linkedlist.h"

#include "parser.h"

#include "string.h"

// stack helper functions

typedef struct

{

    Item *top;

} Stack;

// creates new stack

void initializeStack(Stack *stack)

{

    stack->top = makeNull();
}

// pushes an item to the top of the stack

void push(Stack *stack, Item *item)

{

    // Add item to the top of the stack

    stack->top = cons(item, stack->top);
}

// checks if the stack is empty

int isEmpty(Stack *stack)

{

    return stack->top == NULL || isNull(stack->top);
}

// pops the top item off of the stack

Item *pop(Stack *stack)

{

    if (isEmpty(stack))

    {

        return NULL;
    }

    // Retrieve the top item

    Item *topItem = car(stack->top);

    // Update the stack to remove the top item

    stack->top = cdr(stack->top);

    return topItem;
}

// Takes a (linked) list of tokens from a Scheme program, and returns a pointer to a

// parse tree representing that program.

Item *parse(Item *tokens)

{

    // initialize stack

    Stack stack;

    initializeStack(&stack);

    Stack parenStack;

    initializeStack(&parenStack);

    int parenthesesToClose = 0;

    Item *poppedItem = NULL;

    char prevType;

    // while there are more tokens in the list

    while (!isNull(tokens))

    {

        // get next token

        Item *nextToken;

        nextToken = car(tokens);

        // if next token is not close paren/bracket:

        if (nextToken->type != CLOSE_TYPE && nextToken->type != CLOSEBRACKET_TYPE)

        {

            // if next token is an open paren/bracket

            if (nextToken->type == OPEN_TYPE || nextToken->type == OPENBRACKET_TYPE)

            {

                parenthesesToClose++;

                Item *paren;

                paren = talloc(sizeof(Item));

                if (nextToken->type == OPEN_TYPE)

                {

                    paren->type = OPEN_TYPE;

                    push(&parenStack, paren);

                    // printf("prev type is %c\n", prevType);
                }

                else

                {

                    paren->type = OPENBRACKET_TYPE;

                    push(&parenStack, paren);

                    // push(&parenStack, '[');

                    // printf("prev type is %c\n", prevType);
                }
            }

            // push token to the stack

            push(&stack, nextToken);
        }

        else

        { // nextToken is a close paren/bracket

            if (parenthesesToClose <= 0)

            {

                printf("Syntax error: too many close parentheses\n");

                texit(1);
            }

            // use a stack to track previous parentheses/brackets in order

            Item *lastParen;

            lastParen = pop(&parenStack);

            // if parenthese/brackets don't match up

            if (nextToken->type == CLOSE_TYPE && lastParen->type == OPENBRACKET_TYPE)

            {

                printf("Syntax error: open was a bracket and close was a parentheses\n");

                texit(1);
            }

            if (nextToken->type == CLOSEBRACKET_TYPE && lastParen->type == OPEN_TYPE)

            {

                printf("Syntax error: open was a parentheses and close was a bracket\n");

                texit(1);
            }

            // pop item off the stack

            if (poppedItem == NULL)

            {

                poppedItem = makeNull();
            }

            Item *subTree = poppedItem;

            poppedItem = pop(&stack);

            // while popped item is not an open paren/bracket

            while (poppedItem != NULL && poppedItem->type != OPEN_TYPE && poppedItem->type != OPENBRACKET_TYPE)

            {

                // add popped item to subtree

                subTree = cons(poppedItem, subTree);

                // pop off next item

                poppedItem = pop(&stack);
            }

            // checks the edge case that nothing was put into the subtree

            if (isNull(subTree))

            {

                subTree = cons(subTree, subTree);
            }

            parenthesesToClose--;

            // item was an open paren/bracket so push subtree to stack

            push(&stack, subTree);

            poppedItem = NULL;
        }

        tokens = cdr(tokens);
    }

    if (parenthesesToClose < 0)

    {

        printf("Syntax error: too many close parentheses\n");

        texit(1);
    }

    else if (parenthesesToClose > 0)

    {

        printf("Syntax error: not enough close parentheses\n");

        texit(1);
    }

    // turn stack into a linked list in order of the program

    Item *list = makeNull();

    Item *sExpr;

    while (!isEmpty(&stack))

    {

        sExpr = pop(&stack);

        list = cons(sExpr, list);
    }

    return list;
}

// Prints the tree to the screen in a readable fashion. It should look just like

// Scheme code; use parentheses to indicate subtrees.

void printTree(Item *tree)

{

    if (tree == NULL || isNull(tree))

    {

        // printf("the tree was null\n");

        return;
    }

    // printf("(");

    switch (tree->type)

    {

    case INT_TYPE:

        printf("%i", tree->i);

        break;

    case DOUBLE_TYPE:

        printf("%f", tree->d);

        break;

    case STR_TYPE:

        printf("%s", tree->s);
        break;

    case BOOL_TYPE:

        printf("%s", tree->s);

        break;

    case SYMBOL_TYPE:

        printf("%s", tree->s);

        break;

    case CONS_TYPE:

        // Recursively print children (car and cdr)

        if (car(tree)->type == CONS_TYPE)

        {

            // is internal node?

            printf("(");

            printTree(tree->c.car);

            printf(") ");

            // Changed this on May 24th

            if (cdr(tree)->type != CONS_TYPE && cdr(tree)->type != NULL_TYPE)
            {

                printf(" . ");

                printTree(tree->c.cdr);
            }

            else
            {

                printf(" ");

                printTree(tree->c.cdr);
            }
        }

        else

        {

            printTree(tree->c.car);

            if (tree->c.cdr->type == CONS_TYPE)

            {

                // there is another item after, so it needs the space

                printf(" ");
            }

            else if (!isNull(cdr(tree)))
            {

                printf(" . ");
            }

            printTree(tree->c.cdr);
        }

        break;
    case CLOSURE_TYPE:
        printf("#<procedure>");
        break;
    case PRIMITIVE_TYPE:
        printf("primitive");
        break;
    default:

        break;
    }
}
