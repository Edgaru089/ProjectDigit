#pragma once

#include <vector>
#include <map>
#include <memory>

using namespace std;


class Function {
public:

	class ParamCountMismatchException :public exception {
	public:
		ParamCountMismatchException(const string functionName, const string typicalCallPattern) {}
	};

	void create(const vector<shared_ptr<Function>> params) {
		if (params.size() != getParameterCount())
			throw ParamCountMismatchException(getFunctionName(), getTypicalCallPattern());
		else
			this->params = params;
	}

	const double calculate() {
		vector<double> vec;
		vec.reserve(params.size());
		for (auto i : params)
			vec.push_back(i->calculate());
		return _calculate(vec);
	}

	virtual const string getFunctionName() = 0;
	virtual const string getTypicalCallPattern() = 0;
	virtual const size_t getParameterCount() = 0;

private:

	virtual const double _calculate(const vector<double>& params) = 0;

	vector<shared_ptr<Function>> params;
};
