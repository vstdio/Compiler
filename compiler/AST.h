#ifndef COMPILER_60MIN_AST_H
#define COMPILER_60MIN_AST_H

#include "ASTVisitor.h"
#include "ExpressionType.h"
#include <memory>
#include <string>
#include <vector>
#include <boost/variant.hpp>

class IExpressionAST
{
public:
    using Ptr = std::unique_ptr<IExpressionAST>;
    virtual ~IExpressionAST() = default;
    virtual void Accept(IExpressionVisitor& visitor)const = 0;
};

class BinaryExpressionAST : public IExpressionAST
{
public:
    enum Operator
    {
        Plus,
        Minus,
        Mul,
        Div
    };

    explicit BinaryExpressionAST(
            std::unique_ptr<IExpressionAST> && left,
            std::unique_ptr<IExpressionAST> && right,
            Operator op);

    const IExpressionAST& GetLeft()const;
    const IExpressionAST& GetRight()const;
    Operator GetOperator()const;

    void Accept(IExpressionVisitor& visitor)const override;

private:
    std::unique_ptr<IExpressionAST> m_left;
    std::unique_ptr<IExpressionAST> m_right;
    Operator m_op;
};

class LiteralConstantAST : public IExpressionAST
{
public:
    // TODO: add string literal
    using Value = boost::variant<int, float>;

    explicit LiteralConstantAST(const Value& value);
    const Value& GetValue()const;
    void Accept(IExpressionVisitor& visitor)const override;

private:
    Value m_value;
};

class UnaryAST : public IExpressionAST
{
public:
    enum Operator
    {
        Plus,
        Minus
    };

    explicit UnaryAST(std::unique_ptr<IExpressionAST> && expr, Operator op);

    const IExpressionAST& GetExpr()const;
    Operator GetOperator()const;

    void Accept(IExpressionVisitor& visitor)const override;

private:
    std::unique_ptr<IExpressionAST> m_expr;
    Operator m_op;
};

class IdentifierAST : public IExpressionAST
{
public:
    explicit IdentifierAST(const std::string& name);
    const std::string& GetName()const;

    void Accept(IExpressionVisitor& visitor)const override;

private:
    std::string m_name;
};

class IStatementAST
{
public:
    using Ptr = std::unique_ptr<IStatementAST>;
    virtual ~IStatementAST() = default;
    virtual void Accept(IStatementVisitor& visitor)const = 0;
};

class VariableDeclarationAST : public IStatementAST
{
public:
    explicit VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ExpressionType type);

    const IdentifierAST& GetIdentifier()const;
    ExpressionType GetType()const;

    void Accept(IStatementVisitor& visitor)const override;

private:
    std::unique_ptr<IdentifierAST> m_identifier;
    ExpressionType m_type;
};

class AssignStatementAST : public IStatementAST
{
public:
    explicit AssignStatementAST(
        std::unique_ptr<IdentifierAST> && identifier,
        std::unique_ptr<IExpressionAST> && expr);

    const IdentifierAST& GetIdentifier()const;
    const IExpressionAST& GetExpr()const;

    void Accept(IStatementVisitor& visitor)const override;

private:
    std::unique_ptr<IdentifierAST> m_identifier;
    std::unique_ptr<IExpressionAST> m_expr;
};

class ReturnStatementAST : public IStatementAST
{
public:
    explicit ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr);

    const IExpressionAST& GetExpr()const;
    void Accept(IStatementVisitor& visitor)const override;

private:
    std::unique_ptr<IExpressionAST> m_expr;
};

class IfStatementAST : public IStatementAST
{
public:
    explicit IfStatementAST(
            std::unique_ptr<IExpressionAST> && expr,
            std::unique_ptr<IStatementAST> && then,
            std::unique_ptr<IStatementAST> && elif = nullptr);

    void SetElseClause(std::unique_ptr<IStatementAST> && elif);
    const IExpressionAST& GetExpr()const;
    const IStatementAST& GetThenStmt()const;
    const IStatementAST* GetElseStmt()const;

    void Accept(IStatementVisitor& visitor)const override;

private:
    std::unique_ptr<IExpressionAST> m_expr;
    std::unique_ptr<IStatementAST> m_then;
    std::unique_ptr<IStatementAST> m_elif;
};

class WhileStatementAST : public IStatementAST
{
public:
    explicit WhileStatementAST(
            std::unique_ptr<IExpressionAST> && expr,
            std::unique_ptr<IStatementAST> && stmt);

    const IExpressionAST& GetExpr()const;
    const IStatementAST& GetStatement()const;

    void Accept(IStatementVisitor& visitor)const override;

private:
    std::unique_ptr<IExpressionAST> m_expr;
    std::unique_ptr<IStatementAST> m_stmt;
};

class CompositeStatementAST : public IStatementAST
{
public:
    void AddStatement(std::unique_ptr<IStatementAST> && stmt);
    const IStatementAST& GetStatement(size_t index)const;
    size_t GetCount()const;

    void Accept(IStatementVisitor& visitor)const override;

private:
    std::vector<std::unique_ptr<IStatementAST>> m_statements;
};

class FunctionAST
{
public:
    explicit FunctionAST(std::unique_ptr<IStatementAST> && stmt);

private:
    std::unique_ptr<IStatementAST> m_stmt;
};

class ProgramAST
{
public:
    void AddFunction(std::unique_ptr<FunctionAST> && function);

private:
    std::vector<std::unique_ptr<FunctionAST>> m_functions;
};

#endif //COMPILER_60MIN_AST_H
