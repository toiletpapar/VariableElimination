#include <iostream>
#include <vector>
#include <string>

#include "Factor.h"

using namespace std;

int main () {
	vector<Variable<bool>*> variables;
	vector<bool> domain = { true, false };

	variables.push_back(new Variable<bool>("A", domain));
	variables.push_back(new Variable<bool>("B", domain));
	variables.push_back(new Variable<bool>("C", domain));
	variables.push_back(new Variable<bool>("D", domain));

	Factor<bool> f = Factor<bool>(variables);
	f.set_value(vector<bool>({ true, true, true, true }), 1);
	f.set_value(vector<bool>({ true, true, false, true }), 2);
	f.set_value(vector<bool>({ true, false, true, true }), 3);
	f.set_value(vector<bool>({ false, true, true, true }), 4);
	f.set_value(vector<bool>({ true, false, false, true }), 5);
	f.set_value(vector<bool>({ false, false, true, true }), 6);
	f.set_value(vector<bool>({ false, true, false, true }), 7);
	f.set_value(vector<bool>({ false, false, false, true }), 8);

	f.set_value(vector<bool>({ true, true, true, false }), 9);
	f.set_value(vector<bool>({ true, true, false, false }), 10);
	f.set_value(vector<bool>({ true, false, true, false }), 11);
	f.set_value(vector<bool>({ false, true, true, false }), 12);
	f.set_value(vector<bool>({ true, false, false, false }), 13);
	f.set_value(vector<bool>({ false, false, true, false }), 14);
	f.set_value(vector<bool>({ false, true, false, false }), 15);
	f.set_value(vector<bool>({ false, false, false, false }), 16);

	//f = Factor<bool>::restrict(f, variables.at(0), false);

	Factor<bool>* f2 = Factor<bool>::sumout(f, variables.at(3));

	//Print partial instantiation of variable
	//PartialInstantiations<bool> partial_instantiations = get_instantiation(variables, vector<Variable<bool>*>({variables.at(1), variables.at(2)}));

	//for (unsigned int i = 0; i < partial_instantiations.instantiations.size(); ++i) {
	//	for (unsigned int j = 0; j < partial_instantiations.instantiations.at(i).size(); ++j) {
	//		cout << partial_instantiations.instantiations.at(i).at(j) << " ";
	//	}

	//	cout << endl;
	//}
	
	//Print the factor
	f2->print_table();
	delete f2;

	for (unsigned int i = 0; i < variables.size(); ++i) {
		delete variables.at(i);
	}

	return 0;
}