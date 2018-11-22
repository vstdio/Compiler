#include "AST.h"

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


VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ExpressionType type)
    : m_identifier(std::move(identifier))
    , m_type(type)
{
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


AssignStatementAST::AssignStatementAST(
    std::unique_ptr<IdentifierAST> && identifier,
    std::unique_ptr<IExpressionAST> && expr)
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


ReturnStatementAST::ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr)
    : m_expr(std::move(expr))
{
}

const IExpressionAST& ReturnStatementAST::GetExpr()const
{
    return *m_expr;
}

void ReturnStatementAST::Accept(IStatementVisitor& visitor)const
{
    visitor.Visit(*this);
}


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


FunctionAST::FunctionAST(std::unique_ptr<IStatementAST> && stmt)
    : m_stmt(std::move(stmt))
{
}


void ProgramAST::AddFunction(std::unique_ptr<FunctionAST> && function)
{
    m_functions.push_back(std::move(function));
}