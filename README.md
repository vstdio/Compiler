# compiler-60min

### Грамматика языка
<Program>       -> <FunctionList> EOF\
<FunctionList>  -> <Function> <FunctionList>\
<FunctionList>  -> #Eps#\
<Function>      -> FUN IDENTIFIER LPAREN <ParamList> RPAREN ARROW <Type> COLON <Statement>\
<ParamList>     -> <Param> <TailParamList>\
<ParamList>     -> #Eps#\
<TailParamList> -> COMMA <Param> <TailParamList>\
<TailParamList> -> #Eps#\
<Param>         -> IDENTIFIER COLON <Type>\
<Type>          -> INT\
<Type>          -> FLOAT\
<Type>          -> BOOL\
<Type>          -> ARRAY LABRACKET <Type> RABRACKET\
<Statement>     -> <Condition>\
<Statement>     -> <Loop>\
<Statement>     -> <Decl>\
<Statement>     -> <Assign>\
<Statement>     -> <Return>\
<Statement>     -> <Composite>\
<Condition>     -> IF LPAREN <Expression> RPAREN <Statement> <OptionalElse>\
<OptionalElse>  -> ELSE <Statement>\
<OptionalElse>  -> #Eps#\
<Loop>          -> WHILE LPAREN <Expression> RPAREN <Statement>\
<Decl>          -> VAR IDENTIFIER COLON <Type> SEMICOLON\
<Assign>        -> IDENTIFIER ASSIGN <Expression> SEMICOLON\
<Return>        -> RETURN <Expression> SEMICOLON\
<Composite>     -> LCURLY <StatementList> RCURLY\
<StatementList> -> <Statement> <StatementList>\
<StatementList> -> #Eps#\
<Expression>    -> IDENTIFIER\
<Expression>    -> INTLITERAL\
<Expression>    -> FLOATLITERAL\
<Expression>    -> TRUE\
<Expression>    -> FALSE\

### Токены
EOF          = -1\
FUN          =  0\
IDENTIFIER   =  1\
LPAREN       =  2\
RPAREN       =  3\
ARROW        =  4\
COLON        =  5\
COMMA        =  6\
IDENTIFIER   =  7\
INT          =  8\
FLOAT        =  9\
BOOL         = 10\
ARRAY        = 11\
LABRACKET    = 12\
RABRACKET    = 13\
IF           = 14\
LPAREN       = 15\
RPAREN       = 16\
ELSE         = 17\
WHILE        = 18\
VAR          = 19\
SEMICOLON    = 20\
ASSIGN       = 21\
RETURN       = 22\
LCURLY       = 23\
RCURLY       = 24\
INTLITERAL   = 25\
FLOATLITERAL = 26\
TRUE         = 27\
FALSE        = 28\
