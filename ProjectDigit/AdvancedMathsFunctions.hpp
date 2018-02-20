#pragma once

#include <cmath>
#include "Function.hpp"
#include "FunctionAllocator.hpp"

using namespace std;

class SquareFunction :public Function {
public:
	const string getFunctionName() override { return "sqr"; }
	const string getTypicalCallPattern() override { return "sqr(x)"; }
	const size_t getParameterCount() override { return 1u; }
private:
	const double _calculate(const vector<double>& params) override { return params[0] * params[0]; }
};

class SquareRootFunction :public Function {
public:
	const string getFunctionName() override { return "sqrt"; }
	const string getTypicalCallPattern() override { return "sqrt(x)"; }
	const size_t getParameterCount() override { return 1u; }
private:
	const double _calculate(const vector<double>& params) override { return sqrt(params[0]); }
};

class SineFunction:public Function {
public:
	const string getFunctionName() override { return "sin"; }
	const string getTypicalCallPattern() override { return "sin(x)"; }
	const size_t getParameterCount() override { return 1u; }
private:
	const double _calculate(const vector<double>& params) override { return sin(params[0]); }
};

class CosineRootFunction:public Function {
public:
	const string getFunctionName() override { return "cos"; }
	const string getTypicalCallPattern() override { return "cos(x)"; }
	const size_t getParameterCount() override { return 1u; }
private:
	const double _calculate(const vector<double>& params) override { return cos(params[0]); }
};

class TangentRootFunction:public Function {
public:
	const string getFunctionName() override { return "tan"; }
	const string getTypicalCallPattern() override { return "tan(x)"; }
	const size_t getParameterCount() override { return 1u; }
private:
	const double _calculate(const vector<double>& params) override { return tan(params[0]); }
};

class AdvMathsFunctionAllocator :public FunctionAllocator {
	void _initalaize(set<string>& functionNames) override {
		functionNames.insert("sqr");
		functionNames.insert("sqrt");
		functionNames.insert("sin");
		functionNames.insert("cos");
		functionNames.insert("tan");
	}

	Function* _allocate(const string& name) override {
		Function* func = nullptr;
		if (name == "sqr")
			func = new SquareFunction();
		else if (name == "sqrt")
			func = new SquareRootFunction();
		else if (name == "sin")
			func = new SineFunction();
		else if (name == "cos")
			func = new CosineRootFunction();
		else if (name == "tan")
			func = new TangentRootFunction();
		return func;
	}
};
