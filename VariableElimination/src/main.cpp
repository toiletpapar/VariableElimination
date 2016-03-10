#include <iostream>
#include <vector>
#include <string>

#include "Factor.h"

using namespace std;

int main () {
	vector<Variable<bool>*> f1_variables, f2_variables;
	vector<bool> domain = { true, false };

	f1_variables.push_back(new Variable<bool>("A", domain));
	f1_variables.push_back(new Variable<bool>("B", domain));

	f2_variables.push_back(new Variable<bool>("B", domain));
	f2_variables.push_back(new Variable<bool>("C", domain));

	Factor<bool> f1 = Factor<bool>(f1_variables);
	f1.set_value(vector<bool>({ true, true }), 0.9);
	f1.set_value(vector<bool>({ true, false }), 0.1);
	f1.set_value(vector<bool>({ false, true }), 0.4);
	f1.set_value(vector<bool>({ false, false }), 0.6);

	Factor<bool> f2 = Factor<bool>(f2_variables);
	f2.set_value(vector<bool>({ true, true }), 0.7);
	f2.set_value(vector<bool>({ true, false }), 0.3);
	f2.set_value(vector<bool>({ false, true }), 0.8);
	f2.set_value(vector<bool>({ false, false }), 0.2);

	Factor<bool>* f3 = Factor<bool>::multiply(f1, f2);
	
	//Print the factor
	f3->print_table();

	for (unsigned int i = 0; i < f1_variables.size(); ++i) {
		delete f1_variables.at(i);
	}

	for (unsigned int i = 0; i < f2_variables.size(); ++i) {
		delete f2_variables.at(i);
	}

	delete f3;

	return 0;
}