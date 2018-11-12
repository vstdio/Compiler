#pragma once
#include "IParser.h"
#include "ASTNode.h"

#include "../grammarlib/LLParserTable.h"
#include "../Lexer/ILexer.h"

#include <unordered_map>

class LLParser : public IParser
{
public:
	explicit LLParser(std::unique_ptr<ILexer> && lexer, std::unique_ptr<LLParserTable> && table);

	std::unique_ptr<ASTNode> Parse(const std::string& text) override;
	void SetTokenMapping(TokenKind kind, const std::string& mapping);

private:
	std::unique_ptr<ILexer> m_lexer;
	std::unique_ptr<LLParserTable> m_table;
	std::unordered_map<TokenKind, std::string> m_tokensMap;
};
