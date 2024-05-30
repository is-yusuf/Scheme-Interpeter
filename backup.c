#include "tokenizer.h"
#include <stdlib.h>
#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include "parser.h"
#include "string.h"
#include "interpreter.h"

Frame *top_frame;
Item *getsymbolfromframe(char *symbol, Frame *frame);
void evaluationError(char *error);
void copy_item(Item *destination, Item *source);

// Takes a pointer to item and prints the type of the item for debugging purposes
void print_type(Item *item)
{
    switch (item->type)
    {
    case INT_TYPE:
        printf("INT_TYPE\n");
        break;
    case DOUBLE_TYPE:
        printf("DOUBLE_TYPE\n");
        break;
    case STR_TYPE:
        printf("STR_TYPE\n");
        break;
    case CONS_TYPE:
        printf("CONS_TYPE\n");
        break;
    case NULL_TYPE:
        printf("NULL_TYPE\n");
        break;
    case PTR_TYPE:
        printf("PTR_TYPE\n");
        break;
    case OPEN_TYPE:
        printf("OPEN_TYPE\n");
        break;
    case CLOSE_TYPE:
        printf("CLOSE_TYPE\n");
        break;
    case BOOL_TYPE:
        printf("BOOL_TYPE\n");
        break;
    case SYMBOL_TYPE:
        printf("SYMBOL_TYPE\n");
        break;
    case OPENBRACKET_TYPE:
        printf("OPENBRACKET_TYPE\n");
        break;
    case CLOSEBRACKET_TYPE:
        printf("CLOSEBRACKET_TYPE\n");
        break;
    case DOT_TYPE:
        printf("DOT_TYPE\n");
        break;
    case SINGLEQUOTE_TYPE:
        printf("SINGLEQUOTE_TYPE\n");
        break;
    case VOID_TYPE:
        printf("VOID_TYPE\n");
        break;
    case CLOSURE_TYPE:
        printf("CLOSURE_TYPE\n");
        break;
    case PRIMITIVE_TYPE:
        printf("PRIMITIVE_TYPE\n");
        break;
    default:
        printf("Unknown Type\n");
    }
}

// Takes a pointer to item of symbol type and a pointer to item head of linkedlist and returns 1 if symbol is in list and 0 otherwise.
bool inList(Item *symbol, Item *list)
{
    while (list->type != NULL_TYPE)
    {
        if (strcmp(car(list)->s, symbol->s) == 0)
        {
            return true;
        }
        list = cdr(list);
    }
    return false;
}

// Takes a pointer to Item type listo f evaluated arguments and returns the summation of them as double if needed
Item *p_plus(Item *args)
{
    // if args is empty, return 0
    // while args

    Item *sum = talloc(sizeof(Item));
    sum->type = INT_TYPE;
    sum->i = 0;

    while (!isNull(args))
    {
        // sum is an int and next in args is int
        Item *num = car(args);
        if (sum->type == INT_TYPE && num->type == INT_TYPE)
        {
            sum->i = sum->i + num->i;
        }

        // sum is a double and next in args is int
        else if (sum->type == DOUBLE_TYPE && num->type == INT_TYPE)
        {
            sum->d = sum->d + num->i;
        }

        // sum is int and next in args is double: when we convert to double type
        else if (num->type == DOUBLE_TYPE && sum->type != DOUBLE_TYPE)
        {
            // convert
            sum->type = DOUBLE_TYPE;
            sum->d = sum->i + num->d;
        }
        // sum is double and next in args is double
        else if (num->type == DOUBLE_TYPE && sum->type == DOUBLE_TYPE)
        {
            sum->d = sum->d + num->d;
        }
        else
        {
            evaluationError("Adding non integer or double");
        }
        args = cdr(args);
    }
    return sum;
}

Item *p_div(Item *args)
{
    // Ensure only two arguments are present
    if (isNull(args) || isNull(cdr(args)) || !isNull(cdr(cdr(args))))
    {
        evaluationError("Division requires exactly two arguments");
    }

    Item *result = talloc(sizeof(Item));
    Item *first = car(args);
    Item *second = car(cdr(args));

    // Check types and perform division
    if (first->type == INT_TYPE && second->type == INT_TYPE)
    {
        if (second->i == 0)
        {
            evaluationError("Division by zero");
        }
        // Convert to double if not divisible evenly
        if (first->i % second->i != 0)
        {
            result->type = DOUBLE_TYPE;
            result->d = (double)first->i / second->i;
        }
        else
        {
            result->type = INT_TYPE;
            result->i = first->i / second->i;
        }
    }
    else if ((first->type == DOUBLE_TYPE || first->type == INT_TYPE) &&
             (second->type == DOUBLE_TYPE || second->type == INT_TYPE))
    {
        if ((second->type == INT_TYPE && second->i == 0) ||
            (second->type == DOUBLE_TYPE && second->d == 0))
        {
            evaluationError("Division by zero");
        }
        result->type = DOUBLE_TYPE;
        double firstVal = (first->type == INT_TYPE) ? (double)first->i : first->d;
        double secondVal = (second->type == INT_TYPE) ? (double)second->i : second->d;
        result->d = firstVal / secondVal;
    }
    else
    {
        evaluationError("Division with non-numeric types");
    }

    return result;
}

Item *p_mult(Item *args)
{
    Item *product = talloc(sizeof(Item));
    product->type = INT_TYPE;
    product->i = 1;

    while (!isNull(args))
    {
        Item *num = car(args);
        if (num->type == INT_TYPE)
        {
            if (product->type == INT_TYPE)
            {
                product->i *= num->i;
            }
            else
            {
                product->d *= num->i;
            }
        }
        else if (num->type == DOUBLE_TYPE)
        {
            if (product->type == INT_TYPE)
            {
                product->type = DOUBLE_TYPE;
                product->d = product->i;
            }
            product->d *= num->d;
        }
        else
        {
            evaluationError("Multiplication with non-numeric types");
        }
        args = cdr(args);
    }

    return product;
}

// Takes a pointer to Item type listo f evaluated arguments and returns first - second - .... as double if needed
Item *p_minus(Item *args)
{
    // if args is empty, return 0
    // while args

    Item *sum = talloc(sizeof(Item));
    if (!isNull(car(args)))
    {
        copy_item(sum, car(args));
    }
    args = cdr(args);
    while (!isNull(args))
    {
        // sum is an int and next in args is int
        Item *num = car(args);
        if (sum->type == INT_TYPE && num->type == INT_TYPE)
        {
            sum->i = sum->i - num->i;
        }

        // sum is a double and next in args is int
        else if (sum->type == DOUBLE_TYPE && num->type == INT_TYPE)
        {
            sum->d = sum->d - num->i;
        }

        // sum is int and next in args is double: when we convert to double type
        else if (num->type == DOUBLE_TYPE && sum->type != DOUBLE_TYPE)
        {
            // convert
            sum->type = DOUBLE_TYPE;
            sum->d = sum->i - num->d;
        }
        // sum is double and next in args is double
        else if (num->type == DOUBLE_TYPE && sum->type == DOUBLE_TYPE)
        {
            sum->d = sum->d - num->d;
        }
        else
        {
            evaluationError("Adding non integer or double");
        }
        args = cdr(args);
    }
    return sum;
}

Item *p_modulo(Item *args)
{
    // if args is empty, return 0
    // while args
    if (isNull(car(args)) || isNull(car(cdr(args))) || !isNull(cdr(cdr(args))))
    {
        evaluationError("Incorrect format for modulo");
    }
    Item *mod = talloc(sizeof(Item));

    Item *first = car(args);
    Item *second = car(cdr(args));

    if (first->type != INT_TYPE || second->type != INT_TYPE)
    {
        evaluationError("Not a number for modulo");
    }
    mod->type = INT_TYPE;
    mod->i = first->i % second->i;
    return mod;
}

// takes in one argument and returns a bool_type item indicating whether it is a null item or not
Item *p_null(Item *args)
{
    // if args is empty or more than 1 argument -> eval error
    if (isNull(args))
    {
        evaluationError("No arguments passed into null?");
    }
    Item *res = talloc(sizeof(Item));
    if (cdr(args)->type != NULL_TYPE)
    {
        evaluationError("More than one argument passed into null?");
    }

    res->type = BOOL_TYPE;

    if (isNull(car(args)) || (car(args)->type == CONS_TYPE && isNull(car(car(args)))))
    {
        res->s = "#t";
    }
    else
    {
        res->s = "#f";
    }

    return res;
}

// Takes in one argument that must be a cons type
// returns the car of that cons cell
Item *p_car(Item *args)
{
    // if args is empty or more than 1 argument -> eval error
    if (isNull(args))
    {
        evaluationError("No arguments passed into car");
    }
    if (cdr(args)->type != NULL_TYPE)
    {
        evaluationError("More than one argument passed into car");
    }
    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Is not a cons type");
    }

    return (car(car(args)));
}

// Takes in one argument that must be a cons type
// returns the cdr of that cons cell
Item *p_cdr(Item *args)
{

    if (isNull(args))
    {
        evaluationError("No arguments passed into cdr");
    }

    if (cdr(args)->type != NULL_TYPE)
    {
        evaluationError("More than one argument passed into cdr");
    }
    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Not a cons type");
    }
    if (isNull(cdr(car(args))))
    {
        return makeNull();
    }

    return cdr(car(args));
}

// Takes in two args of any type and creates a new cons cell with its car being the first arg and cdr being the second arg
Item *p_cons(Item *args)
{
    // printTree(args);
    if (isNull(args))
    {
        evaluationError("No arguments passed into cons");
    }
    if (cdr(args)->type == NULL_TYPE)
    {
        evaluationError("One more argument needed for cons");
    }
    if (cdr(cdr(args))->type != NULL_TYPE)
    {
        evaluationError("More than two argument passed into cons");
    }
    return cons(car(args), (car(cdr(args))));
}

// (taken from linkedlist.c)
// Takes two pointers to Items destination and source and copies the source to destination without pointing to source
void copy_item(Item *destination, Item *source)
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
    case CLOSURE_TYPE:
        destination->type = CLOSURE_TYPE;
        destination->cl = source->cl;
    case CONS_TYPE:
        destination->type = CONS_TYPE;
        copy_item(destination->c.car, source->c.car);
        copy_item(destination->c.cdr, source->c.cdr);
    default:
        break;
    }
}

// Takes a pointer to item and returns a new copy of the item
Item *copy_list(Item *original_list)
{

    Item *new_list = makeNull();

    if (original_list->type == CONS_TYPE)
    {
        new_list->type = CONS_TYPE;
        new_list->c.car = copy_list(car(original_list));
        new_list->c.cdr = copy_list(cdr(original_list));
    }
    else if (isNull(original_list))
    {
        return makeNull();
    }
    else
    {
        copy_item(new_list, original_list);
    }
    return new_list;
}

// Takes two args, first one being a list and second being any type
// makes a new copy of the first list and then tacks on the second arg
// to the end of the new list
Item *p_append(Item *args)
{
    if (isNull(args))
    {
        evaluationError("No arguments passed into append");
    }
    if (cdr(args)->type == NULL_TYPE)
    {
        evaluationError("One more argument needed for append");
    }
    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Type should be cons");
    }
    if (cdr(cdr(args))->type != NULL_TYPE)
    {
        evaluationError("More than 2 arguments to append");
    }

    Item *new_copy = copy_list(car(args));

    Item *head = makeNull();
    while (!isNull(new_copy))
    {
        head = cons(car(new_copy), head);
        new_copy = cdr(new_copy);
    }
    Item *second = car(cdr(args));

    if (isNull(car(head)))
    {
        // first arg has nothing inside it, so return the second
        return second;
    }

    if (second->type != CONS_TYPE)
    {
        head = reverse(head);
        Item *current = head;
        while (!isNull(cdr(current)))
        {
            current = cdr(current);
        }
        current->c.cdr = second;

        return head;
    }

    while (!isNull(second))
    {
        head = cons(car(second), head);
        second = cdr(second);
    }
    head = reverse(head);
    return head;
}

Item *p_l(Item *args)
{
    if (isNull(car(args)) || isNull(car(cdr(args))) || !isNull(cdr(cdr(args))))
    {
        evaluationError("Incorrect format for <");
    }

    Item *first = car(args);
    Item *second = car(cdr(args));

    if ((first->type != INT_TYPE && first->type != DOUBLE_TYPE) || (second->type != INT_TYPE && second->type != DOUBLE_TYPE))
    {
        evaluationError("Not a number for <");
    }
    Item *ret = talloc(sizeof(Item));
    ret->type = BOOL_TYPE;
    ret->s = talloc(2);

    if (first->type == INT_TYPE)
    {
        if (second->type == INT_TYPE)
        {
            if (first->i < second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->i < second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    else
    {
        if (second->type == INT_TYPE)
        {
            if (first->d < second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->d < second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    return ret;
}

Item *p_g(Item *args)
{
    if (isNull(car(args)) || isNull(car(cdr(args))) || !isNull(cdr(cdr(args))))
    {
        evaluationError("Incorrect format for <");
    }

    Item *first = car(args);
    Item *second = car(cdr(args));

    if ((first->type != INT_TYPE && first->type != DOUBLE_TYPE) || (second->type != INT_TYPE && second->type != DOUBLE_TYPE))
    {
        evaluationError("Not a number for >");
    }
    Item *ret = talloc(sizeof(Item));
    ret->type = BOOL_TYPE;
    ret->s = talloc(2);

    if (first->type == INT_TYPE)
    {
        if (second->type == INT_TYPE)
        {
            if (first->i > second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->i > second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    else
    {
        if (second->type == INT_TYPE)
        {
            if (first->d > second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->d > second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    return ret;
}

Item *p_e(Item *args)
{
    if (isNull(car(args)) || isNull(car(cdr(args))) || !isNull(cdr(cdr(args))))
    {
        evaluationError("Incorrect format for <");
    }

    Item *first = car(args);
    Item *second = car(cdr(args));

    if ((first->type != INT_TYPE && first->type != DOUBLE_TYPE) || (second->type != INT_TYPE && second->type != DOUBLE_TYPE))
    {
        evaluationError("Not a number for =");
    }
    Item *ret = talloc(sizeof(Item));
    ret->type = BOOL_TYPE;
    ret->s = talloc(2);

    if (first->type == INT_TYPE)
    {
        if (second->type == INT_TYPE)
        {
            if (first->i == second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->i == second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    else
    {
        if (second->type == INT_TYPE)
        {
            if (first->d == second->i)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
        if (second->type == DOUBLE_TYPE)
        {
            if (first->d == second->d)
            {
                ret->s = "#t";
            }
            else
            {
                ret->s = "#f";
            }
        }
    }
    return ret;
}

/*
 * Adds a binding between the given name
 * and the input function. Used to add
 * bindings for primitive funtions to the top-level
 * bindings list.
 */
void bind(char *name, Item *(*function)(Item *), Frame *frame)
{
    // Code omitted
    Item *prim = talloc(sizeof(Item));
    prim->type = PRIMITIVE_TYPE;
    prim->pf = function;
    Item *cell = talloc(sizeof(Item));
    Item *name_item = talloc(sizeof(Item));
    name_item->type = STR_TYPE;
    name_item->s = name;
    cell->type = CONS_TYPE;
    cell = cons(name_item, prim);
    frame->bindings = cons(cell, frame->bindings);
}

// Takes a pointer to a parse tree and interprets it. Prints the result of execution if there is a result
void interpret(Item *tree)
{
    top_frame = talloc(sizeof(Frame));
    top_frame->parent = NULL;
    top_frame->bindings = makeNull();

    // make primitive function bindings
    bind("+", p_plus, top_frame);
    bind("-", p_minus, top_frame);
    bind("null?", p_null, top_frame);
    bind("car", p_car, top_frame);
    bind("cdr", p_cdr, top_frame);
    bind("cons", p_cons, top_frame);
    bind("append", p_append, top_frame);
    bind("<", p_l, top_frame);
    bind(">", p_g, top_frame);
    bind("=", p_e, top_frame);
    bind("modulo", p_modulo, top_frame);
    bind("/", p_div, top_frame);
    bind("*", p_mult, top_frame);

    // int i =0;
    while (tree->type != NULL_TYPE)
    {
        // printTree(tree);
        // printf("---\n");
        Item *result = eval(car(tree), top_frame); // eval car(tree) worked for lambdas

        if (result && result->type != VOID_TYPE)
        {
            if (result->type == CONS_TYPE)
            {
                printf("(");
            }
            printTree(result);

            if (result->type == CONS_TYPE)
            {
                printf(")");
            }
            printf("\n");
        }
        tree = cdr(tree);
    }
}

// takes a char* that specefies the type of evaluation error and frees the allocated memory before exiting
void evaluationError(char *error)
{
    printf("Evaluation Error: %s\n", error);
    texit(1);
}

// Takes an pointer to Item and a frame pointer and evaluates the clause of the if condition
// returns an evaluation error if incorrect number of arguments is provided.
Item *evalIf(Item *args, Frame *frame)
{

    Item *result = eval(car(args), frame);

    if (!strcmp(result->s, "#f"))
    {
        return eval(car(cdr(cdr(args))), frame);
    }
    else
    {
        return eval(car(cdr(args)), frame);
    }
}

Item *evalCond(Item *args, Frame *frame)
{
    while (!isNull(args))
    {
        Item *currentPair = car(args);
        if (isNull(currentPair))
        {
            evaluationError("Empty condition in cond");
        }

        Item *condition = car(currentPair);
        // Check for 'else' symbol as a special case
        if (!strcmp(condition->s, "else"))
        {
            // If 'else' is present, evaluate the next expression in the pair
            if (!isNull(cdr(currentPair)))
            {
                return eval(car(cdr(currentPair)), frame);
            }
            else
            {
                evaluationError("Else without following expression");
            }
        }

        Item *evaluatedCondition = eval(condition, frame);

        // Ensure the condition evaluates to a boolean
        if (evaluatedCondition->type != BOOL_TYPE)
        {
            evaluationError("Non-boolean condition in cond");
        }

        // If condition is true, evaluate and return the corresponding expression
        if (!strcmp(evaluatedCondition->s, "#t"))
        {
            if (!isNull(cdr(currentPair)))
            {
                return eval(car(cdr(currentPair)), frame);
            }
            else
            {
                // Return VOID_TYPE if no expression follows a true condition
                Item *voidItem = talloc(sizeof(Item));
                voidItem->type = VOID_TYPE;
                return voidItem;
            }
        }

        // Move to the next condition-expression pair
        args = cdr(args);
    }

    // If no conditions are true and there's no 'else', return VOID_TYPE
    Item *voidItem = talloc(sizeof(Item));
    voidItem->type = VOID_TYPE;
    return voidItem;
}

// Takes a string symbol and a pointer to a frame and returns the pointer to the Item refereenced to by the symbol.
// Produces an evaluation error if the symbol isn't bound
// recursively through parent frames until top-frame is reached
Item *getsymbolfromframe(char *symbol, Frame *frame)
{
    if (frame == NULL)
    {
        // printf("Symbol is : %s \n", symbol);
        evaluationError("symbol not found: ");
    }

    Item *current = frame->bindings;
    while (current != NULL && current->type != NULL_TYPE)
    {

        if (!strcmp(car(car(current))->s, symbol))
        {
            return cdr(car(current));
        }
        current = cdr(current);
    }

    return getsymbolfromframe(symbol, frame->parent);
}

// Takes a pointer to a Item type of bindings and a pointer to Item type name and returns 1 if the same symbol name is already bound and false 0
int isDuplicateBinding(Item *bindings, Item *name)
{
    Item *list = bindings;

    while (list->type != NULL_TYPE)
    {
        Item *nameToCheck = car(car(bindings));
        if (!strcmp(name->s, nameToCheck->s))
        {

            return 1;
        }
        list = cdr(list);
    }
    return 0;
}

// Takes an pointer to Item type of arguments in the syle of ((x 3) (y 4)) and a pointer to a frame
// Evaluates the bindings and returns the bindings list
// produces an evaluation eror if incorrect symbol name or duplicate binding is provided
Item *getbindings(Item *args, Frame *frame)
{
    Item *ret = makeNull();
    Item *linkedlist = args;

    while (linkedlist->type != NULL_TYPE)
    {
        Item *name = car(car(linkedlist));
        if (name->type != SYMBOL_TYPE)
        {
            evaluationError("Tyring to get value of non symbol");
        }

        // checks if the name for the next binding to be added has already been defined in the bindings list so far
        if (isDuplicateBinding(ret, name))
        {

            evaluationError("duplicate binding");
        }

        Item *cell = cons(name, eval(car(cdr(car(linkedlist))), frame));

        ret = cons(cell, ret);

        linkedlist = cdr(linkedlist);
    }

    return ret;
}

// Takes a pointer to a body of expressions and a pointer to frame
// evaluates the expressions in the body recursively and returns a pointer to item containing the result of last expression result
Item *evalBody(Item *body, Frame *frame)
{
    while (cdr(body)->type != NULL_TYPE)
    {
        eval(car(body), frame);
        body = cdr(body);
    }

    return eval(car(body), frame);
}

Item *evalSet(Item *args, Frame *frame)
{
    if (frame == NULL)
    {
        evaluationError("No such variable");
    }
    if (isNull(cdr(args)) || !isNull(cdr(cdr(args))))
    {
        evaluationError("incorrect n.o. arguments");
    }
    Item *olditem = getsymbolfromframe(car(args)->s, frame);
    Item *newitem = eval(car(cdr(args)), frame);
    copy_item(olditem, newitem);
    Item *ret = talloc(sizeof(Item));
    ret->type = VOID_TYPE;
    return ret;
}

Item *evalSetCar(Item *args, Frame *frame)
{
    if (frame == NULL)
    {
        evaluationError("No such variable");
    }
    if (isNull(cdr(args)) || !isNull(cdr(cdr(args))))
    {
        evaluationError("incorrect n.o. arguments");
    }

    Item *reference = getsymbolfromframe(car(args)->s, frame);
    if (reference->type != CONS_TYPE)
    {
        evaluationError("not cons type");
    }
    *reference->c.car = *eval(car(cdr(args)), frame);

    Item *ret = talloc(sizeof(Item));
    ret->type = VOID_TYPE;
    return ret;
}

Item *evalSetCdr(Item *args, Frame *frame)
{
    if (frame == NULL)
    {
        evaluationError("No such variable");
    }

    if (isNull(cdr(args)) || !isNull(cdr(cdr(args))))
    {
        evaluationError("incorrect n.o. arguments");
    }
    Item *reference = getsymbolfromframe(car(args)->s, frame);

    if (reference->type != CONS_TYPE)
    {
        evaluationError("not cons type");
    }
    *reference->c.cdr = *eval(car(cdr(args)), frame);

    // tfree(reference->c.cdr);
    // *reference->c.cdr = *eval(car(cdr(args)), frame);
    Item *ret = talloc(sizeof(Item));
    ret->type = VOID_TYPE;
    return ret;
}

// Takes an item pointer to arguments of a let expression and a pointer to a frame
// Evaluates the bindings of the let expression and the returns a pointer to Item containing result of executing the body
// produces an evaluation error for incorrect synax or duplicate bindings
Item *evalLet(Item *args, Frame *frame)
{

    Frame *subframe = talloc(sizeof(Frame));
    subframe->parent = frame;

    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Incorrect let format");
    }

    if (isNull(car(car(args))) && isNull(cdr(car(args))))
    {
        // args does not have any bindings because its a cons type that points to two null types
        subframe->bindings = makeNull();
    }
    else
    {

        if (car(car(args))->type != CONS_TYPE)
        {

            evaluationError("Incorrect let body format");
        }

        if (isNull(car(car(car(args)))) && isNull(cdr(car(car(args)))))
        {
            // args has a null binding

            evaluationError("args has a null binding");
        }
        // args has bindings to be gotten
        subframe->bindings = getbindings(car(args), frame);
    }

    if (isNull(cdr(args)))
    {
        evaluationError("no args following the bindings in let");
    }
    subframe->bindings = reverse(subframe->bindings);
    return evalBody(cdr(args), subframe);
}

Item *evalLetStar(Item *args, Frame *frame)
{

    Frame *subframe = talloc(sizeof(Frame));
    subframe->parent = frame;
    subframe->bindings = makeNull();

    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Incorrect let format");
    }

    if (isNull(car(car(args))) && isNull(cdr(car(args))))
    {
        // args does not have any bindings because its a cons type that points to two null types
        subframe->bindings = makeNull();
    }
    else
    {

        if (car(car(args))->type != CONS_TYPE)
        {

            evaluationError("Incorrect let body format");
        }

        if (isNull(car(car(car(args)))) && isNull(cdr(car(car(args)))))
        {
            // args has a null binding

            evaluationError("args has a null binding");
        }
        // args has bindings to be gotten
        Item *bindings = car(args);
        while (!isNull(bindings))
        {
            Item *binding = car(bindings);
            Item *first = car(binding);
            if (first->type != SYMBOL_TYPE)
            {
                evaluationError("Tyring to get value of non symbol");
            }
            Item *second = eval(car(cdr(binding)), subframe);

            Item *cell = cons(first, second);
            Frame *newSubframe = talloc(sizeof(Frame));
            newSubframe->parent = subframe;
            newSubframe->bindings = makeNull();
            newSubframe->bindings = cons(cell, newSubframe->bindings);
            subframe = newSubframe;
            bindings = cdr(bindings);
        }
        return evalBody(cdr(args), subframe);
    }

    if (isNull(cdr(args)))
    {
        evaluationError("no args following the bindings in let");
    }

    return evalBody(cdr(args), subframe);
}

Item *evalLetRec(Item *args, Frame *frame)
{

    Frame *subframe = talloc(sizeof(Frame));
    subframe->parent = frame;
    subframe->bindings = makeNull();

    if (car(args)->type != CONS_TYPE)
    {
        evaluationError("Incorrect let format");
    }
    if (isNull(cdr(args)))
    {
        evaluationError("args following the bindings in let");
    }

    if (isNull(car(car(args))) && isNull(cdr(car(args))))
    {
        subframe->bindings = makeNull();
    }
    // args does not have any bindings because its a cons type that points to two null types
    else
    {

        if (car(car(args))->type != CONS_TYPE)
        {

            evaluationError("Incorrect let body format");
        }

        if (isNull(car(car(car(args)))) && isNull(cdr(car(car(args)))))
        {
            // args has a null binding

            evaluationError("args has a null binding");
        }
        // args has bindings to be gotten
        Item *bindings = car(args);
        Item *evals = makeNull();
        while (!isNull(bindings))
        {
            Item *binding = car(bindings);
            Item *first = car(binding);
            if (first->type != SYMBOL_TYPE)
            {
                evaluationError("Tyring to get value of non symbol");
            }
            Item *second = eval(car(cdr(binding)), subframe);
            evals = cons(second, evals);
            bindings = cdr(bindings);
        }
        bindings = car(args);
        evals = reverse(evals);

        while (!isNull(bindings))
        {
            Item *binding = car(bindings);
            Item *first = car(binding);
            Item *second = car(evals);
            Item *cell = cons(first, second);
            subframe->bindings = cons(cell, subframe->bindings);
            bindings = cdr(bindings);
            evals = cdr(evals);
        }
    }
    subframe->bindings = reverse(subframe->bindings);
    return evalBody(cdr(args), subframe);
}

// // Takes an item pointer to arguments of a define expression and a pointer to the frame
// Evaluates the bindings of the define expression and adds them to the top_frame and overrides duplicate bindings
// produces an evaluation error for incorrect synax
// returns nothing
void evalDefine(Item *args, Frame *frame)

{

    Item *first = car(args);

    if (isNull(cdr(args)))

    {

        evaluationError("incorrect define format");
    }

    Item *second = car(cdr(args));

    if (first->type != SYMBOL_TYPE)

    {

        evaluationError("incorrect define format");
    }

    if (!strcmp(first->s, "lambda"))

    {

        //
    }

    if (isDuplicateBinding(frame->bindings, first))

    {

        Item *current = frame->bindings;

        while (current->type != NULL_TYPE)

        {

            if (!strcmp(car(car(current))->s, first->s))

            {

                cdr(car(current))->c.car = eval(second, frame);

                return;
            }

            current = cdr(current);
        }
    }

    else

    {
        Item *binding = cons(first, eval(second, frame));
        frame->bindings = cons(binding, frame->bindings);
    }
}

// Takes a pointer to a item of type closure and arguments
// Sets the parameter names to the arguments
// Evaluates the lambda function and returns the result in a pointer to Item
// Produces an evaluation error on incorrect number of arguments or incorrect syntax
Item *applyLambda(Item *closure, Item *args)
{

    Frame *evalframe = talloc(sizeof(Frame));
    evalframe->bindings = makeNull();
    evalframe->parent = closure->cl.frame;
    Item *body = closure->cl.functionCode;
    if (closure->cl.paramNames->type == NULL_TYPE)
    {
        if (!isNull(args))
        {
            evaluationError("Should't take args");
        }
        // no params
    }
    else if (closure->cl.paramNames->type == CONS_TYPE)
    {
        // (x y z)
        Item *current = closure->cl.paramNames;

        while (args->type != NULL_TYPE && current->type != NULL_TYPE)
        {
            if (args->type == NULL_TYPE)
            {
                evaluationError("args don't match with procedure");
            }

            Item *cell = cons(car(current), car(args));

            evalframe->bindings = cons(cell, evalframe->bindings);
            current = cdr(current);
            args = cdr(args);
        }

        if (args->type != NULL_TYPE && current->type == NULL_TYPE)
        {
            evaluationError("number of actual parameters does not equal number of formal parameters");
        }
    }
    else
    {

        Item *cell = cons(closure->cl.paramNames, args);
        evalframe->bindings = cons(cell, evalframe->bindings);
    }
    evalframe->bindings = reverse(evalframe->bindings);
    return evalBody(body, evalframe);
}

// Takes a pointer to the args and the pointer to the corresponding frame
// Walks through the list of args and evaluates each argument in order while
// adding the evaluated arg to a new list of evaluated args. returns that list
Item *evaluateArgs(Item *args, Frame *frame)
{

    Item *evaluated_args = makeNull();
    if (isNull(args))
    {
        // no args type
        return makeNull();
    }

    else
    {
        // (x y z) type
        while (!isNull(args))
        {
            Item *arg = eval(car(args), frame);
            evaluated_args = cons(arg, evaluated_args);
            args = cdr(args);
        }
        evaluated_args = reverse(evaluated_args);
        return evaluated_args;
    }
}

// Takes a lambda expression and creates a closure type item with the parameter names, frame, and code
// produces an evaluation error on incorrect syntax
// returns an pointer to the closure type item containing the lambda expression
Item *makeLambda(Item *args, Frame *frame)
{

    Item *c = talloc(sizeof(Item));
    c->type = CLOSURE_TYPE;
    c->cl.frame = frame;
    c->cl.functionCode = cdr(args);
    if (cdr(args)->type == NULL_TYPE)
    {
        evaluationError("No code");
    }
    c->cl.paramNames = makeNull();

    if (args->type == NULL_TYPE)
    {
        // no args
    }
    else if (car(args)->type == CONS_TYPE)
    {
        // x y az
        Item *current = car(args);
        while (current->type != NULL_TYPE)
        {

            if (car(current)->type != SYMBOL_TYPE && car(current)->type != NULL_TYPE)
            {
                evaluationError("Must be symbol");
            }
            if (inList(car(current), c->cl.paramNames))
            {
                evaluationError("Duplicate symbol");
            }
            if (car(current)->type == CONS_TYPE)
            {
                evaluationError("lol how");
            }

            c->cl.paramNames = cons(car(current), c->cl.paramNames);
            current = cdr(current);
        }
        c->cl.paramNames = reverse(c->cl.paramNames);
    }
    else if (car(args)->type == SYMBOL_TYPE)
    {

        c->cl.paramNames = car(args);
    }
    return c;
}

Item *evalAnd(Item *args, Frame *frame)
{

    Item *clause = car(args);
    Item *ret = talloc(sizeof(Item));
    ret->type = BOOL_TYPE;
    ret->s = talloc(2);

    while (!isNull(args))
    {
        Item *clause = car(args);

        Item *result = eval(clause, frame);
        if (result->type == BOOL_TYPE)
        {
            if (result->s[1] == 'f')
            {
                ret->s = "#f";
                return ret;
            }
        }
        args = cdr(args);
    }
    ret->s = "#t";
    return ret;
}

Item *evalOr(Item *args, Frame *frame)
{

    Item *clause = car(args);
    Item *ret = talloc(sizeof(Item));
    ret->type = BOOL_TYPE;
    ret->s = talloc(2);

    while (!isNull(args))
    {
        Item *clause = car(args);

        Item *result = eval(clause, frame);
        if (result->type == BOOL_TYPE)
        {
            if (result->s[1] == 't')
            {
                ret->s = "#t";
                return ret;
            }
        }
        args = cdr(args);
    }
    ret->s = "#f";
    return ret;
}

// Takes a pointer to a parse tree item type and a pointer to a frame
// evaluates the parse tree within the frame and returns the result of evaluation
Item *eval(Item *tree, Frame *frame)
{

    switch (tree->type)
    {
    case INT_TYPE:
    {
        return tree;
    }
    case DOUBLE_TYPE:
    {
        return tree;
    }
    case SYMBOL_TYPE:
    {
        return getsymbolfromframe(tree->s, frame);
    }
    case BOOL_TYPE:
    {
        return tree;
    }
    case STR_TYPE:
    {
        return tree;
    }
    case CLOSURE_TYPE:
    {
        return tree;
    }
    case CONS_TYPE:
    {
        Item *first = car(tree);
        Item *args = cdr(tree);
        if (first->type == CLOSURE_TYPE)
        {
            return applyLambda(first, evaluateArgs(args, frame));
        }

        if (first->type == PRIMITIVE_TYPE)
        {
            // printTree(evaluateArgs(args, frame));
            // printf("\n");
            return first->pf(evaluateArgs(args, frame));
        }

        if (first->type == SYMBOL_TYPE)
        {
            if (!strcmp(first->s, "if"))
            {
                if (args->type == NULL_TYPE || cdr(args)->type == NULL_TYPE)
                {
                    evaluationError("incorrect if format");
                }

                return evalIf(args, frame); // Helper functions can make your code easier to navigate!
            }
            else if (!strcmp(first->s, "let"))
            {
                return evalLet(args, frame);
            }
            else if (!strcmp(first->s, "display"))
            {
                printTree(eval(car(args), frame));
                Item *x = makeNull();
                x->type = VOID_TYPE;
                return x;
            }
            else if (!strcmp(first->s, "newline"))
            {
                printf("\n");
                Item *x = makeNull();
                x->type = VOID_TYPE;
                return x;
            }
            else if (!strcmp(first->s, "let*"))
            {
                return evalLetStar(args, frame);
            }
            else if (!strcmp(first->s, "letrec"))
            {
                return evalLetRec(args, frame);
            }
            else if (!strcmp(first->s, "quote"))
            {
                if (args->type == NULL_TYPE || cdr(args)->type != NULL_TYPE)
                {

                    evaluationError("incorrect quote format");
                }
                // printTree(args);
                // print_type(args);

                return car(args);
            }
            else if (!strcmp(first->s, "define"))
            {
                if (isNull(args))
                {
                    evaluationError("Incorrect form");
                }
                Item *v = talloc(sizeof(Item));
                v->type = VOID_TYPE;
                evalDefine(args, frame);
                return v;
            }
            else if (!strcmp(first->s, "lambda"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format");
                }
                return makeLambda(args, frame);
            }
            else if (!strcmp(first->s, "set!"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format set");
                }

                return evalSet(args, frame);
            }
            else if (!strcmp(first->s, "set-car!"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format set");
                }

                return evalSetCar(args, frame);
            }
            else if (!strcmp(first->s, "set-cdr!"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format set");
                }

                return evalSetCdr(args, frame);
            }
            else if (!strcmp(first->s, "and"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format and");
                }

                return evalAnd(args, frame);
            }
            else if (!strcmp(first->s, "or"))
            {
                if (isNull(args))
                {
                    evaluationError("incorrect format or");
                }
                return evalOr(args, frame);
            }
            else if (!strcmp(first->s, "cond"))
            {
                if (args->type == NULL_TYPE || cdr(args)->type == NULL_TYPE)
                {
                    evaluationError("incorrect cond format");
                }
                return evalCond(args, frame); // Helper functions can make your code easier to navigate!
            }

            else
            {
                return eval(cons(eval(first, frame), args), frame);
            }
        }

        else
        {
            if (first->type == CONS_TYPE)
            {
                Item *first_result = eval(first, frame);
                if (first_result->type == CLOSURE_TYPE)
                {
                    return eval(cons(first_result, args), frame);
                }
                else if (first_result->type == PRIMITIVE_TYPE)
                {
                    return first_result->pf(evaluateArgs(args, frame));
                }
                else
                {
                    return first_result;
                }
            }
        }
    }
    default:
    {
        return 0;
    }
    }

    return 0;
}
