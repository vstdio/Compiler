#include "stdafx.h"
#include "AST.h"

// Binary expression
BinaryExpressionAST::BinaryExpressionAST(
	std::unique_ptr<IExpressionAST> && left,
	std::unique_ptr<IExpressionAST> && right,
	BinaryExpressionAST::Operator op)
	: m_left(std::move(left))
	, m_right(std::move(right))
	, m_op(op)
{
}

const IExpressionAST& BinaryExpressionAST::GetLeft()const
{
	return *m_left;
}

const IExpressionAST& BinaryExpressionAST::GetRight()const
{
	return *m_right;
}

BinaryExpressionAST::Operator BinaryExpressionAST::GetOperator()const
{
	return m_op;
}

void BinaryExpressionAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Literal constant
LiteralConstantAST::LiteralConstantAST(const LiteralConstantAST::Value& value)
	: m_value(value)
{
}

const LiteralConstantAST::Value& LiteralConstantAST::GetValue()const
{
	return m_value;
}

void LiteralConstantAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Array element access
ArrayElementAccessAST::ArrayElementAccessAST(const std::string& name, std::unique_ptr<IExpressionAST> && index)
	: m_name(name)
	, m_indices()
{
	m_indices.push_back(std::move(index));
}

const std::string& ArrayElementAccessAST::GetName()const
{
	return m_name;
}

size_t ArrayElementAccessAST::GetIndexCount()const
{
	return m_indices.size();
}

const IExpressionAST& ArrayElementAccessAST::GetIndex(size_t index /* = 0 */)const
{
	if (index >= m_indices.size())
	{
		throw std::out_of_range("index must be less than indices count in array element access");
	}
	return *m_indices[index];
}

void ArrayElementAccessAST::AddIndex(std::unique_ptr<IExpressionAST> && index)
{
	m_indices.push_back(std::move(index));
}

void ArrayElementAccessAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Unary operator
UnaryAST::UnaryAST(std::unique_ptr<IExpressionAST> && expr, UnaryAST::Operator op)
	: m_expr(std::move(expr))
	, m_op(op)
{
}

const IExpressionAST& UnaryAST::GetExpr()const
{
	return *m_expr;
}

UnaryAST::Operator UnaryAST::GetOperator()const
{
	return m_op;
}

void UnaryAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Identifier node
IdentifierAST::IdentifierAST(const std::string &name)
	: m_name(name)
{
}

const std::string& IdentifierAST::GetName()const
{
	return m_name;
}

void IdentifierAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

FunctionCallExpressionAST::FunctionCallExpressionAST(
	const std::string& name,
	std::vector<std::unique_ptr<IExpressionAST>>&& params
)
	: m_name(name)
	, m_params(std::move(params))
{
}

const std::string& FunctionCallExpressionAST::GetName()const
{
	return m_name;
}

size_t FunctionCallExpressionAST::GetParamsCount() const
{
	return m_params.size();
}

const IExpressionAST& FunctionCallExpressionAST::GetParam(size_t index)const
{
	if (index < m_params.size())
	{
		return *m_params[index];
	}
	throw std::runtime_error("index must be less than function call's params count");
}

void FunctionCallExpressionAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Variable declaration node
VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ExpressionType type)
	: m_identifier(std::move(identifier))
	, m_type(type)
	, m_expr(nullptr)
{
}

void VariableDeclarationAST::SetExpression(std::unique_ptr<IExpressionAST> && expr)
{
	m_expr = std::move(expr);
}

const IExpressionAST* VariableDeclarationAST::GetExpression()const
{
	return m_expr.get();
}

const IdentifierAST& VariableDeclarationAST::GetIdentifier()const
{
	return *m_identifier;
}

ExpressionType VariableDeclarationAST::GetType()const
{
	return m_type;
}

void VariableDeclarationAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Assign statement node
AssignStatementAST::AssignStatementAST(
	std::unique_ptr<IdentifierAST> && identifier,
	std::unique_ptr<IExpressionAST> && expr
)
	: m_identifier(std::move(identifier))
	, m_expr(std::move(expr))
{
}

const IdentifierAST& AssignStatementAST::GetIdentifier()const
{
	return *m_identifier;
}

const IExpressionAST& AssignStatementAST::GetExpr()const
{
	return *m_expr;
}

void AssignStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

ArrayElementAssignAST::ArrayElementAssignAST(
	std::unique_ptr<ArrayElementAccessAST> && access,
	std::unique_ptr<IExpressionAST> && expression
)
	: m_access(std::move(access))
	, m_expression(std::move(expression))
{
}

size_t ArrayElementAssignAST::GetIndexCount()const
{
	return m_access->GetIndexCount();
}

const std::string& ArrayElementAssignAST::GetName()const
{
	return m_access->GetName();
}

const IExpressionAST& ArrayElementAssignAST::GetIndex(size_t index /* = 0 */)const
{
	return m_access->GetIndex(index);
}

const IExpressionAST& ArrayElementAssignAST::GetExpression()const
{
	return *m_expression;
}

void ArrayElementAssignAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Return statement node
ReturnStatementAST::ReturnStatementAST(std::unique_ptr<IExpressionAST> && expression)
	: m_expression(std::move(expression))
{
}

const IExpressionAST* ReturnStatementAST::GetExpression()const
{
	return m_expression.get();
}

void ReturnStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// If statement node
IfStatementAST::IfStatementAST(
	std::unique_ptr<IExpressionAST> && expr,
	std::unique_ptr<IStatementAST> && then,
	std::unique_ptr<IStatementAST> && elif)
	: m_expr(std::move(expr))
	, m_then(std::move(then))
	, m_elif(std::move(elif))
{
}

void IfStatementAST::SetElseClause(std::unique_ptr<IStatementAST> && elif)
{
	m_elif = std::move(elif);
}

const IExpressionAST& IfStatementAST::GetExpr()const
{
	return *m_expr;
}

const IStatementAST& IfStatementAST::GetThenStmt()const
{
	return *m_then;
}

const IStatementAST* IfStatementAST::GetElseStmt()const
{
	return m_elif ? m_elif.get() : nullptr;
}

void IfStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// While statement node
WhileStatementAST::WhileStatementAST(
	std::unique_ptr<IExpressionAST> && expr,
	std::unique_ptr<IStatementAST> && stmt)
	: m_expr(std::move(expr))
	, m_stmt(std::move(stmt))
{
}

const IExpressionAST& WhileStatementAST::GetExpr()const
{
	return *m_expr;
}

const IStatementAST& WhileStatementAST::GetStatement()const
{
	return *m_stmt;
}

void WhileStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Composite statement node
void CompositeStatementAST::AddStatement(std::unique_ptr<IStatementAST> && stmt)
{
	m_statements.push_back(std::move(stmt));
}

const IStatementAST& CompositeStatementAST::GetStatement(size_t index)const
{
	if (index >= m_statements.size())
	{
		throw std::out_of_range("index must be less that statements count");
	}
	return *m_statements[index];
}

size_t CompositeStatementAST::GetCount()const
{
	return m_statements.size();
}

void CompositeStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Function node
FunctionAST::FunctionAST(
	boost::optional<ExpressionType> returnType,
	std::unique_ptr<IdentifierAST> && identifier,
	std::vector<Param> && params,
	std::unique_ptr<IStatementAST> && statement
)
	: m_returnType(returnType)
	, m_identifier(std::move(identifier))
	, m_params(std::move(params))
	, m_statement(std::move(statement))
{
}

boost::optional<ExpressionType> FunctionAST::GetReturnType()const
{
	return m_returnType;
}

const IdentifierAST& FunctionAST::GetIdentifier()const
{
	return *m_identifier;
}

const std::vector<FunctionAST::Param>& FunctionAST::GetParams()const
{
	return m_params;
}

const IStatementAST& FunctionAST::GetStatement()const
{
	return *m_statement;
}

// Program node (root)
void ProgramAST::AddFunction(std::unique_ptr<FunctionAST> && function)
{
	m_functions.push_back(std::move(function));
}

size_t ProgramAST::GetFunctionsCount()const
{
	return m_functions.size();
}

const FunctionAST& ProgramAST::GetFunction(size_t index)const
{
	if (index >= m_functions.size())
	{
		throw std::out_of_range("index must be less than functions count");
	}
	return *m_functions[index];
}

BuiltinCallStatementAST::BuiltinCallStatementAST(Builtin builtin)
	: m_builtin(builtin)
{
}

BuiltinCallStatementAST::Builtin BuiltinCallStatementAST::GetBuiltin()const
{
	return m_builtin;
}

size_t BuiltinCallStatementAST::GetParamsCount()const
{
	return m_params.size();
}

const IExpressionAST& BuiltinCallStatementAST::GetExpression(size_t index)const
{
	if (index >= m_params.size())
	{
		throw std::out_of_range("index must be less than expressions count");
	}
	return *m_params[index];
}

void BuiltinCallStatementAST::AddExpression(std::unique_ptr<IExpressionAST> && expression)
{
	m_params.push_back(std::move(expression));
}

void BuiltinCallStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

FunctionCallStatementAST::FunctionCallStatementAST(std::unique_ptr<FunctionCallExpressionAST> && call)
	: m_call(std::move(call))
{
}

const IExpressionAST& FunctionCallStatementAST::GetCall()const
{
	return *m_call;
}

const FunctionCallExpressionAST& FunctionCallStatementAST::GetCallAsDerived()const
{
	return *m_call;
}

void FunctionCallStatementAST::Accept(IStatementVisitor& visitor) const
{
	visitor.Visit(*this);
}

std::string ToString(BinaryExpressionAST::Operator operation)
{
	switch (operation)
	{
	case BinaryExpressionAST::Or:
		return "||";
	case BinaryExpressionAST::And:
		return "&&";
	case BinaryExpressionAST::Equals:
		return "==";
	case BinaryExpressionAST::NotEquals:
		return "!=";
	case BinaryExpressionAST::Less:
		return "<";
	case BinaryExpressionAST::LessOrEquals:
		return "<=";
	case BinaryExpressionAST::More:
		return ">";
	case BinaryExpressionAST::MoreOrEquals:
		return ">=";
	case BinaryExpressionAST::Plus:
		return "+";
	case BinaryExpressionAST::Minus:
		return "-";
	case BinaryExpressionAST::Mul:
		return "*";
	case BinaryExpressionAST::Div:
		return "/";
	case BinaryExpressionAST::Mod:
		return "%";
	}
	throw std::logic_error("ToString: can't cast undefined binary operator to string");
}

std::string ToString(UnaryAST::Operator operation)
{
	switch (operation)
	{
	case UnaryAST::Plus:
		return "+";
	case UnaryAST::Minus:
		return "-";
	case UnaryAST::Negation:
		return "!";
	}
	throw std::logic_error("ToString: can't cast undefined unary operator to string");
}
