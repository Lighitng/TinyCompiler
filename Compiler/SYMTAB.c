/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
 in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
    int i = 0;
    while (key[i] != '\0')
    { temp = ((temp << SHIFT) + key[i]) % SIZE;
        ++i;
    }
    return temp;
}

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec
{ int lineno;
    struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
{ char * name;
    LineList lines;
    int memloc ; /* memory location for variable */
    struct BucketListRec * next;
} * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc )
{
    int hashnum=hash(name);
    BucketList l=hashTable[hashnum];
    LineList newLine;
    newLine=(LineList)malloc(sizeof(struct LineListRec));
    newLine->lineno=lineno;
    newLine->next=NULL;
    if(loc==0){
        
        while ( strcmp(name,l->name) != 0)
            l = l->next;
        newLine->next=l->lines;
        l->lines=newLine;
    }
    else{
        BucketList new;
        new=(BucketList )malloc(sizeof(struct BucketListRec));
        new->name=name;
        new->lines=newLine;
        new->memloc=loc;
        new->next=NULL;
        if(l==NULL){
            hashTable[hashnum]=new;
        }
        else{
            new->next=l;
            hashTable[hashnum]=new;
        }
    }
    
    
    
    
       
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ int h = hash(name);
    BucketList l =  hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
        l = l->next;
    if (l == NULL) return -1;
    else return l->memloc;
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
    fprintf(listing,"Variable Name  Location   Line Numbers\n");
    fprintf(listing,"-------------  --------   ------------\n");
    for (i=0;i<SIZE;++i)
    { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
        while (l != NULL)
        { LineList t = l->lines;
            fprintf(listing,"%-14s ",l->name);
            fprintf(listing,"%-8d  ",l->memloc);
            while (t != NULL)
            { fprintf(listing,"%4d ",t->lineno);
                t = t->next;
            }
            fprintf(listing,"\n");
            l = l->next;
        }
    }
    }
} /* printSymTab */
