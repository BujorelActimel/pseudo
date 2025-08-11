grammar Pseudocode;

program: stmt* EOF;

stmt
    : assign
    | readStmt
    | writeStmt
    | ifStmt
    | forStmt
    | whileStmt
    | doWhileStmt
    | repeatStmt
    ;

assign: NAME '<-' expr ;

readStmt: 'citeste' nameList ;
writeStmt: 'scrie' exprList ;

nameList: NAME (',' NAME)* ;
exprList: expr (',' expr)* ;

ifStmt: 'daca' expr 'atunci' thenBlock ('altfel' elseBlock)? 'sf' ;
thenBlock: stmt* ;
elseBlock: stmt* ;

forStmt: 'pentru' NAME '<-' expr ',' expr (',' expr)? 'executa' stmt* 'sf' ;

whileStmt: 'cat' 'timp' expr 'executa' stmt* 'sf' ;
doWhileStmt: 'executa' stmt* 'cat' 'timp' expr ;

repeatStmt: 'repeta' stmt* 'pana' 'cand' expr ;

// Expressions
expr: expr ('*'|'/'|'%') expr #mulExpr
    | '[' expr '/' expr ']'   #intDivExpr
    | expr ('+'|'-') expr     #addExpr
    | '-' expr                #negExpr
    | expr ('=' | '!=' | '<' | '<=' | '>' | '>=') expr  #compareExpr
    | atom                #atomExpr
    | expr 'SAU' expr     #orExpr
    | expr 'SI' expr      #andExpr
    | 'NOT' expr          #notExpr
    ;

atom: NUMBER | STRING | NAME | '(' expr ')' ;

NAME: [a-zA-Z_][a-zA-Z0-9_]* ;
NUMBER: [0-9]+ ('.' [0-9]+)? ;
STRING: '"' (~["\r\n])* '"' | '\'' (~['\r\n])* '\'' ;

WS: [ \t\r\n]+ -> skip ;
COMMENT: '#' ~[\r\n]* -> skip ;
