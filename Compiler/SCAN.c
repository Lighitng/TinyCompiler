/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
{ START,INASSIGN,INCOMMENT,INNUM,INID,DONE }
StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
 source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
 from lineBuf, reading in a new line if lineBuf is
 exhausted */
static int getNextChar(void)
{ if (!(linepos < bufsize))
{ lineno++;
    if (fgets(lineBuf,BUFLEN-1,source))
    { if (EchoSource) fprintf(listing,"%4d: %s",lineno,lineBuf);
        bufsize = strlen(lineBuf);
        linepos = 0;
        return lineBuf[linepos++];
    }
    else
    { EOF_flag = TRUE;
        return EOF;
    }
}
else return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
 in lineBuf */
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}

/* lookup table of reserved words */
static struct
{ char* str;
    TokenType tok;
} reservedWords[MAXRESERVED]
= {{"if",IF},{"then",THEN},{"else",ELSE},{"end",END},
    {"repeat",REPEAT},{"until",UNTIL},{"read",READ},
    {"write",WRITE}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ int i;
    for (i=0;i<MAXRESERVED;i++)
        if (!strcmp(s,reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void)
{  /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;
    while (state != DONE)
    { int c = getNextChar();
        save = TRUE;
        switch (state)
        { case START:
                if (isdigit(c))
                    state = INNUM;
                else if(isalpha(c))
                    state =INID;
                else if(c=='{')
                {state=INCOMMENT;save=FALSE;}
                else if(c==':')
                    state=INASSIGN;
                else if(c=='+'||c=='-'||c=='*'||c=='/'||c==';'||c=='('||c==')'||c=='<'||c=='=')
                {
                    switch(c)
                    {
                        case '+':save = FALSE;
                                 state = DONE;
                                 currentToken =PLUS;
                            break;
                        case '-':save = FALSE;
                                 state = DONE;
                                 currentToken =MINUS;
                            break;
                        case '*':save = FALSE;
                                 state = DONE;
                                 currentToken =TIMES;
                            break;
                        case '/':
                                 save = FALSE;
                                 state = DONE;
                                 currentToken =OVER;
                            break;
                        case '(':
                                 save = FALSE;
                                 state = DONE;
                                 currentToken =LPAREN;
                            break;
                        case ')':
                                 save = FALSE;
                                 state = DONE;
                                 currentToken =RPAREN;
                            break;
                        case '<':
                                 save = FALSE;
                                 state = DONE;
                                 currentToken =LT;
                            break;
                        case '=':
                                 save = FALSE;
                                 state = DONE;
                                 currentToken =EQ;
                            break;
                        case ';':
                                save = FALSE;
                                state = DONE;
                                currentToken =SEMI;
                            break;
                    }
                }
                else if(c==EOF)
                {
                    save=FALSE;
                    currentToken=ENDFILE;
                    state=DONE;
                    break;
                }
                else{
                    state=START;
                    save=FALSE;}
                    break;
            case INCOMMENT:
                if (c=='}')
                    state=START;
                else
                    state=INCOMMENT;
                save=FALSE;
                break;
            case INASSIGN:
                if(c=='='){
                state = DONE;
                currentToken = ASSIGN;
                }
                else{
                    ungetNextChar();
                    save=FALSE;
                    state=DONE;
                    currentToken=ERROR;
                    
                }
                break;
            case INNUM:
                if (!isdigit(c))
                { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = NUM;
                }
                break;
            case INID:
                if (!isalpha(c))
                { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = ID;
                }
                break;
            case DONE:
            default: /* should never happen */
                fprintf(listing,"Scanner Bug: state= %d\n",state);
                state = DONE;
                currentToken = ERROR;
                break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char) c;
        if (state == DONE)
        { tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan) {
        fprintf(listing,"\t%d: ",lineno);
        printToken(currentToken,tokenString);
    }
    return currentToken;
} /* end getToken */
