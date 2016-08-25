%{
#include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

/* regular definitions */
delim		[ \t\n\r\v]
ws 			{delim}+
comment		\/\/[^\n]*\n
digit		[0-9]
letter		[A-Za-z_]
number		{digit}+
string		\"([^\n\"]|\\\")*\"
identifier	{letter}({letter}|{digit})*

%%
FUNC                    { return FUNC; }
BEGIN					{ return OPENBLOCK; }
END						{ return CLOSEBLOCK; }
RETURN					{ return RETURN; }
PRINT					{ return PRINT; }
CONTINUE				{ return CONTINUE; }
IF						{ return IF; }
THEN					{ return THEN; }
ELSE					{ return ELSE; }
WHILE					{ return WHILE; }
DO						{ return DO; }
VAR						{ return VAR; }
{number}				{ return NUMBER; }
{identifier}			{ return IDENTIFIER; }
{string}				{ return STRING; }
{ws}					{/* no action and no return */}
{comment}				{/* no action and no return */}
.                       { return yytext[0]; }
%%
