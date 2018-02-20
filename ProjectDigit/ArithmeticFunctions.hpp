#pragma once

#include <cmath>
#include "Function.hpp"
#include "FunctionAllocator.hpp"

using namespace std;

class AddFunction :public Function {
public:
	const string getFunctionName() override { return "add"; }
	const string getTypicalCallPattern() override { return "add(x, y)"; }
	const size_t getParameterCount() override { return 2u; }
private:
	const double _calculate(const vector<double>& params) override { return params[0] + params[1]; }
};

class MinusFunction :public Function {
public:
	const string getFunctionName() override { return "minus"; }
	const string getTypicalCallPattern() override { return "minus(base, subtractor)"; }
	const size_t getParameterCount() override { return 2u; }
private:
	const double _calculate(const vector<double>& params) override { return params[0] - params[1]; }
};

class MultplyFunction :public Function {
public:
	const string getFunctionName() override { return "multply"; }
	const string getTypicalCallPattern() override { return "multply(factor1, factor2)"; }
	const size_t getParameterCount() override { return 2u; }
private:
	const double _calculate(const vector<double>& params) override { return params[0] * params[1]; }
};

class DivisionFunction :public Function {
public:
	const string getFunctionName() override { return "divide"; }
	const string getTypicalCallPattern() override { return "divide(base, divisor)"; }
	const size_t getParameterCount() override { return 2u; }
private:
	const double _calculate(const vector<double>& params) override { return params[0] / params[1]; }
};

class PowerFunction :public Function {
public:
	const string getFunctionName() override { return "power"; }
	const string getTypicalCallPattern() override { return "power(base, exponent)"; }
	const size_t getParameterCount() override { return 2u; }
private:
	const double _calculate(const vector<double>& params) override { return pow(params[0], params[1]); }
};


class ArithmeticFunctionAllocator :public FunctionAllocator {
	void _initalaize(set<string>& functionNames) override {
		functionNames.insert("add");
		functionNames.insert("minus");
		functionNames.insert("multply");
		functionNames.insert("divide");
		functionNames.insert("power");
	}

	Function* _allocate(const string& name) override {
		Function* func = nullptr;
		if (name == "add")
			func = new AddFunction();
		else if (name == "minus")
			func = new MinusFunction();
		else if (name == "multply")
			func = new MultplyFunction();
		else if (name == "divide")
			func = new DivisionFunction();
		else if (name == "power")
			func = new PowerFunction();
		return func;
	}
};
