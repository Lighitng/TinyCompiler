//
//  PARSE.c
//  Compiler
//
//  Created by Administration on 16/5/16.
//  Copyright © 2016年 Administration. All rights reserved.
//



/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode * stmt_sequence(void);
static TreeNode * statement(void);
static TreeNode * if_stmt(void);
static TreeNode * repeat_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * expt(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

static void syntaxError(char * message)
{ fprintf(listing,"\n>>> ");
    fprintf(listing,"Syntax error at line %d: %s",lineno,message);
    Error = TRUE;
}

static void match(TokenType expected)
{ if (token == expected) token = getToken();
else {
    syntaxError("unexpected token -> ");
    printToken(token,tokenString);
    fprintf(listing,"      ");
}
}

TreeNode * stmt_sequence(void)
{ TreeNode * t = statement();
    TreeNode * p = t;
    while ((token!=ENDFILE) && (token!=END) &&
           (token!=ELSE) && (token!=UNTIL))
    { TreeNode * q;
        match(SEMI);
        q = statement();
        if (q!=NULL) {
            if (t==NULL) t = p = q;
            else /* now p cannot be NULL either */
            { p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode * statement(void)
{ TreeNode * t = NULL;
    switch (token) {
        case IF : t = if_stmt(); break;
        case REPEAT : t = repeat_stmt(); break;
        case ID : t = assign_stmt(); break;
        case READ : t = read_stmt(); break;
        case WRITE : t = write_stmt(); break;
        default : syntaxError("unexpected token -> ");
            printToken(token,tokenString);
            token = getToken();
            break;
    } /* end case */
    return t;
}

TreeNode * if_stmt(void)
{
    TreeNode * t=newStmtNode(IfK);
    t->attr.op=token;
    match(IF);
    TreeNode *p=expt();
    t->child[0]=p;
    match(THEN);
    p=stmt_sequence();
    t->child[1]=p;
    
    if(token==ELSE){
        match(ELSE);
        p=stmt_sequence();
        t->child[2]=p;
        match(END);
    }
    
    else{
        match(END);
    }
    
    return t;
}

TreeNode * repeat_stmt(void)
{
    TreeNode * t=newStmtNode(RepeatK);
    t->attr.op=token;
    match(REPEAT);
    TreeNode *p=stmt_sequence();
    t->child[0]=p;
    match(UNTIL);
    p=expt();
    t->child[1]=p;
    return t;
}

TreeNode * assign_stmt(void)
{
    TreeNode * t=newStmtNode(AssignK);
    TreeNode * p=newExpNode(IdK);
    p->attr.name=copyString(tokenString);
    t->attr.name=p->attr.name;
    match(token);
    match(ASSIGN);
    p=expt();
    t->child[0]=p;
    
    
    return t;
}

TreeNode * read_stmt(void)
{
    TreeNode * t=newStmtNode(ReadK);
    match(token);
    t->attr.name=copyString(tokenString);
    match(token);
    return t;
    
}

TreeNode * write_stmt(void)
{
    TreeNode * t=newStmtNode(WriteK);
    t->attr.op=token;
    match(WRITE);
    TreeNode *p=expt();
    t->child[0]=p;
    return t;
    
}

TreeNode * expt(void)
{ TreeNode * t = simple_exp();
    if ((token==LT)||(token==EQ)) {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t!=NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

TreeNode * simple_exp(void)
{
    TreeNode * t=term();
    if((token==PLUS)||(token==MINUS)){
        TreeNode *p=newExpNode(OpK);
        if(p!=NULL){
            p->child[0]=t;
            p->attr.op=token;
            t=p;
            
        }
        match(token);
        if(t!=NULL)
            t->child[1]=term();
    }
    return t;
}

TreeNode * term(void)
{
    TreeNode * t=factor();
    if((token==TIMES)||(token==OVER)){
        TreeNode *p=newExpNode(OpK);
        if(p!=NULL){
            p->child[0]=t;
            p->attr.op=token;
            t=p;
        }
    
    match(token);
    if(t!=NULL)
        t->child[1]=factor();
    }
    return t;
}

TreeNode * factor(void)
{
    TreeNode * t;
    if(token==ID){
        t=newExpNode(IdK);
        t->attr.name=copyString(tokenString);
    }
    else if(token==NUM){
        t=newExpNode(ConstK);
        t->attr.val=atoi(tokenString);//transfer number string into integer
        
    }
    else{
        t = expt();
    }
    match(token);
    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode * parse(void)
{ TreeNode * t;
    token = getToken();
    t = stmt_sequence();
    if (token!=ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
