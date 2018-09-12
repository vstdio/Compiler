#include "stdafx.h"
#include "../Lexer/Lexer.h"
#include "Table.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	using namespace FormatUtils;
	Table table;
	table.Append({ "Column1", "Column2", "Column3" });
	table.Append({ "Value1", "Value2", "Value3" });
	std::cout << table << std::endl;

	return 0;
}
