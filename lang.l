%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "lang.tab.h"

char *unescape_string(const char *src) {
    // Allocate a buffer big enough for the worst case
    // (same length as src, since we only shrink on escapes)
    char *dest = malloc(strlen(src) + 1);
    char *d = dest;
    const char *s = src;

    while (*s) {
        if (*s == '\\') {
            s++;
            switch (*s) {
                case 'n':  *d++ = '\n'; break;
                case 't':  *d++ = '\t'; break;
                case '\\': *d++ = '\\'; break;
                case '"':  *d++ = '"'; break;
                // ... handle other escapes you care about ...
                default:
                    // If it's an unknown escape like \q,
                    // you might just copy it literally or skip it.
                    *d++ = *s;
                    break;
            }
            s++;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    return dest;
}


extern int yylineno;
%}

%option yylineno

%%

"skibidi"        { return SKIBIDI; }
"bussin"         { return BUSSIN; }
"flex"           { return FLEX; }
"rizz"           { return RIZZ; }
"main"           { return MAIN; }
"bruh"           { return BREAK; }
"sigma rule"     { return CASE; }
"yap"            { return YAP; }
"grimace"        { return CONST; }
"grind"          { return CONTINUE; }
"based"          { return DEFAULT; }
"mewing"         { return DO; }
"gigachad"       { return DOUBLE; }
"gyatt"          { return ENUM; }
"whopper"        { return EXTERN; }
"chad"           { return CHAD; }
"cringe"         { return GOTO; }
"edging"         { return IF; }
"amogus"         { return ELSE; }
"giga"           { return LONG; }
"edgy"           { return REGISTER; }
"soy"            { return SHORT; }
"nut"            { return SIGNED; }
"maxxing"        { return SIZEOF; }
"salty"          { return STATIC; }
"gang"           { return STRUCT; }
"ohio"           { return SWITCH; }
"chungus"        { return UNION; }
"nonut"          { return UNSIGNED; }
"schizo"         { return VOLATILE; }
"goon"           { return GOON; }
"baka"           { return BAKA; }
"cap"            { return CAP; }

"=="             { return EQ; }
"!="             { return NE; }
"<="             { return LE; }
">="             { return GE; }
"&&"             { return AND; }
"||"             { return OR; }
"<"              { return LT; }
">"              { return GT; }
"="              { return EQUALS; }

"+"              { return PLUS; }
"-"              { return MINUS; }
"*"              { return TIMES; }
"/"              { return DIVIDE; }
"%"              { return MOD; }

"("              { return LPAREN; }
")"              { return RPAREN; }
"{"              { return LBRACE; }
"}"              { return RBRACE; }
";"              { return SEMICOLON; }
","              { return COMMA; }
":"              { return COLON; }

"//"[^\n]*      ; /* Ignore single line comments */
"yes"            { yylval.ival = 1; return BOOLEAN; }
"no"             { yylval.ival = 0; return BOOLEAN; }
[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
[0-9]+           { yylval.ival = atoi(yytext); return NUMBER; }
'.' { yylval.ival = yytext[1]; return CHAR; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENTIFIER; }
\"([^\\\"]|\\.)*\" {
    // Strip the leading and trailing quotes:
    char *raw = strdup(yytext + 1);
    raw[strlen(raw) - 1] = '\0';

    // Convert backslash escapes to real characters:
    char *unescaped = unescape_string(raw);
    free(raw);

    yylval.sval = unescaped;  // Now it has real newlines, etc.
    return STRING_LITERAL;
}
\'([^\\\']|\\.)\' {
    // The text matched includes the leading and trailing single quotes, e.g. 'c'.
    // We'll decode the actual character inside.
    char c;
    
    if (yytext[1] == '\\') {
        // Handle escape sequences like '\n', '\t', '\\', '\''
        switch (yytext[2]) {
            case 'n':  c = '\n'; break;
            case 't':  c = '\t'; break;
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            // You can add more escapes if you need them
            default:
                // If unrecognized, just use the literal char
                c = yytext[2];
                break;
        }
    } else {
        // Normal single character, e.g. 'c'
        c = yytext[1];
    }
    
    yylval.ival = c;  // Put the character in the parser’s yylval
    return YAP; 
}

[ \t\n]+         ; /* Ignore whitespace */
.                { /* Ignore unrecognized characters */ }

%%

int yywrap(void) {
    return 1;
}
