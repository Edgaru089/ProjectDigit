#pragma once

#include "Function.hpp"

class Variable:public Function {
public:

	Variable() :val(0.0) {}
	Variable(const double val) :val(val) {}

	virtual const string getFunctionName() override { return "variable"; }
	virtual const string getTypicalCallPattern() override { return "variable"; }
	virtual const size_t getParameterCount() override { return 0u; }

	void setValue(const double val) { this->val = val; }
	const double getValue() { return val; }

private:
	virtual const double _calculate(const vector<double>& params) override { return val; }
	double val;
};
