#include "stdafx.h"
#include "ExpressionTypeAST.h"

namespace
{
const std::unordered_map<ExpressionTypeAST, std::unordered_set<ExpressionTypeAST>> gcAvailableCasts = {
	{ ExpressionTypeAST::Int, { ExpressionTypeAST::Float, ExpressionTypeAST::Bool } },
	{ ExpressionTypeAST::Float, { ExpressionTypeAST::Int, ExpressionTypeAST::Bool } },
	{ ExpressionTypeAST::Bool, { ExpressionTypeAST::Int, ExpressionTypeAST::Float } }
};

const std::map<std::pair<ExpressionTypeAST, ExpressionTypeAST>, ExpressionTypeAST> gcBinaryCasts = {
	{ { ExpressionTypeAST::Int, ExpressionTypeAST::Float }, ExpressionTypeAST::Float },
	{ { ExpressionTypeAST::Float, ExpressionTypeAST::Int }, ExpressionTypeAST::Float }
};
}

bool Convertible(ExpressionTypeAST from, ExpressionTypeAST to)
{
	if (from == to)
	{
		throw std::runtime_error("trying to convert from '" + ToString(from) +  "' to itself");
	}

	auto found = gcAvailableCasts.find(from);
	if (found == gcAvailableCasts.end())
	{
		return false;
	}

	const std::unordered_set<ExpressionTypeAST> & availableCasts = found->second;
	return availableCasts.find(to) != availableCasts.end();
}

bool ConvertibleToBool(ExpressionTypeAST type)
{
	return Convertible(type, ExpressionTypeAST::Bool);
}

boost::optional<ExpressionTypeAST> GetPreferredTypeFromBinaryExpression(
	ExpressionTypeAST left, ExpressionTypeAST right)
{
	if (left == right)
	{
		return left;
	}

	auto found = gcBinaryCasts.find(std::make_pair(left, right));
	if (found == gcBinaryCasts.end())
	{
		return boost::none;
	}
	return found->second;
}

std::string ToString(ExpressionTypeAST type)
{
	switch (type)
	{
	case ExpressionTypeAST::Int:
		return "Int";
	case ExpressionTypeAST::Float:
		return "Float";
	case ExpressionTypeAST::Bool:
		return "Bool";
	case ExpressionTypeAST::String:
		return "String";
	default:
		throw std::logic_error("can't get string representation of undefined expression type");
	}
}
