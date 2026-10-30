// Gramática del intérprete SQL: una sola fuente para el lexer y el parser.
// ANTLR genera desde aquí SQLLexer, SQLParser y SQLVisitor en C++.
grammar SQL;

// Las palabras reservadas e identificadores no distinguen mayúsculas.
options { caseInsensitive = true; }

// ───────────────────────────── Parser ─────────────────────────────

// Un script es una secuencia de sentencias separadas por ';'.
script
  : SEMI* (statement (SEMI+ statement)* SEMI*)? EOF
  ;

statement
  : createTableStatement
  | dropTableStatement
  | createIndexStatement
  | dropIndexStatement
  | insertStatement
  | updateStatement
  | deleteStatement
  | selectStatement
  ;

// ── DDL: definición de tablas e índices ──

createTableStatement
  : K_CREATE K_TABLE (K_IF K_NOT K_EXISTS)? identifier
    LPAREN tableElement (COMMA tableElement)* RPAREN
  ;

tableElement
  : columnDefinition
  | tableConstraint
  ;

columnDefinition
  : identifier dataType columnConstraint*
  ;

dataType
  : K_INT
  | K_FLOAT
  | K_VARCHAR LPAREN INT_LITERAL RPAREN
  | K_BOOLEAN
  | K_DATE
  ;

columnConstraint
  : K_PRIMARY K_KEY
  | K_NOT K_NULL
  | K_NULL
  | K_UNIQUE
  | K_REFERENCES identifier LPAREN identifier RPAREN
  ;

tableConstraint
  : K_PRIMARY K_KEY LPAREN identifier RPAREN
  | K_UNIQUE LPAREN identifier RPAREN
  | K_FOREIGN K_KEY LPAREN identifier RPAREN
    K_REFERENCES identifier LPAREN identifier RPAREN
  ;

dropTableStatement
  : K_DROP K_TABLE (K_IF K_EXISTS)? identifier
  ;

createIndexStatement
  : K_CREATE K_UNIQUE? K_INDEX identifier K_ON identifier
    LPAREN identifier RPAREN
  ;

dropIndexStatement
  : K_DROP K_INDEX (K_IF K_EXISTS)? identifier
  ;

// ── DML: modificación de registros ──

insertStatement
  : K_INSERT K_INTO identifier (LPAREN identifier (COMMA identifier)* RPAREN)?
    K_VALUES valuesRow (COMMA valuesRow)*
  ;

valuesRow
  : LPAREN expression (COMMA expression)* RPAREN
  ;

updateStatement
  : K_UPDATE identifier K_SET assignment (COMMA assignment)* whereClause?
  ;

assignment
  : identifier EQ expression
  ;

deleteStatement
  : K_DELETE K_FROM identifier whereClause?
  ;

// ── Consultas ──

selectStatement
  : K_SELECT K_DISTINCT? selectItem (COMMA selectItem)*
    fromClause?
    whereClause?
    groupByClause?
    havingClause?
    orderByClause?
    limitClause?
  ;

selectItem
  : STAR
  | identifier DOT STAR
  | expression (K_AS? identifier)?
  ;

fromClause
  : K_FROM tableReference joinClause*
  ;

tableReference
  : identifier (K_AS? identifier)?
  ;

joinClause
  : K_INNER? K_JOIN tableReference K_ON expression
  ;

whereClause
  : K_WHERE expression
  ;

groupByClause
  : K_GROUP K_BY expression (COMMA expression)*
  ;

havingClause
  : K_HAVING expression
  ;

orderByClause
  : K_ORDER K_BY orderItem (COMMA orderItem)*
  ;

orderItem
  : expression (K_ASC | K_DESC)?
  ;

limitClause
  : K_LIMIT INT_LITERAL
  ;

// ── Expresiones: el orden de las alternativas define la precedencia ──
// (primero se agrupa lo de mayor precedencia: unarios, *, +, comparaciones,
// IS/IN, NOT, AND y por último OR).

expression
  : primary                                                       # primaryExpression
  | op=(MINUS | PLUS) expression                                  # unaryExpression
  | expression op=(STAR | SLASH | PERCENT) expression             # multiplicativeExpression
  | expression op=(PLUS | MINUS) expression                       # additiveExpression
  | expression op=(EQ | NEQ | LT | LTE | GT | GTE) expression     # comparisonExpression
  | expression K_IS K_NOT? K_NULL                                 # isNullExpression
  | expression K_NOT? K_IN LPAREN selectStatement RPAREN          # inSubqueryExpression
  | expression K_NOT? K_IN LPAREN expression (COMMA expression)* RPAREN # inListExpression
  | K_NOT expression                                              # notExpression
  | expression K_AND expression                                   # andExpression
  | expression K_OR expression                                    # orExpression
  ;

primary
  : literal                                 # literalPrimary
  | aggregateFunction                       # aggregatePrimary
  | columnReference                         # columnPrimary
  | K_EXISTS LPAREN selectStatement RPAREN  # existsPrimary
  | LPAREN selectStatement RPAREN           # subqueryPrimary
  | LPAREN expression RPAREN                # parenthesizedPrimary
  ;

aggregateFunction
  : K_COUNT LPAREN STAR RPAREN
  | name=(K_COUNT | K_SUM | K_AVG | K_MIN | K_MAX) LPAREN K_DISTINCT? expression RPAREN
  ;

columnReference
  : identifier (DOT identifier)?
  ;

literal
  : INT_LITERAL
  | FLOAT_LITERAL
  | STRING_LITERAL
  | K_TRUE
  | K_FALSE
  | K_NULL
  | K_DATE STRING_LITERAL
  ;

identifier
  : IDENTIFIER
  ;

// ───────────────────────────── Lexer ─────────────────────────────
// Prefijo K_ en palabras reservadas: evita choques con macros de C/C++
// como NULL, TRUE, FALSE, DELETE o IN al generar el código C++.

K_CREATE     : 'CREATE';
K_TABLE      : 'TABLE';
K_DROP       : 'DROP';
K_INDEX      : 'INDEX';
K_ON         : 'ON';
K_IF         : 'IF';
K_EXISTS     : 'EXISTS';
K_INSERT     : 'INSERT';
K_INTO       : 'INTO';
K_VALUES     : 'VALUES';
K_UPDATE     : 'UPDATE';
K_SET        : 'SET';
K_DELETE     : 'DELETE';
K_SELECT     : 'SELECT';
K_DISTINCT   : 'DISTINCT';
K_FROM       : 'FROM';
K_WHERE      : 'WHERE';
K_AND        : 'AND';
K_OR         : 'OR';
K_NOT        : 'NOT';
K_IN         : 'IN';
K_IS         : 'IS';
K_NULL       : 'NULL';
K_INNER      : 'INNER';
K_JOIN       : 'JOIN';
K_AS         : 'AS';
K_ORDER      : 'ORDER';
K_GROUP      : 'GROUP';
K_BY         : 'BY';
K_ASC        : 'ASC';
K_DESC       : 'DESC';
K_HAVING     : 'HAVING';
K_LIMIT      : 'LIMIT';
K_COUNT      : 'COUNT';
K_SUM        : 'SUM';
K_AVG        : 'AVG';
K_MIN        : 'MIN';
K_MAX        : 'MAX';
K_PRIMARY    : 'PRIMARY';
K_KEY        : 'KEY';
K_FOREIGN    : 'FOREIGN';
K_REFERENCES : 'REFERENCES';
K_UNIQUE     : 'UNIQUE';
K_INT        : 'INTEGER' | 'INT';
K_FLOAT      : 'FLOAT';
K_VARCHAR    : 'VARCHAR';
K_BOOLEAN    : 'BOOLEAN';
K_DATE       : 'DATE';
K_TRUE       : 'TRUE';
K_FALSE      : 'FALSE';

NEQ     : '<>' | '!=';
LTE     : '<=';
GTE     : '>=';
EQ      : '=';
LT      : '<';
GT      : '>';
PLUS    : '+';
MINUS   : '-';
STAR    : '*';
SLASH   : '/';
PERCENT : '%';
COMMA   : ',';
SEMI    : ';';
DOT     : '.';
LPAREN  : '(';
RPAREN  : ')';

FLOAT_LITERAL  : DIGIT+ '.' DIGIT+;
INT_LITERAL    : DIGIT+;
// Comillas simples; una comilla dentro del texto se escribe doble ('').
STRING_LITERAL : '\'' ( ~'\'' | '\'\'' )* '\'';
IDENTIFIER     : [A-Z_] [A-Z_0-9]*;

LINE_COMMENT  : '--' ~[\r\n]* -> skip;
BLOCK_COMMENT : '/*' .*? '*/' -> skip;
WS            : [ \t\r\n]+ -> skip;

fragment DIGIT : [0-9];
