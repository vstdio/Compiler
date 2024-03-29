#include "stdafx.h"
#include "LLParser.h"

#include "LLParserTable.h"
#include "../Lexer/ILexer.h"
#include "../AST/AST.h"

namespace
{
template <typename T>
T Pop(std::vector<T> & vect)
{
	auto value = std::move(vect.back());
	vect.pop_back();
	return std::move(value);
}

template <typename Derived, typename Base>
std::unique_ptr<Derived> DowncastUniquePtr(std::unique_ptr<Base> && base)
{
	std::unique_ptr<Derived> derived = nullptr;
	if (Derived* ptr = dynamic_cast<Derived*>(base.get()))
	{
		base.release();
		derived.reset(ptr);
		return std::move(derived);
	}
	assert(false);
	return nullptr;
}

// Название класса не имеет отношения к паттерну
class ASTBuilder
{
public:
	ASTBuilder(const Token& token)
		: m_token(token)
	{
	}

	std::unique_ptr<ProgramAST> BuildProgramAST()
	{
		auto program = std::make_unique<ProgramAST>();
		for (auto& func : m_functions)
		{
			program->AddFunction(std::move(func));
		}
		return program;
	}

	void OnFunctionParsed()
	{
		assert(!m_statements.empty());
		assert(!m_expressions.empty());

		auto statement = Pop(m_statements);
		auto identifier = DowncastUniquePtr<IdentifierAST>(Pop(m_expressions));
		assert(identifier);

		auto type = m_functionReturnType;
		m_functionReturnType = boost::none;
		auto params = std::move(m_funcProtoParamList);
		m_funcProtoParamList.clear();

		m_functions.push_back(
			std::make_unique<FunctionAST>(
				type,
				std::move(identifier),
				std::move(params),
				std::move(statement)
			)
		);
	}

	void OnFunctionParamParsed()
	{
		assert(!m_expressions.empty());
		assert(!m_types.empty());

		// Достаем распарсенный идентификатор из стека (если выражение из стека имеет тип не идентификатора, то внутренняя ошибка)
		auto identifier = DowncastUniquePtr<IdentifierAST>(Pop(m_expressions));
		assert(identifier);

		FunctionAST::Param param;
		param.first = identifier->GetName();
		param.second = Pop(m_types);

		m_funcProtoParamList.push_back(param);
	}

	void OnFunctionReturnTypeParsed()
	{
		assert(!m_types.empty());
		m_functionReturnType = Pop(m_types);
	}

	void OnTypeParsed(ExpressionType type)
	{
		m_types.push_back(type);
	}

	void OnArrayTypeParsed()
	{
		assert(!m_types.empty());
		++m_types.back().nesting;
	}

	void OnIfStatementParsed()
	{
		assert(!m_expressions.empty());
		assert(!m_statements.empty());
		auto expr = Pop(m_expressions);
		auto then = Pop(m_statements);
		m_statements.push_back(std::make_unique<IfStatementAST>(std::move(expr), std::move(then)));
	}

	void OnOptionalElseClauseParsed()
	{
		assert(m_statements.size() >= 2);
		auto stmt = Pop(m_statements);
		auto ifStmt = DowncastUniquePtr<IfStatementAST>(Pop(m_statements));
		ifStmt->SetElseClause(std::move(stmt));
		m_statements.push_back(std::move(ifStmt));
	}

	void OnWhileLoopParsed()
	{
		assert(!m_expressions.empty());
		assert(!m_statements.empty());
		auto expr = Pop(m_expressions);
		auto stmt = Pop(m_statements);
		m_statements.push_back(std::make_unique<WhileStatementAST>(
			std::move(expr), std::move(stmt)));
	}

	void OnVariableDeclarationParsed()
	{
		// Если при вызове этой функции не были распарсены тип и идентификатор, то внутренняя ошибка
		assert(!m_expressions.empty());
		assert(!m_types.empty());

		// Достаем из стека тип объявляемой переменной
		auto type = Pop(m_types);

		// Достаем из стека идентификатор объявляемой переменной (если тип не IdentifierAST, тогда это внутренняя ошибка)
		auto identifier = DowncastUniquePtr<IdentifierAST>(std::move(Pop(m_expressions)));
		assert(identifier);

		// Создаем узел объявления переменной
		auto node = std::make_unique<VariableDeclarationAST>(std::move(identifier), type);

		// Если был распарсен блок опционального присваивания при объявлении, то достаем выражение и сохраняем его
		if (m_optionalAssignExpression)
		{
			node->SetExpression(std::move(m_optionalAssignExpression));
			m_optionalAssignExpression = nullptr;
		}

		// Добавляем узел объявления переменной в стек
		m_statements.push_back(std::move(node));
	}

	void OnOptionalAssignParsed()
	{
		assert(!m_expressions.empty());
		m_optionalAssignExpression = std::move(Pop(m_expressions));
	}

	void OnAssignStatementParsed()
	{
		assert(m_expressions.size() >= 2);
		auto expr = Pop(m_expressions);
		auto identifier = DowncastUniquePtr<IdentifierAST>(std::move(Pop(m_expressions)));

		m_statements.push_back(std::make_unique<AssignStatementAST>(
			std::move(identifier), std::move(expr)));
	}

	void OnArrayElementAssignStatement()
	{
		assert(m_expressions.size() >= 2);

		auto expression = Pop(m_expressions);
		auto access = DowncastUniquePtr<ArrayElementAccessAST>(Pop(m_expressions));

		assert(access);
		m_statements.push_back(std::make_unique<ArrayElementAssignAST>(std::move(access), std::move(expression)));
	}

	void PrepareFnCallParamsParsing()
	{
		m_functionCallParamList.emplace_back();
	}

	void OnFunctionCallParamListMemberParsed()
	{
		assert(!m_expressions.empty());
		assert(!m_functionCallParamList.empty());

		auto& params = m_functionCallParamList.back();
		params.push_back(Pop(m_expressions));
	}

	void PrepareArrayLiteralElementsParsing()
	{
		m_arrayLiteralElementsList.emplace_back();
	}

	void OnArrayLiteralConstantParsed()
	{
		assert(!m_arrayLiteralElementsList.empty());
		std::vector<std::shared_ptr<IExpressionAST>> expressions;
		for (auto& expression : m_arrayLiteralElementsList.back())
		{
			expressions.push_back(std::move(expression));
		}
		m_arrayLiteralElementsList.pop_back();
		m_expressions.push_back(std::make_unique<LiteralConstantAST>(std::move(expressions)));
	}

	void OnArrayExpressionListMemberParsed()
	{
		assert(!m_expressions.empty());
		assert(!m_arrayLiteralElementsList.empty());
		m_arrayLiteralElementsList.back().push_back(Pop(m_expressions));
	}

	void OnFunctionCallStatementParsed()
	{
		auto call = CreateFunctionCallExprAST();
		m_statements.push_back(std::make_unique<FunctionCallStatementAST>(std::move(call)));
	}

	void OnReturnStatementParsed()
	{
		if (m_optionalReturnExpression)
		{
			m_statements.push_back(std::make_unique<ReturnStatementAST>(std::move(m_optionalReturnExpression)));
			m_optionalReturnExpression = nullptr;
		}
		else
		{
			m_statements.push_back(std::make_unique<ReturnStatementAST>(nullptr));
		}
	}

	void OnReturnExpression()
	{
		assert(!m_expressions.empty());
		auto expression = Pop(m_expressions);
		m_optionalReturnExpression = std::move(expression);
	}

	void PrepareCompositeStatementParsing()
	{
		m_compositeCache.emplace_back();
	}

	void OnCompositeStatementParsed()
	{
		auto composite = std::make_unique<CompositeStatementAST>();
		for (auto& stmt : m_compositeCache.back())
		{
			composite->AddStatement(std::move(stmt));
		}
		m_compositeCache.pop_back();
		m_statements.push_back(std::move(composite));
	}

	void OnCompositeStatementPartParsed()
	{
		assert(!m_statements.empty());
		m_compositeCache.back().push_back(Pop(m_statements));
	}

	void OnPrintStatementParsed()
	{
		assert(!m_functionCallParamList.empty());
		auto printStatement = std::make_unique<BuiltinCallStatementAST>(BuiltinCallStatementAST::Print);

		for (auto& expr : m_functionCallParamList.back())
		{
			printStatement->AddExpression(std::move(expr));
		}

		m_functionCallParamList.pop_back();
		m_statements.push_back(std::move(printStatement));
	}

	void OnScanStatementParsed()
	{
		assert(!m_functionCallParamList.empty());
		auto printStatement = std::make_unique<BuiltinCallStatementAST>(BuiltinCallStatementAST::Scan);

		for (auto& expr : m_functionCallParamList.back())
		{
			printStatement->AddExpression(std::move(expr));
		}

		m_functionCallParamList.pop_back();
		m_statements.push_back(std::move(printStatement));
	}

	void OnBinaryOperatorParsed(BinaryExpressionAST::Operator op)
	{
		assert(m_expressions.size() >= 2);

		auto right = Pop(m_expressions);
		auto left = Pop(m_expressions);

		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(std::move(left), std::move(right), op));
	}

	void OnIdentifierParsed()
	{
		assert(m_token.type == TokenType::Identifier);
		m_expressions.push_back(std::make_unique<IdentifierAST>(*m_token.value));
	}

	void OnIntegerConstantParsed()
	{
		assert(m_token.type == TokenType::IntegerConstant);
		m_expressions.push_back(std::make_unique<LiteralConstantAST>(std::stoi(*m_token.value)));
	}

	void OnFloatConstantParsed()
	{
		assert(m_token.type == TokenType::FloatConstant);
		m_expressions.push_back(std::make_unique<LiteralConstantAST>(std::stod(*m_token.value)));
	}

	void OnTrueConstantParsed()
	{
		assert(m_token.type == TokenType::True);
		auto trueConstantLiteral = std::make_unique<LiteralConstantAST>(true);
		m_expressions.push_back(std::move(trueConstantLiteral));
	}

	void OnFalseConstantParsed()
	{
		assert(m_token.type == TokenType::False);
		auto falseConstantLiteral = std::make_unique<LiteralConstantAST>(false);
		m_expressions.push_back(std::move(falseConstantLiteral));
	}

	void OnStringConstantParsed()
	{
		assert(m_token.type == TokenType::StringConstant);
		auto stringConstantLiteral = std::make_unique<LiteralConstantAST>(*m_token.value);
		m_expressions.push_back(std::move(stringConstantLiteral));
	}

	void ArrayElementAccess()
	{
		assert(m_expressions.size() >= 2);

		auto index = Pop(m_expressions);
		auto identifier = DowncastUniquePtr<IdentifierAST>(Pop(m_expressions));

		assert(identifier);
		m_expressions.push_back(std::make_unique<ArrayElementAccessAST>(identifier->GetName(), std::move(index)));
	}

	void OnAccessAdditionalSquareBracketParse()
	{
		assert(m_expressions.size() >= 2);

		auto expression = Pop(m_expressions);
		auto access = DowncastUniquePtr<ArrayElementAccessAST>(Pop(m_expressions));

		assert(access);
		access->AddIndex(std::move(expression));

		m_expressions.push_back(std::move(access));
	}

	void OnUnaryMinusParsed()
	{
		assert(!m_expressions.empty());
		auto node = std::move(m_expressions.back());
		m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<UnaryAST>(std::move(node), UnaryAST::Minus));
	}

	void OnUnaryPlusParsed()
	{
		assert(!m_expressions.empty());
		auto node = std::move(m_expressions.back());
		m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<UnaryAST>(std::move(node), UnaryAST::Plus));
	}

	void OnUnaryNegationParsed()
	{
		assert(!m_expressions.empty());
		auto node = std::move(m_expressions.back());
		m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<UnaryAST>(std::move(node), UnaryAST::Negation));
	}

	void OnFunctionCallExprParsed()
	{
		auto funcCall = CreateFunctionCallExprAST();
		m_expressions.push_back(std::move(funcCall));
	}

private:
	std::unique_ptr<FunctionCallExpressionAST> CreateFunctionCallExprAST()
	{
		assert(!m_expressions.empty());

		auto identifier = DowncastUniquePtr<IdentifierAST>(Pop(m_expressions));
		assert(identifier);

		assert(!m_functionCallParamList.empty());
		std::vector<std::unique_ptr<IExpressionAST>> expressions;

		for (auto& expr : m_functionCallParamList.back())
		{
			expressions.push_back(std::move(expr));
		}

		m_functionCallParamList.pop_back();
		return std::make_unique<FunctionCallExpressionAST>(identifier->GetName(), std::move(expressions));
	}

private:
	// Текущий токен, который был считан лексером
	const Token& m_token;

	// Стек для временного хранения считанных типов
	std::vector<ExpressionType> m_types;

	// Стек для временного хранения считанных параметров узла объявления функции
	std::vector<FunctionAST::Param> m_funcProtoParamList;

	// Стек для постепенного создания AST выражений
	std::vector<std::unique_ptr<IExpressionAST>> m_expressions;

	// Вспомогательный стек для хранения параметров вызова функции
	std::vector<std::vector<std::unique_ptr<IExpressionAST>>> m_functionCallParamList;

	std::vector<std::vector<std::unique_ptr<IExpressionAST>>> m_arrayLiteralElementsList;

	// Если был распарсен опциональный тип возвращаемого значения функции, эта переменная будет не пуста
	boost::optional<ExpressionType> m_functionReturnType;

	// Если был распарсен опциональный блок присваивания при объявлении, эта переменная будет не пуста
	std::unique_ptr<IExpressionAST> m_optionalAssignExpression;

	// Если было распарсено опциональное выражение возврата из функции, эта переменная будет не пуста
	std::unique_ptr<IExpressionAST> m_optionalReturnExpression;

	// Стек для постепенного создания AST инструкций
	std::vector<std::unique_ptr<IStatementAST>> m_statements;

	// Стек для временного хранения узлов AST вложенных инструкций
	std::vector<std::vector<std::unique_ptr<IStatementAST>>> m_compositeCache;

	// Стек для хранения AST функций
	std::vector<std::unique_ptr<FunctionAST>> m_functions;
};
}

LLParser::LLParser(
	std::unique_ptr<ILexer> && lexer,
	std::unique_ptr<LLParserTable> && table,
	std::ostream& output
)
	: m_lexer(std::move(lexer))
	, m_table(std::move(table))
	, m_output(output)
{
}

std::unique_ptr<ProgramAST> LLParser::Parse(const std::string& text)
{
	m_lexer->SetText(text);
	Token token = m_lexer->GetNextToken();

	std::vector<size_t> addresses;
	size_t index = 0;

	ASTBuilder astBuilder(token);
	std::unordered_map<std::string, std::function<void()>> actions = {
		{ "OnFunctionCallStatementParsed", std::bind(&ASTBuilder::OnFunctionCallStatementParsed, &astBuilder) },
		{ "OnFunctionCallParamListMemberParsed", std::bind(&ASTBuilder::OnFunctionCallParamListMemberParsed, &astBuilder ) },
		{ "OnFunctionParsed", std::bind(&ASTBuilder::OnFunctionParsed, &astBuilder) },
		{ "OnFunctionReturnTypeParsed", std::bind(&ASTBuilder::OnFunctionReturnTypeParsed, &astBuilder) },
		{ "OnFunctionParamParsed", std::bind(&ASTBuilder::OnFunctionParamParsed, &astBuilder) },
		{ "OnIfStatementParsed", std::bind(&ASTBuilder::OnIfStatementParsed, &astBuilder) },
		{ "OnOptionalElseClauseParsed", std::bind(&ASTBuilder::OnOptionalElseClauseParsed, &astBuilder) },
		{ "OnWhileLoopParsed", std::bind(&ASTBuilder::OnWhileLoopParsed, &astBuilder) },
		{ "OnVariableDeclarationParsed", std::bind(&ASTBuilder::OnVariableDeclarationParsed, &astBuilder) },
		{ "OnOptionalAssignParsed", std::bind(&ASTBuilder::OnOptionalAssignParsed, &astBuilder) },
		{ "OnAssignStatementParsed", std::bind(&ASTBuilder::OnAssignStatementParsed, &astBuilder) },
		{ "OnArrayElementAssignStatement", std::bind(&ASTBuilder::OnArrayElementAssignStatement, &astBuilder) },
		{ "OnReturnStatementParsed", std::bind(&ASTBuilder::OnReturnStatementParsed, &astBuilder) },
		{ "OnReturnExpression", std::bind(&ASTBuilder::OnReturnExpression, &astBuilder) },
		{ "PrepareCompositeStatementParsing", std::bind(&ASTBuilder::PrepareCompositeStatementParsing, &astBuilder) },
		{ "OnCompositeStatementParsed", std::bind(&ASTBuilder::OnCompositeStatementParsed, &astBuilder) },
		{ "OnCompositeStatementPartParsed", std::bind(&ASTBuilder::OnCompositeStatementPartParsed, &astBuilder) },
		{ "OnPrintStatementParsed", std::bind(&ASTBuilder::OnPrintStatementParsed, &astBuilder) },
		{ "OnScanStatementParsed", std::bind(&ASTBuilder::OnScanStatementParsed, &astBuilder) },
		{ "OnIntegerTypeParsed", std::bind(&ASTBuilder::OnTypeParsed, &astBuilder, ExpressionType{ ExpressionType::Int, 0 }) },
		{ "OnFloatTypeParsed", std::bind(&ASTBuilder::OnTypeParsed, &astBuilder, ExpressionType{ ExpressionType::Float, 0 } ) },
		{ "OnBoolTypeParsed", std::bind(&ASTBuilder::OnTypeParsed, &astBuilder, ExpressionType{ ExpressionType::Bool, 0 }) },
		{ "OnStringTypeParsed", std::bind(&ASTBuilder::OnTypeParsed, &astBuilder, ExpressionType{ ExpressionType::String, 0 }) },
		{ "OnArrayTypeParsed", std::bind(&ASTBuilder::OnArrayTypeParsed, &astBuilder) },
		{ "OnBinaryOrParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Or) },
		{ "OnBinaryAndParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::And) },
		{ "OnBinaryEqualsParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Equals) },
		{ "OnBinaryNotEqualsParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::NotEquals) },
		{ "OnBinaryLessParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Less) },
		{ "OnBinaryLessOrEqualsParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::LessOrEquals) },
		{ "OnBinaryMoreParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::More) },
		{ "OnBinaryMoreOrEqualsParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::MoreOrEquals) },
		{ "OnBinaryPlusParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Plus) },
		{ "OnBinaryMinusParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Minus) },
		{ "OnBinaryMulParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Mul) },
		{ "OnBinaryDivParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Div) },
		{ "OnBinaryModuloParsed", std::bind(&ASTBuilder::OnBinaryOperatorParsed, &astBuilder, BinaryExpressionAST::Mod) },
		{ "OnIdentifierParsed", std::bind(&ASTBuilder::OnIdentifierParsed, &astBuilder) },
		{ "OnIntegerConstantParsed", std::bind(&ASTBuilder::OnIntegerConstantParsed, &astBuilder) },
		{ "OnFloatConstantParsed", std::bind(&ASTBuilder::OnFloatConstantParsed, &astBuilder) },
		{ "OnTrueConstantParsed", std::bind(&ASTBuilder::OnTrueConstantParsed, &astBuilder) },
		{ "OnFalseConstantParsed", std::bind(&ASTBuilder::OnFalseConstantParsed, &astBuilder) },
		{ "OnStringConstantParsed", std::bind(&ASTBuilder::OnStringConstantParsed, &astBuilder) },
		{ "ArrayElementAccess", std::bind(&ASTBuilder::ArrayElementAccess, &astBuilder) },
		{ "OnAccessAdditionalSquareBracketParse", std::bind(&ASTBuilder::OnAccessAdditionalSquareBracketParse, &astBuilder) },
		{ "OnUnaryMinusParsed", std::bind(&ASTBuilder::OnUnaryMinusParsed, &astBuilder) },
		{ "OnUnaryPlusParsed", std::bind(&ASTBuilder::OnUnaryPlusParsed, &astBuilder) },
		{ "OnUnaryNegationParsed", std::bind(&ASTBuilder::OnUnaryNegationParsed, &astBuilder) },
		{ "OnFunctionCallExprParsed", std::bind(&ASTBuilder::OnFunctionCallExprParsed, &astBuilder) },
		{ "PrepareFnCallParamsParsing", std::bind(&ASTBuilder::PrepareFnCallParamsParsing, &astBuilder) },
		{ "PrepareArrayLiteralElementsParsing", std::bind(&ASTBuilder::PrepareArrayLiteralElementsParsing, &astBuilder) },
		{ "OnArrayLiteralConstantParsed", std::bind(&ASTBuilder::OnArrayLiteralConstantParsed, &astBuilder) },
		{ "OnArrayExpressionListMemberParsed", std::bind(&ASTBuilder::OnArrayExpressionListMemberParsed, &astBuilder) },
	};

	while (true)
	{
		auto state = m_table->GetEntry(index);

		if (state->isAttribute)
		{
			auto it = actions.find(state->name);
			if (it != actions.end())
			{
				it->second();
			}
			else
			{
				throw std::logic_error("attribute '" + state->name + "' doesn't have associated action");
			}
		}
		else if (!EntryAcceptsTerminal(*state, TokenTypeToString(token.type)))
		{
			if (!state->isError)
			{
				++index;
				continue;
			}
			else
			{
				const auto fmt = boost::format("unexpected token '%1%' found at line %2%, column %3%. Maybe you wanted to use '%4%' token?")
					% TokenTypeToString(token.type)
					% token.line
					% token.column
					% *state->beginnings.begin();
				throw std::runtime_error(fmt.str());
			}
		}

		if (state->isEnding)
		{
			assert(addresses.empty());
			return astBuilder.BuildProgramAST();
		}
		if (state->doPush)
		{
			addresses.push_back(index + 1);
		}
		if (state->doShift)
		{
			token = m_lexer->GetNextToken();
		}

		if (bool(state->next))
		{
			index = *state->next;
		}
		else
		{
			assert(!addresses.empty());
			index = addresses.back();
			addresses.pop_back();
		}
	}

	assert(false);
	throw std::logic_error("parse method of parser doesn't have a break statement in while loop");
}
