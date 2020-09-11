%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "common.h"
#include "parser.h"

extern int lastError;
extern FILE* yyin;
int yyinput(char* buf, int result, int max_size);

#define YY_INPUT(buf,result,max_size) result = yyinput(buf, result, max_size)

%}

blanks [ \t]+
chars [^ \t\n;=\<\>\$]+

%option nounput noinput
%option noyywrap

%%

{blanks}    { 

}

\n    {
    return NEWLINE_T;
}

\;  {
    return SEPARATOR;
}

&& {
    return AND;
}

&/[^&]  {
    return AMP;
}

"\>\>"    {
    return DGREAT;
}

\<  {
    return LESS;
}

\>  {
    return GREAT;
}

\|  {
    return PIPE;
}

\=  {
    return EQUALS;
}


[0-9]+/[\<\>]   {
    yylval.iVal = atoi(yytext);
    return IO_NUMBER;
}

{chars} {
    yylval.strVal = strdup(yytext);
    return WORD;
}

<<EOF>> {
    return END;
}
%%

void yyerror (char const *s) {
    fprintf (stderr, "%s\n", s);
}

int yyinput(char* buf, int result, int max_size)
{
    if (isatty(fileno(yyin))) {                                                                         
        char *_line;                                                                                    
        char _prompt[256] = {0};                                                                        
        if (feof(yyin))                                                                                 
            result = 0;                                                                                 
        else {                                                                                          
            snprintf(_prompt, 255, "dish [%d]> ", lastError);                                           
            _line = readline(_prompt);                                                                  
                                                                                                        
            if(!_line)                                                                                  
                result = 0;                                                                             
            else {                                                                                      
                if(strlen(_line) > (unsigned long)(max_size-2)) {                                       
                    fprintf(stderr,"Input line too long\n");                                            
                    result = 0;                                                                         
                }                                                                                      
                result = sprintf(buf,"%s\n",_line);                                                     
                add_history(_line);                                                                     
                free(_line);                                                                            
            }                                                                                           
        }                                                                                               
    }                                                                                                   
    else {                                                                                              
        if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive )                                              
		{                                                                                               
            int c = '*';                                                                                
            int n;                                                                                      
            for ( n = 0; n < max_size &&                                                                
                    (c = getc( yyin )) != EOF && c != '\n'; ++n )                                       
                buf[n] = (char) c;                                                                      
            if ( c == '\n' )                                                                            
                buf[n++] = (char) c;                                                                    
            if ( c == EOF && ferror( yyin ) )                                                           
                YY_FATAL_ERROR( "input in flex scanner failed" );                                       
            result = n;                                                                                 
		}                                                                                               
	    else                                                                                            
		{                                                                                               
		    errno=0;                                                                                    
		    while ( (result = (int) fread(buf, 1, (yy_size_t) max_size, yyin)) == 0 && ferror(yyin))    
			    {                                                                                       
			    if( errno != EINTR)                                                                     
			    {                                                                                       
				    YY_FATAL_ERROR( "input in flex scanner failed" );                                   
				    break;                                                                              
				}                                                                                       
			    errno=0;                                                                                
			    clearerr(yyin);                                                                         
			}                                                                                           
		}                                                                                               
    }
    return result;               
}