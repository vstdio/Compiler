#include "stdafx.h"

#include "../Utils/FileUtils.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtils.h"
#include "../grammarlib/GrammarBuilder.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../Lexer/Lexer.h"
#include "../Parser/LLParser.h"
#include "../Parser/LLParserTable.h"

#include "SemanticsVerifier.h"
#include "CodegenVisitor.h"
#include "WriteHelpers.h"

namespace
{
bool VerifyGrammarTerminalsMatchLexerTokens(const Grammar& grammar)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		auto production = grammar.GetProduction(row);
		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const GrammarSymbol& symbol = production->GetSymbol(col);
			if (symbol.GetType() == GrammarSymbolType::Terminal &&
				!TokenExists(symbol.GetText()))
			{
				return false;
			}
		}
	}
	return true;
}

std::unique_ptr<LLParser> CreateYolangParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>        -> <FunctionList> EndOfFile")
		// Функции
		.AddProduction("<FunctionList>   -> <Function> <FunctionList>")
		.AddProduction("<FunctionList>   -> #Eps#")
		.AddProduction("<Function>       -> Func Identifier {OnIdentifierParsed} LeftParenthesis <ParamList> RightParenthesis Arrow <Type> Colon <Statement> {OnFunctionParsed}")
		.AddProduction("<ParamList>      -> <Param> <ParamListTail>")
		.AddProduction("<ParamList>      -> #Eps#")
		.AddProduction("<ParamListTail>  -> Comma <Param> <ParamListTail>")
		.AddProduction("<ParamListTail>  -> #Eps#")
		.AddProduction("<Param>          -> Identifier {OnIdentifierParsed} Colon <Type> {OnFunctionParamParsed}")
		// Типы
		.AddProduction("<Type>           -> Int {OnIntegerTypeParsed}")
		.AddProduction("<Type>           -> Float {OnFloatTypeParsed}")
		.AddProduction("<Type>           -> Bool {OnBoolTypeParsed}")
		// Инструкции
		.AddProduction("<Statement>      -> <Condition>")
		.AddProduction("<Statement>      -> <Loop>")
		.AddProduction("<Statement>      -> <Decl>")
		.AddProduction("<Statement>      -> <Assign>")
		.AddProduction("<Statement>      -> <Return>")
		.AddProduction("<Statement>      -> <Composite>")
		.AddProduction("<Statement>      -> <Print>")
		.AddProduction("<Condition>      -> If LeftParenthesis <Expression> RightParenthesis <Statement> {OnIfStatementParsed} <OptionalElse>")
		.AddProduction("<OptionalElse>   -> Else <Statement> {OnOptionalElseClauseParsed}")
		.AddProduction("<OptionalElse>   -> #Eps#")
		.AddProduction("<Loop>           -> While LeftParenthesis <Expression> RightParenthesis <Statement> {OnWhileLoopParsed}")
		.AddProduction("<Decl>           -> Var Identifier {OnIdentifierParsed} Colon <Type> <OptionalAssign> Semicolon {OnVariableDeclarationParsed}")
		.AddProduction("<OptionalAssign> -> Assign <Expression> {OnOptionalAssignParsed}")
		.AddProduction("<OptionalAssign> -> #Eps#")
		.AddProduction("<Assign>         -> Identifier {OnIdentifierParsed} Assign <Expression> Semicolon {OnAssignStatementParsed}")
		.AddProduction("<Return>         -> Return <Expression> Semicolon {OnReturnStatementParsed}")
		.AddProduction("<Composite>      -> LeftCurly {OnCompositeStatementBeginParsed} <StatementList> RightCurly {OnCompositeStatementParsed}")
		.AddProduction("<StatementList>  -> <Statement> {OnCompositeStatementPartParsed} <StatementList>")
		.AddProduction("<StatementList>  -> #Eps#")
		.AddProduction("<Print>          -> Print <Expression> Semicolon {OnPrintStatementParsed}")
		// Выражения
		.AddProduction("<Expression>     -> <AddSubExpr>")
		.AddProduction("<AddSubExpr>     -> <MulDivExpr> <AddSubExprTail>")
		.AddProduction("<AddSubExprTail> -> Plus <MulDivExpr> {OnBinaryPlusParsed} <AddSubExprTail>")
		.AddProduction("<AddSubExprTail> -> Minus <MulDivExpr> {OnBinaryMinusParsed} <AddSubExprTail>")
		.AddProduction("<AddSubExprTail> -> #Eps#")
		.AddProduction("<MulDivExpr>     -> <AtomExpr> <MulDivExprTail>")
		.AddProduction("<MulDivExprTail> -> Mul <AtomExpr> {OnBinaryMulParsed} <MulDivExprTail>")
		.AddProduction("<MulDivExprTail> -> Div <AtomExpr> {OnBinaryDivParsed} <MulDivExprTail>")
		.AddProduction("<MulDivExprTail> -> #Eps#")
		.AddProduction("<AtomExpr>       -> LeftParenthesis <AddSubExpr> RightParenthesis")
		.AddProduction("<AtomExpr>       -> IntegerConstant {OnIntegerConstantParsed}")
		.AddProduction("<AtomExpr>       -> FloatConstant {OnFloatConstantParsed}")
		.AddProduction("<AtomExpr>       -> Identifier {OnIdentifierParsed}")
		.AddProduction("<AtomExpr>       -> Minus <AtomExpr> {OnUnaryMinusParsed}")
		.Build();

#ifdef _DEBUG
	WriteGrammar(*grammar, std::cout);
#endif

	if (VerifyGrammarTerminalsMatchLexerTokens(*grammar))
	{
		return std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar));
	}
	throw std::logic_error("all grammar terminals must match lexer tokens");
}

std::unique_ptr<LLParser> CreateCalcParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>       -> <Expr> EndOfFile")
		.AddProduction("<Expr>          -> <Term> <ExprHelper>")
		.AddProduction("<ExprHelper>    -> Plus <Term> {OnBinaryPlusParse} <ExprHelper>")
		.AddProduction("<ExprHelper>    -> Minus <Term> {OnBinaryMinusParse} <ExprHelper>")
		.AddProduction("<ExprHelper>    -> #Eps#")
		.AddProduction("<Term>          -> <Factor> <TermHelper>")
		.AddProduction("<TermHelper>    -> Mul <Factor> {OnBinaryMulParse} <TermHelper>")
		.AddProduction("<TermHelper>    -> Div <Factor> {OnBinaryDivParse} <TermHelper>")
		.AddProduction("<TermHelper>    -> #Eps#")
		.AddProduction("<Factor>        -> LeftParenthesis <Expr> RightParenthesis")
		.AddProduction("<Factor>        -> IntegerConstant {OnIntegerConstantParse}")
		.AddProduction("<Factor>        -> FloatConstant {OnFloatConstantParse}")
		.AddProduction("<Factor>        -> Identifier {OnIdentifierParse}")
		.AddProduction("<Factor>        -> Minus <Factor> {OnUnaryMinusParse}")
		.Build();

#ifdef _DEBUG
	WriteGrammar(*grammar, std::cout);
#endif

	if (VerifyGrammarTerminalsMatchLexerTokens(*grammar))
	{
		return std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar));
	}
	throw std::logic_error("all grammar terminals must match lexer tokens");
}

std::string GetStreamContent(std::istream& is)
{
	std::stringstream strm;
	strm << is.rdbuf();
	return strm.str();
}

void Compile(const std::string& text, std::ostream& out)
{
	auto parser = CreateYolangParser();
	auto ast = parser->Parse(text);

	if (!ast)
	{
		throw std::runtime_error("ast can't be generated...");
	}

	// SemanticsVerifier verifier;
	// verifier.VerifySemantics(*ast);

	// CodegenContext context;
	// StatementCodegen codegen(context);
	// codegen.GenerateMainFn(*ast);

	// context.Dump(out);
}
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		auto input = file_utils::OpenFileForReading("input.txt");
		auto output = file_utils::OpenFileForWriting("output.ll");
		Compile(GetStreamContent(*input), *output);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
