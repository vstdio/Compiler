#include "stdafx.h"
#include "../Lexer/Lexer.h"
#include "../Parser/Parser.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	// Пример с лексером
	Lexer lexer;
	lexer.SetText("some text written by me");

	while (true)
	{
		const Token token = lexer.Advance();
		if (token.kind == TokenKind::EndOfFile)
		{
			break;
		}
		std::cout << token.text << std::endl;
	}

	// Пример с парсером
	Parser parser(lexer);
	if (parser.Parse("hello"))
	{
		std::cout << "accepted" << std::endl;
	}
	else
	{
		std::cout << "not accepted" << std::endl;
	}

	return 0;
}
