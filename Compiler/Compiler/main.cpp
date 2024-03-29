#include "stdafx.h"
#include "CompilerDriver.h"
#include "../utils/file_utils.h"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	try
	{
		auto driver = std::make_unique<CompilerDriver>(std::cout);
		driver->Compile(file_utils::GetFileContent("input.txt"));
		driver->SaveIRToFile("output.ll");
		driver->SaveObjectCodeToFile("output.o");
	}
	catch (const std::exception& ex)
	{
		std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
