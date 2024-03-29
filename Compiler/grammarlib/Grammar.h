#pragma once
#include "GrammarProduction.h"

class Grammar
{
public:
	void AddProduction(std::shared_ptr<GrammarProduction> production);

	std::shared_ptr<const GrammarProduction> GetProduction(size_t index)const;
	size_t GetProductionsCount()const;

	const std::string& GetStartSymbol()const;
	const std::string& GetEndSymbol()const;

private:
	std::vector<std::shared_ptr<GrammarProduction>> m_productions;
};
