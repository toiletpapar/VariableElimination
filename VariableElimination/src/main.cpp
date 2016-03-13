#include <iostream>
#include <vector>
#include <string>

#include "Factor.h"

using namespace std;

int main () {
	//Setup the variables
	vector<bool> domain = { true, false };

	Variable<bool>* FM = new Variable<bool>("FM", domain);
	Variable<bool>* NA = new Variable<bool>("NA", domain);
	Variable<bool>* NDG = new Variable<bool>("NDG", domain);
	Variable<bool>* FH = new Variable<bool>("FH", domain);
	Variable<bool>* FS = new Variable<bool>("FS", domain);
	Variable<bool>* FB = new Variable<bool>("FB", domain);

	vector<Variable<bool>*> all_variables = vector<Variable<bool>*>({ FM, NA, NDG, FH, FS, FB });
	vector<Variable<bool>*> FM_variables = vector<Variable<bool>*>({ FM });
	vector<Variable<bool>*> NA_variables = vector<Variable<bool>*>({ NA });
	vector<Variable<bool>*> NDG_variables = vector<Variable<bool>*>({ NDG, FM, NA });
	vector<Variable<bool>*> FH_variables = vector<Variable<bool>*>({ FH, FM, NDG, FS });
	vector<Variable<bool>*> FS_variables = vector<Variable<bool>*>({ FS });
	vector<Variable<bool>*> FB_variables = vector<Variable<bool>*>({ FB, FS });

	//Setup the conditional probability tables
	Factor<bool> FM_cond = Factor<bool>(FM_variables);
	FM_cond.set_value(vector<bool>({ true }), 0.04);
	FM_cond.set_value(vector<bool>({ false }), 0.96);

	Factor<bool> NA_cond = Factor<bool>(NA_variables);
	NA_cond.set_value(vector<bool>({ true }), 0.3);
	NA_cond.set_value(vector<bool>({ false }), 0.7);

	Factor<bool> NDG_cond = Factor<bool>(NDG_variables);
	NDG_cond.set_value(vector<bool>({ true, true, true }), 0.8);
	NDG_cond.set_value(vector<bool>({ true, false, true }), 0.5);
	NDG_cond.set_value(vector<bool>({ true, true, false }), 0.4);
	NDG_cond.set_value(vector<bool>({ true, false, false }), 0.0);
	NDG_cond.set_value(vector<bool>({ false, true, true }), 0.2);
	NDG_cond.set_value(vector<bool>({ false, false, true }), 0.5);
	NDG_cond.set_value(vector<bool>({ false, true, false }), 0.6);
	NDG_cond.set_value(vector<bool>({ false, false, false }), 1.0);

	Factor<bool> FH_cond = Factor<bool>(FH_variables);
	FH_cond.set_value(vector<bool>({ true, true, true, true }), 0.99);
	FH_cond.set_value(vector<bool>({ true, false, true, true }), 0.75);
	FH_cond.set_value(vector<bool>({ true, true, false, true }), 0.9);
	FH_cond.set_value(vector<bool>({ true, true, true, false }), 0.65);
	FH_cond.set_value(vector<bool>({ true, false, true, false }), 0.2);
	FH_cond.set_value(vector<bool>({ true, true, false, false }), 0.4);
	FH_cond.set_value(vector<bool>({ true, false, false, true }), 0.5);
	FH_cond.set_value(vector<bool>({ true, false, false, false }), 0.0);
	FH_cond.set_value(vector<bool>({ false, true, true, true }), 0.01);
	FH_cond.set_value(vector<bool>({ false, false, true, true }), 0.25);
	FH_cond.set_value(vector<bool>({ false, true, false, true }), 0.1);
	FH_cond.set_value(vector<bool>({ false, true, true, false }), 0.35);
	FH_cond.set_value(vector<bool>({ false, false, true, false }), 0.8);
	FH_cond.set_value(vector<bool>({ false, true, false, false }), 0.6);
	FH_cond.set_value(vector<bool>({ false, false, false, true }), 0.5);
	FH_cond.set_value(vector<bool>({ false, false, false, false }), 1.0);

	Factor<bool> FS_cond = Factor<bool>(FS_variables);
	FS_cond.set_value(vector<bool>({ true }), 0.05);
	FS_cond.set_value(vector<bool>({ false }), 0.95);

	Factor<bool> FB_cond = Factor<bool>(FB_variables);
	FB_cond.set_value(vector<bool>({ true, true }), 0.6);
	FB_cond.set_value(vector<bool>({ true, false }), 0.1);
	FB_cond.set_value(vector<bool>({ false, true }), 0.4);
	FB_cond.set_value(vector<bool>({ false, false }), 0.9);

	//Setup variable elimination variables
	vector<Factor<bool>*> factor_list = vector<Factor<bool>*>({ &FM_cond, &NA_cond, &NDG_cond, &FS_cond, &FB_cond, &FH_cond });

	Variable<bool>* query_variable = FS;

	vector<Variable<bool>*> elimination_ordering = vector<Variable<bool>*>({ FB, NA, FM, NDG, FH });

	vector<Variable<bool>*> variables_with_evidence = vector<Variable<bool>*>();

	vector<bool> evidence_list = vector<bool>();

	Factor<bool>* result = Factor<bool>::inference(factor_list, query_variable, elimination_ordering, variables_with_evidence, evidence_list);
	
	//Print the factor
	result->print_table();

	for (unsigned int i = 0; i < all_variables.size(); ++i) {
		delete all_variables.at(i);
	}

	delete result;

	return 0;
}