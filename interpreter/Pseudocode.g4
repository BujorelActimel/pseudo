grammar Pseudocode;

program: stmt* EOF;

stmt
    : assign
    | swapStmt
    | readStmt
    | writeStmt
    | ifStmt
    | forStmt
    | whileStmt
    | doWhileStmt
    | repeatStmt
    | multiStmt
    ;

assign: NAME '<-' expr ;
swapStmt: NAME '<->' NAME ;
multiStmt: (assign | swapStmt | readStmt | writeStmt) (';' (assign | swapStmt | readStmt | writeStmt))+ ;

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
    | '[' expr ']'            #int
    | expr ('+'|'-') expr     #addExpr
    | '-' atom                #negExpr
    | '√' atom                #sqrtExpr
    | expr ('=' | '!=' | '<' | '<=' | '>' | '>=') expr  #compareExpr
    | atom                #atomExpr
    | expr ('SAU' | 'sau') expr     #orExpr
    | expr ('SI' | 'si') expr      #andExpr
    | ('NOT' | 'not') expr          #notExpr
    ;

atom: NUMBER | STRING | NAME | '(' expr ')' ;

NAME: [a-zA-Z_][a-zA-Z0-9_]* ;
NUMBER: [0-9]+ ('.' [0-9]+)? ;
STRING: '"' (~["\r\n])* '"' | '\'' (~['\r\n])* '\'' ;

WS: [ \t\r\n]+ -> skip ;
COMMENT: '#' ~[\r\n]* -> skip ;
