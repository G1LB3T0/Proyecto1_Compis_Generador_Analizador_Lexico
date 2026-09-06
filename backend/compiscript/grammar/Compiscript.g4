// Fuente de verdad del lenguaje; ANTLR genera el lexer, parser y Visitors.
grammar Compiscript;

// Gramática oficial extendida para reconciliar el enunciado con sus ejemplos:
// float es obligatorio en la rúbrica y los cuerpos de una sola sentencia se
// usan en el ejemplo oficial de recursión.

program: statement* EOF;

statement
  : variableDeclaration
  | constantDeclaration
  | assignmentStatement
  | functionDeclaration
  | classDeclaration
  | expressionStatement
  | printStatement
  | block
  | ifStatement
  | whileStatement
  | doWhileStatement
  | forStatement
  | foreachStatement
  | tryCatchStatement
  | switchStatement
  | breakStatement
  | continueStatement
  | returnStatement
  ;

block: '{' statement* '}';
controlBody: block | statement;

variableDeclaration: ('let' | 'var') Identifier typeAnnotation? initializer? ';';
constantDeclaration: 'const' Identifier typeAnnotation? '=' expression ';';
typeAnnotation: ':' type;
initializer: '=' expression;

assignmentStatement: leftHandSide '=' expression ';';
expressionStatement: expression ';';
printStatement: 'print' '(' expression ')' ';';

ifStatement: 'if' '(' expression ')' controlBody ('else' controlBody)?;
whileStatement: 'while' '(' expression ')' controlBody;
doWhileStatement: 'do' controlBody 'while' '(' expression ')' ';';
forStatement
  : 'for' '(' (variableDeclaration | expressionStatement | ';')
    expression? ';' expression? ')' controlBody
  ;
foreachStatement: 'foreach' '(' Identifier 'in' expression ')' controlBody;
breakStatement: 'break' ';';
continueStatement: 'continue' ';';
returnStatement: 'return' expression? ';';
tryCatchStatement: 'try' block 'catch' '(' Identifier ')' block;

switchStatement: 'switch' '(' expression ')' '{' switchCase* defaultCase? '}';
switchCase: 'case' expression ':' statement*;
defaultCase: 'default' ':' statement*;

functionDeclaration: 'function' Identifier '(' parameters? ')' (':' type)? block;
parameters: parameter (',' parameter)*;
parameter: Identifier (':' type)?;

classDeclaration: 'class' Identifier (':' Identifier)? '{' classMember* '}';
classMember: functionDeclaration | variableDeclaration | constantDeclaration;

expression: assignmentExpr;
assignmentExpr
  : leftHandSide '=' assignmentExpr # AssignExpr
  | conditionalExpr                 # ExprNoAssign
  ;
conditionalExpr: logicalOrExpr ('?' expression ':' expression)?;
logicalOrExpr: logicalAndExpr ('||' logicalAndExpr)*;
logicalAndExpr: equalityExpr ('&&' equalityExpr)*;
equalityExpr: relationalExpr (('==' | '!=') relationalExpr)*;
relationalExpr: additiveExpr (('<' | '<=' | '>' | '>=') additiveExpr)*;
additiveExpr: multiplicativeExpr (('+' | '-') multiplicativeExpr)*;
multiplicativeExpr: unaryExpr (('*' | '/' | '%') unaryExpr)*;
unaryExpr: ('-' | '!') unaryExpr | primaryExpr;
primaryExpr: literalExpr | leftHandSide | '(' expression ')';

literalExpr
  : IntegerLiteral
  | FloatLiteral
  | StringLiteral
  | arrayLiteral
  | 'null'
  | 'true'
  | 'false'
  ;

leftHandSide: primaryAtom suffixOp*;
primaryAtom
  : Identifier                          # IdentifierExpr
  | 'new' Identifier '(' arguments? ')' # NewExpr
  | 'this'                              # ThisExpr
  ;
suffixOp
  : '(' arguments? ')' # CallExpr
  | '[' expression ']' # IndexExpr
  | '.' Identifier     # PropertyAccessExpr
  ;
arguments: expression (',' expression)*;
arrayLiteral: '[' (expression (',' expression)*)? ']';

type: baseType ('[' ']')*;
baseType: 'boolean' | 'integer' | 'float' | 'string' | 'void' | Identifier;

FloatLiteral: [0-9]+ '.' [0-9]+;
IntegerLiteral: [0-9]+;
StringLiteral: '"' (~["\\\r\n] | '\\' .)* '"';
Identifier: [a-zA-Z_][a-zA-Z0-9_]*;
WS: [ \t\r\n]+ -> skip;
COMMENT: '//' ~[\r\n]* -> skip;
MULTILINE_COMMENT: '/*' .*? '*/' -> skip;
