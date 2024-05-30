#include "tokenizer.h"
#include <stdlib.h>
#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include "string.h"

#ifndef ITEM_H
#define ITEM
#endif

// checks if a character is a special character
bool isspecial(char x)
{
    char *specials = "!$%&*/:<=>?~_^";

    while (*specials != '\0')
    {
        if (*specials == x)
        {
            return true;
        }
        specials++;
    }
    return false;
}

// Takes input from stdin and returns a linkedlist that contains the tokens
Item *tokenize()
{
    char charRead;
    Item *list = makeNull();
    charRead = (char)fgetc(stdin);
    while (charRead != EOF)
    {
        if (charRead == ' ' || charRead == '\n')
        {
            charRead = (char)fgetc(stdin);
            continue;
        }
        else if (charRead == '(')
        {
            Item *item = talloc(sizeof(Item));
            item->type = OPEN_TYPE;
            item->s = "(";
            list = cons(item, list);
            charRead = fgetc(stdin);
            continue;
        }

        else if (charRead == ')')
        {
            Item *item = talloc(sizeof(Item));
            item->type = CLOSE_TYPE;
            item->s = ")";
            list = cons(item, list);
            charRead = fgetc(stdin);

            continue;
        }

        else if (charRead == '-' || charRead == '+')
        {
            char sign = charRead;
            char potentialdigit = fgetc(stdin);
            if ((potentialdigit >= '9' || potentialdigit <= '0') && charRead != EOF)
            {
                Item *item = talloc(sizeof(Item));
                item->type = SYMBOL_TYPE;
                item->s = (charRead == '-') ? "-" : "+";
                list = cons(item, list);
                ungetc(potentialdigit, stdin);
            }
            else
            {
                ungetc(potentialdigit, stdin);
                char buffer[300] = "";
                int i = 0;
                charRead = (char)fgetc(stdin);
                while (charRead <= '9' && charRead >= '0' && charRead != EOF)
                {
                    buffer[i] = charRead;
                    i++;
                    charRead = (char)fgetc(stdin);
                }
                if (charRead == '.')
                {
                    buffer[i] = charRead;
                    i++;
                    charRead = fgetc(stdin);
                    while (charRead <= '9' && charRead >= '0')
                    {
                        buffer[i] = charRead;
                        i++;
                        charRead = fgetc(stdin);
                    }
                    buffer[i + 1] = '\0';
                    // ungetc(charRead, stdin);
                    Item *item = talloc(sizeof(Item));
                    item->type = DOUBLE_TYPE;
                    if (sign == '-')
                    {
                        item->d = -strtold(buffer, NULL);
                    }
                    else
                    {
                        item->d = strtold(buffer, NULL);
                    }

                    list = cons(item, list);
                    continue;
                }

                ungetc(charRead, stdin);

                Item *item = talloc(sizeof(Item));

                item->type = INT_TYPE;
                if (sign == '-')
                {
                    item->i = -strtol(buffer, NULL, 10);
                }
                else
                {
                    item->i = strtol(buffer, NULL, 10);
                }

                list = cons(item, list);
                // continue;
            }
        }
        else if (charRead == '.')
        {
            char buffer[300];
            int i = 0;
            buffer[i] = charRead;
            i++;
            charRead = (char)fgetc(stdin);
            while (charRead <= '9' && charRead >= '0')
            {
                buffer[i] = charRead;
                i++;
                charRead = (char)fgetc(stdin);
            }
            Item *item = talloc(sizeof(Item));
            item->type = DOUBLE_TYPE;
            item->d = strtold(buffer, NULL);
            list = cons(item, list);
            continue;
        }
        else if (charRead == '[')
        {
            Item *item = talloc(sizeof(Item));
            item->type = OPENBRACKET_TYPE;
            item->s = "[";
            list = cons(item, list);
        }
        else if (charRead == ']')
        {
            Item *item = talloc(sizeof(Item));
            item->type = CLOSEBRACKET_TYPE;
            item->s = "]";
            list = cons(item, list);
        }

        else if (charRead == '\"')
        {
            char buffer[300] = "";
            int i = 0;
            buffer[i] = charRead;
            i++;
            charRead = fgetc(stdin);
            while (charRead != '\"' && charRead != EOF)
            {
                buffer[i] = charRead;
                charRead = fgetc(stdin);
                i++;
            }
            buffer[i] = '\"';
            buffer[i + 1] = '\0';
            Item *item = talloc(sizeof(Item));
            item->type = STR_TYPE;
            item->s = talloc(sizeof(buffer));
            strcpy(item->s, buffer);
            list = cons(item, list);
        }

        else if (charRead == ';')
        {
            char nextChar = fgetc(stdin);
            {
                while (true)
                {
                    charRead = fgetc(stdin);
                    if (charRead == '\n' || charRead == EOF)
                        break;
                }
            }
        }

        else if (charRead <= '9' && charRead >= '0')
        {
            char buffer[300] = "";
            int i = 0;

            while (charRead <= '9' && charRead >= '0')
            {
                buffer[i] = charRead;
                i++;
                charRead = (char)fgetc(stdin);
            }

            if (charRead == '.')
            {
                buffer[i] = charRead;
                i++;
                charRead = (char)fgetc(stdin);
                while (charRead <= '9' && charRead >= '0')
                {
                    buffer[i] = charRead;
                    i++;
                    charRead = (char)fgetc(stdin);
                }
                Item *item = talloc(sizeof(Item));
                item->type = DOUBLE_TYPE;
                item->d = strtold(buffer, NULL);
                list = cons(item, list);

                continue;
            }

            ungetc(charRead, stdin);

            Item *item = talloc(sizeof(Item));
            item->type = INT_TYPE;
            item->i = strtol(buffer, NULL, 10);
            list = cons(item, list);
        }
        else if (charRead == '#')
        {
            charRead = fgetc(stdin);
            if (charRead == 'f')
            {
                Item *item = talloc(sizeof(Item));
                item->type = BOOL_TYPE;
                // item->s = talloc((3));
                item->s = "#f";
                list = cons(item, list);
            }
            else if (charRead == 't')
            {
                Item *item = talloc(sizeof(Item));
                item->type = BOOL_TYPE;
                item->s = "#t";
                list = cons(item, list);
            }
            else
            {
                printf("Syntax error (readBoolean): boolean was not #t or #f\n");
                texit(1);
            }
        }
        else if (isspecial(charRead))
        {
            Item *item = talloc(sizeof(Item));
            item->type = SYMBOL_TYPE;
            item->s = talloc(2);
            item->s[0] = charRead;
            item->s[1] = '\0';
            list = cons(item, list);
        }

        else
        {
            if (!isspecial(charRead) && !(charRead >= 'a' && charRead <= 'z') && !(charRead >= 'A' && charRead <= 'Z'))
            {
                printf("Syntax error (readSymbol): symbol %c does not start with an allowed first character.\n", charRead);
                texit(1);
            }
            char buffer[300] = "";
            int i = 0;
            buffer[i] = charRead;
            i++;
            charRead = fgetc(stdin);
            while (charRead != ']' && charRead != ']' && charRead != '(' && charRead != EOF && charRead != ' ' && charRead != '\n' && charRead != ')')
            {
                buffer[i] = charRead;
                charRead = fgetc(stdin);
                i++;
            }

            buffer[i + 1] = '\0';
            Item *item = talloc(sizeof(Item));
            item->type = SYMBOL_TYPE;
            item->s = talloc(sizeof(buffer));
            strcpy(item->s, buffer);
            list = cons(item, list);
            continue;
        }

        charRead = fgetc(stdin);
    }
    Item *revList = reverse(list);
    return revList;
}

// Takes an item and prints its content
void print_token(Item *current)
{
    switch (current->type)
    {
    case INT_TYPE:
        printf("%d:integer", current->i);

        return;
    case DOUBLE_TYPE:
        printf("%f:double", current->d);

        break;
    case STR_TYPE:
        printf("%s:string", current->s);

        break;
    case BOOL_TYPE:
        printf("%s:boolean", current->s);

        break;
    case SYMBOL_TYPE:
        printf("%s:symbol", current->s);

        break;
    case OPEN_TYPE:
        printf("%s:open", current->s);

        break;
    case CLOSE_TYPE:
        printf("%s:close", current->s);

        break;
    case OPENBRACKET_TYPE:
        printf("%s:openbracket", current->s);

        break;
    case CLOSEBRACKET_TYPE:
        printf("%s:closebracket", current->s);

        break;
    default:
        break;
    }
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Item *list)
{
    if (list == NULL)
    {
        printf("list is null");
        return;
    }
    Item *current = list;
    while (!isNull(current))
    {
        if (current->type == CONS_TYPE)
        {
            print_token(car(current));
            printf("\n");
            if (current->c.cdr != NULL)
            {
                current = cdr(current);
            }
            else
            {
                return;
            }
        }
        else
        {
            print_token(current);
            printf("\n");
            return;
        }
    }
}
