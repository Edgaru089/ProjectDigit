#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <memory>
#include <cctype>
#include <stack>

#include "FunctionAllocator.hpp"
#include "BasicFunctions.hpp"
#include "LogSystem.hpp"

#include "Main.hpp"

USING_NAMESPACE;

class Script {
public:

	friend class ScriptParser;

	enum Mode {
		Expression,
		FunctionStream
	};

	Script() :mode(FunctionStream) {}

	class DisplayFunction {
	public:

		// No effect if variable undefined
		void changeVVal(string name, double value) {
			map<string, shared_ptr<Variable>>::iterator i = changeVal.find(name);
			if (i != changeVal.end())
				i->second->setValue(value);
		}

		// No effect if variable undefined
		void changeCVal(string name, double value) {
			map<string, shared_ptr<Variable>>::iterator i = constVal.find(name);
			if (i != constVal.end()) {
				i->second->setValue(value);
				//constValD[name] = value;
			}
		}

		void changeXCoord(double value) {
			for (pair<const string, shared_ptr<Variable>>& i : xCoordVal) {
				i.second->setValue(value);
			}
		}

		const double calculate() { return func->calculate(); }

		void reinitalaizeConstVals() {
			//for (pair<const string, double>& i : constValD) {
				//constVal[i.first]->setValue(i.second);
			//}
		}

		// Default value: ZERO
		map<string, shared_ptr<Variable>> changeVal, constVal, xCoordVal;
		//map<string, double> constValD;
		shared_ptr<Function> func;
		string name;
	};

	set<string> valNames;
	map<string, shared_ptr<Function>> functions;
	set<string> usedFunctions;
	map<string, shared_ptr<Variable>> val;
	vector<DisplayFunction> displays;

	Mode mode;
};

class ScriptParser {
public:

	class SyntaxErrorException:public exception {
	public:
		SyntaxErrorException(const string& what) :str(what) {}
		const char* what() { return str.c_str(); }
	private:
		const string str;
	};

	static const bool parseFromFile(Script& script, const string filename) {
		mlog << "[Script] Prasing scrpit file: " << filename << dlog;
		script = Script();

		ifstream fin;
		fin.open(filename);
		if (!fin.good()) {
			mlog << "         File access failed." << dlog;
			return false;
		}

		string str;
		char buf[2048];
		int line = 0;
		while (!fin.getline(buf, 2048).eof()) {
			str = buf;
			line++;

			if (!parseLine(script, str, line, true))
				return false;
		}
		mlog << "         File parsing done." << dlog;
		return true;
	}

	static const bool parseLine(Script& script, string str, int line = -1, bool emptyScriptOnFailure = false) {
		Script _script;
		if (!emptyScriptOnFailure)
			_script = script;
		// This line is a comment; ignoring
		if (str[0] == '#')
			return true;
		try {
			if (line != -1)
				mlog << Log::Debug << "[Script] Line " << line << ":" << dlog;
			else
				mlog << "[Script] Parsing Immediate Window input: " << str << dlog;
			if (str.substr(0u, 6u) == "defval") {
				_defval(str.substr(6u), script);
			}
			else if (str.substr(0u, 7u) == "deffunc") {
				_deffunc(str.substr(7u), script);
			}
			else if (str.substr(0u, 11u) == "displayfunc") {
				_displayfunc(str.substr(11u), script);
			}
			else if (str.substr(0u, 5u) == "clear") {
				script = Script();
			}
			else if (str.substr(0u, 4u) == "mode") {
				int i = 4;
				while (!isalnum(str[i]))i++;
				if (str[i] == '0' || str.substr(i, 4u) == "func"s)
					script.mode = Script::FunctionStream;
				else if (str[i] == '1' || str.substr(i, 4u) == "expr"s)
					script.mode = Script::Expression;
			}
		} catch (SyntaxErrorException e) {
			mlog << Log::Error << "[Script/EXCEPTION] ERROR(" << (line == -1 ? "Immediate"s : StringParser::toStringFormatted(
				"Line %d", line
			)) << "): " << e.what() << dlog;
			if (desktop != NULL)
				sfMessageBox(*desktop, "[Script/EXCEPTION] ERROR("s + (line == -1 ? "Immediate"s : StringParser::toStringFormatted(
					"Line %d"s, line
				)) + "): "s + e.what(), L"Error");
			script = _script;
			return false;
		}
		return true;
	}

private:

	static void _defval(const string& vallist, Script& script) {
		mlog << Log::Debug << "[Script] Parsing variable define list: " << vallist << dlog;
		int i = 0;
		while (i < vallist.size()) {
			string name = "";

			// First look for an alphabet
			while (!isalpha(vallist[i]))i++;
			// Read it through
			while (isalpha(vallist[i]) || isdigit(vallist[i])) {
				name += vallist[i];
				i++;
			}
			// Add the variable name
			mlog << Log::Debug << "         Inserted: " << name << dlog;
			script.valNames.insert(name);
			script.val.insert(pair<string, shared_ptr<Variable>>(name, make_shared<Variable>()));
			// Look for an comma; abort if there are none
			while (vallist[i] != ','&&i < vallist.size())i++;
		}
	}

	static void _parseFunctionParams(const std::string& str, int& i, std::vector<std::string> &strparams) {
		// Find the function paramlist
		while (str[i] != '(')i++;
		// Spread the function params
		while (i < str.size()) {

			string param = "";
			// Find an alphabet or number or decimal dot or minus(negative) sign
			while (!(isalnum(str[i]) || str[i] == '.' || str[i] == '-'))i++;
			// Read the function name
			while (isalpha(str[i]) || isdigit(str[i]) || str[i] == '.' || str[i] == '-') {
				param += str[i];
				i++;
			}

			// Parse the paramlist
			int level = 0;
			while (i < str.size()) {

				if (level == 0 && (str[i] == ',' || str[i] == ')')) {
					strparams.push_back(param);
					break;
				}

				if (str[i] == '(')
					level++;
				if (str[i] == ')')
					level--;

				param += str[i];
				i++;
			}

			if (str[i] == ')')
				break;
		}
	}

	static shared_ptr<Function> _parseFunction(const string& str, Script& script) {

		mlog << Log::Debug << "[Script] Parsing function string: " << str << dlog;

		shared_ptr<Function> func;
		int i = 0;
		string funcName;

		// First look for an alphabet or number or decimal dot or minus(negative) sign
		while (!(isalnum(str[i]) || str[i] == '.' || str[i] == '-'))i++;
		// Read the function name
		while (isalpha(str[i]) || isdigit(str[i]) || str[i] == '.' || str[i] == '-') {
			funcName += str[i];
			i++;
		}

		mlog << Log::Debug << "         Function name: " << funcName << dlog;

		// If this is a constant value
		if (isdigit(funcName[0]) || funcName[0] == '.' || funcName[0] == '-') {
			mlog << Log::Debug << "         Constant value" << dlog;
			return make_shared<Variable>(Variable(StringParser::toDouble(funcName))); // Return variable pointer 
		}

		// If this is a variable
		if (script.valNames.find(funcName) != script.valNames.end()) {
			mlog << Log::Debug << "         Variable" << dlog;
			return script.val[funcName]; // Return variable pointer; bye!
		}

		// Allocate the function
		map<string, shared_ptr<Function>>::iterator iter;
		if ((func = functionAllocatorManager.allocate(funcName)) == nullptr)
			if ((iter = script.functions.find(funcName)) == script.functions.end())
				throw SyntaxErrorException("Function \""s + funcName + "\" not defined by script standard or prevouis script"s);
			else
				func = iter->second;
		else {
			vector<shared_ptr<Function>> params;
			vector<string> strparams;

			_parseFunctionParams(str, i, strparams);

			for (string& i : strparams)
				params.push_back(_parseFunction(i, script));

			try {
				func->create(params);
			} catch (Function::ParamCountMismatchException e) {
				throw SyntaxErrorException("Function \""s + funcName + "\" parameter count mismatch"s);
			}
		}

		return func;
	}

	// Parse function in Expression Mode
	static shared_ptr<Function> _parseFunctionExpr(const string& str, Script& script) {
		mlog << "[Script/EXPR] Parsing Expression: " << str << dlog;

		stack<char> adds, muls;
		stack<shared_ptr<Function>> vals;

		auto calcMultplyStack = [&]() {
			while (!muls.empty()) {
				char c = muls.top();
				shared_ptr<Function> result;
				muls.pop();
				shared_ptr<Function> val1, val2;
				val2 = vals.top(); vals.pop();
				val1 = vals.top(); vals.pop();
				if (c == '*') // Multply
					result = functionAllocatorManager.allocate("multply"s);
				else if (c == '/') // Divide
					result = functionAllocatorManager.allocate("divide"s);
				result->create({ val1,val2 });
				vals.push(result);
			}
			mlog << Log::Debug << "              Multply Stack Cleared" << dlog;
		};
		auto calcAddStack = [&]() {
			while (!adds.empty()) {
				char c = adds.top();
				shared_ptr<Function> result;
				adds.pop();
				shared_ptr<Function> val1, val2;
				val2 = vals.top(); vals.pop();
				val1 = vals.top(); vals.pop();
				if (c == '+') // Add
					result = functionAllocatorManager.allocate("add"s);
				else if (c == '-') // Minus
					result = functionAllocatorManager.allocate("minus"s);
				result->create({ val1,val2 });
				vals.push(result);
			}
			mlog << Log::Debug << "              Addition Stack Cleared" << dlog;
		};

		int i = 0;
		while (i < str.size()) {

			string funcName = ""s;
			// Find function / number
			while (!(isalnum(str[i]) || str[i] == '.' || str[i] == '-'))i++;
			// Read function / number
			while (isalnum(str[i]) || str[i] == '.' || str[i] == '-') {
				funcName += str[i];
				i++;
			}
			mlog << Log::Debug << "              Read function name: " << funcName << dlog;
			// If number -> new variable
			if (isdigit(funcName[0]) || funcName[0] == '.' || funcName[0] == '-') {
				mlog << Log::Debug << "              Constant" << dlog;
				vals.push(make_shared<Variable>(Variable(StringParser::toDouble(funcName)))); // Return variable pointer 
			} // Else -> function / variable
			else {
				// If this is a variable
				if (script.valNames.find(funcName) != script.valNames.end()) {
					mlog << Log::Debug << "              Variable" << dlog;
					vals.push(script.val[funcName]); // Return variable pointer; bye!
				}
				else {
					mlog << Log::Debug << "              Function" << dlog;
					// Allocate the function
					shared_ptr<Function> func;
					map<string, shared_ptr<Function>>::iterator iter;
					if ((func = functionAllocatorManager.allocate(funcName)) == nullptr)
						if ((iter = script.functions.find(funcName)) == script.functions.end())
							throw SyntaxErrorException("Function \""s + funcName + "\" not defined by script standard or prevouis script"s);
						else
							func = iter->second;
					else {
						vector<shared_ptr<Function>> params;
						vector<string> strparams;

						_parseFunctionParams(str, i, strparams);

						stringstream ss;
						for (string& i : strparams)
							ss << " \"" << i << "\"";
						mlog << Log::Debug << "              Params:" << ss.str() << dlog;

						for (string& i : strparams)
							params.push_back(_parseFunctionExpr(i, script));

						try {
							func->create(params);
						} catch (Function::ParamCountMismatchException e) {
							throw SyntaxErrorException("Function \""s + funcName + "\" parameter count mismatch"s);
						}
					}
					vals.push(func);
				}

			}

			//Find operator (+, -, *, /)
			while (i < str.length() && str[i] != '+'&&str[i] != '-'&&str[i] != '*'&&str[i] != '/')i++;

			// If out of range -> finished; break
			if (i >= str.length())
				break;

			char op = str[i]; i++; // Get operator
			mlog << Log::Debug << "              Get operator: " << op << dlog;

			if (op == '+' || op == '-') { // Add operation
				calcMultplyStack();
				adds.push(op);
			}
			else if (op == '*' || op == '/') // Multply operation
				muls.push(op);

		}

		// It is possible that a multply stream is pending; clear both stacks
		calcMultplyStack();
		calcAddStack();

		return vals.top();
	}

	static void _deffunc(const string& str, Script& script) {
		shared_ptr<Function> func;
		int i = 0;
		string functionName;

		// First look for an alphabet
		while (!isalpha(str[i]))i++;
		// Read the function name
		while (isalpha(str[i]) || isdigit(str[i])) {
			functionName += str[i];
			i++;
		}

		mlog << Log::Debug << "[Script] Parsing function: " << functionName << dlog;

		// If prevouisly defined -> syntax error
		if (script.functions.find(functionName) != script.functions.end())
			throw SyntaxErrorException("Function \""s + functionName + "\" prevouisly defined"s);

		// Find the equal sign
		while (str[i] != '=')i++;
		// Look for the function expression
		while (!(isalnum(str[i]) || str[i] == '.' || str[i] == '-'))i++;

		if (script.mode == Script::FunctionStream)
			func = _parseFunction(str.substr(i), script);
		else if (script.mode == Script::Expression)
			func = _parseFunctionExpr(str.substr(i), script);
		script.functions.insert(pair<string, shared_ptr<Function>>(functionName, func));

	}

	static void _displayfunc(const string& str, Script& script) {
		mlog << Log::Debug << "[Script] Parsing function display sentence: " << str << dlog;

		Script::DisplayFunction func;
		string funcName;
		int i = 0;

		// Find 'y' and '=' chars
		while (str[i] != 'y')i++;
		while (str[i] != '=')i++;
		// Find the next alphabet
		while (!isalpha(str[i]))i++;
		// Read the function name
		while (isalpha(str[i]) || isdigit(str[i])) {
			funcName += str[i];
			i++;
		}
		mlog << Log::Debug << "         Function name:" << funcName << dlog;

		if (script.functions.find(funcName) == script.functions.end())
			throw SyntaxErrorException("Function \""s + funcName + "\" undefined"s);

		// One function can only be used once
		if (script.usedFunctions.find(funcName) != script.usedFunctions.end())
			throw SyntaxErrorException("Function \""s + funcName + "\" used more than once"s);
		else
			script.usedFunctions.insert(funcName);

		func.name = funcName;
		func.func = script.functions[funcName];

		// Find the "where" keyword
		while (str[i] != 'w')i++;
		// Skip the word
		while (isalpha(str[i]))i++;

		string valn, word;
		while (i < str.size()) {
			// Read a word (variable name)
			valn = word = "";
			while (!isalpha(str[i]))i++;
			while (isalnum(str[i])) {
				valn += str[i];
				i++;
			}
			// Find the equal sign
			while (str[i] != '=')i++;
			// Read another word (variable value)
			while (!(isalnum(str[i]) || str[i] == '.' || str[i] == '-'))i++;
			while (i < str.size() && (isalnum(str[i]) || str[i] == '.' || str[i] == '-')) {
				word += str[i];
				i++;
			}

			mlog << Log::Debug << "         Function variable:" << valn << " = " << word << dlog;

			map<string, shared_ptr<Variable>>::iterator iter = script.val.find(valn);
			if (iter == script.val.end())
				throw SyntaxErrorException("Variable \""s + valn + "\" undefined"s);
			shared_ptr<Variable> val = iter->second;
			val->setValue(0.0);

			if (word == "xcoord"s)
				func.xCoordVal.insert(pair<string, shared_ptr<Variable>>(valn, val));
			else if (word == "setval"s)
				func.changeVal.insert(pair<string, shared_ptr<Variable>>(valn, val));
			else if (isdigit(word[0]) || word[0] == '.' || word[0] == '-') {
				func.constVal.insert(pair<string, shared_ptr<Variable>>(valn, val));
				func.changeCVal(valn, StringParser::toDouble(word));
			}
			else
				throw SyntaxErrorException("Value \""s + word + "\" of variable \""s + valn + "\" unidentified"s);

			// Find the next word or abort if failure
			while (i < str.size() && !isalpha(str[i]))i++;
		}

		script.displays.push_back(func);

	}


};


/*
#Comment
#Circle

defval r, x, xc, yc

deffunc circleHigh=add(sqrt(minus(sqr(r), sqr(minus(x, xc)))), yc)
deffunc circleLow=minus(0, add(sqrt(minus(sqr(r), sqr(minus(x, xc)))), yc))

displayfunc y=circleHigh where x=xcoord r=2.5 xc=setval yc=setval
displayfunc y=circleLow where x=xcoord r=2.5 xc=setval yc=setval
*/
