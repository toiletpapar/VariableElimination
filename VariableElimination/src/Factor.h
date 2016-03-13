#include <vector>
#include <string>

#include <iostream>

#include "Graph.h"

using namespace std;

/*
 *	Variable
 */
//Random variable with discrete domain
template <class T>
class Variable {
public:
	Variable(string name, vector<T> domain);

	vector<T> &get_domain();
	string get_name();
private:
	vector<T> domain;
	string name;
};

template <typename T>
Variable<T>::Variable(string name, vector<T> domain) : name(name), domain(domain) {

}

template <typename T>
vector<T> &Variable<T>::get_domain() {
	return this->domain;
}

template <typename T>
string Variable<T>::get_name() {
	return this->name;
}

/*
 *	Factor
 */
//A function over a set of random variables
template <class T>
class Factor {
public:
	static Factor<T>& normalize(Factor<T> &f);
	static Factor<T>* multiply(Factor<T> &f1, Factor<T> &f2);
	static Factor<T>& restrict(Factor<T> &f, Variable<T>* variable, T value);
	static Factor<T>* sumout(Factor<T> &f, Variable<T>* variable);

	static Factor<T>* inference(vector<Factor<T>*> factor_list, Variable<T>* query_variable, vector<Variable<T>*> ordered_list_of_hidden_variables, vector<Variable<T>*> variables_with_evidence, vector<T> evidence_list);

	Factor(vector<Variable<T>*> variables);
	~Factor();

	vector<Variable<T>*> &get_variables();
	Node<T, double>* get_nary();
	Factor<T>* set_nary(Node<T, double>* nary);

	//Given some instantiation of the random variables (in the order that they were passed), get/set it's value
	double get_value(vector<T> instantiation);
	Factor<T>* set_value(vector<T> instantiation, double value);

	//Debugging
	void print_table();
private:
	vector<Variable<T>*> variables;
	Node<T, double>* nary;	//We represent the multidimensional array as a tree where the leaf nodes contain the value and the path is the instantiation of the random variables
};

template <typename K>
void construct_nary(Node<K, double>* current_node, typename vector<Variable<K>*>::iterator it, typename vector<Variable<K>*>::iterator end) {
	//Construct the nary
	if (it != end) {
		vector<K> current_domain = (*it)->get_domain();
		advance(it, 1);
		for (unsigned int i = 0; i < current_domain.size(); ++i) {
			Node<K, double>* new_node = new Node<K, double>(-1, current_node);
			current_node->add_neighbour(current_domain.at(i), new_node);
			construct_nary(new_node, it, end);
		}
	}
}

template <typename T>
Factor<T>::Factor(vector<Variable<T>*> variables) : variables(variables) {
	this->nary = new Node<T, double>(-1, NULL);
	
	construct_nary(this->nary, variables.begin(), variables.end());
}

template <typename T>
void delete_tree(Node<T, double>* tree) {
	for (map<T, Node<T, double>*>::iterator it = tree->get_neighbours().begin(); it != tree->get_neighbours().end(); ++it) {
		delete_tree(it->second);
	}
	delete tree;
}

template <typename T>
Factor<T>::~Factor() {
	delete_tree(this->nary);
}

template<class T>
vector<Variable<T>*> &Factor<T>::get_variables()
{
	return this->variables;
}

template<class T>
inline Node<T, double>* Factor<T>::get_nary()
{
	return this->nary;
}

template<class T>
Factor<T>* Factor<T>::set_nary(Node<T, double>* nary)
{
	this->nary = nary;
	return this;
}

template <typename T>
double Factor<T>::get_value(vector<T> instantiation) {
	//Find the appropriate leaf node
	Node<T, double>* current_node = this->nary;
	for (unsigned int i = 0; i < instantiation.size(); i++) {
		current_node = current_node->get_neighbour(instantiation.at(i));
	}

	return current_node->get_data();
}

template <typename T>
Factor<T>* Factor<T>::set_value(vector<T> instantiation, double value) {
	//Find the appropriate leaf node
	Node<T, double>* current_node = this->nary;
	for (unsigned int i = 0; i < instantiation.size(); i++) {
		current_node = current_node->get_neighbour(instantiation.at(i));
	}

	current_node->set_data(value);
	return this;
}

template <class T>
void rprint_table(Node<T, double>* current_node, typename vector<Variable<T>*>::iterator it, typename vector<Variable<T>*>::iterator end, vector<pair<string, T> > path) {
	if (current_node->is_leaf()) {
		for (unsigned int i = 0; i < path.size(); ++i) {
			cout << path.at(i).first << "=" << path.at(i).second << " ";
		}
		cout << ": " << current_node->get_data() << endl;
	}
	else {
		for (map<T, Node<T, double>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
			vector<pair<string, T> > new_path = path;
			new_path.push_back(pair<string, T>((*it)->get_name(), cit->first));
			rprint_table(cit->second, next(it, 1), end, new_path);
		}
	}
}

template <class T>
void Factor<T>::print_table()
{
	rprint_table(this->get_nary(), this->get_variables().begin(), this->get_variables().end(), vector<pair<string,T> >());
}

template <typename T>
vector<Variable<T>*> remove_variable(vector<Variable<T>*> variables, Variable<T>* variable) {
	for (vector<Variable<T>*>::iterator it = variables.begin(); it != variables.end(); ++it) {
		if ((*it)->get_name() == variable->get_name()) {
			variables.erase(it);
			return variables;
		}
	}
}

template <typename K>
void rrestrict(Node<K, double>* current_node, typename vector<Variable<K>*>::iterator it, typename vector<Variable<K>*>::iterator end, Variable<K>* restricted_variable, K restricted_value) {
	if (it == end) {
		//Variable not found
	}
	else if (advance(it, 1), it != end && (*it)->get_name() == restricted_variable->get_name()) {	//Forward lookahead 1 level is necessary so that children don't update parent multiple times
		//Variable found, restrict
		for (map<K, Node<K, double>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
			//Retrieve child and grandchild
			Node<K, double>* old_child = cit->second;
			Node<K, double>* new_child = old_child->get_neighbour(restricted_value);

			//Update child (of current node)
			current_node->set_neighbour(cit->first, new_child);

			//Update parent (of grandchild)
			new_child->set_parent(current_node);

			//Delete (child node and all unused children)
			old_child->remove_neighbour(restricted_value);
			delete_tree(old_child);
		}
	}
	else {
		//Continue deeper into tree
		for (map<K, Node<K, double>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
			rrestrict(cit->second, it, end, restricted_variable, restricted_value);
		}
	}
}

//Node: Modifies this factor
template <typename T>
Factor<T>& Factor<T>::restrict(Factor<T> &f, Variable<T>* variable, T value)
{
	if (!f.get_variables().empty() && f.get_variables().at(0)->get_name() == variable->get_name()) {
		//Handle case where root node is restricted variable and update nary accordingly
		Node<T, double>* old_nary = f.get_nary();
		Node<T, double>* new_nary = old_nary->get_neighbour(value);

		f.set_nary(new_nary);
		old_nary->remove_neighbour(value);
		delete_tree(old_nary);
	}
	else {
		rrestrict(f.get_nary(), f.get_variables().begin(), f.get_variables().end(), variable, value);
	}
	
	for (vector<Variable<T>*>::iterator it = f.get_variables().begin(); it != f.get_variables().end(); ++it) {
		if ((*it)->get_name() == variable->get_name()) {
			f.get_variables().erase(it);
			break;
		}
	}
	return f;
}

template <class T>
struct PartialInstantiations {
	vector<vector<T>> instantiations;
	vector<int> index_of_uninstantiated_variables;
};

//Returns the index of the variable in variables or -1 if not found
template <class T>
int variable_in_variables(Variable<T>* variable, vector<Variable<T>*> variables) {
	for (unsigned int i = 0; i < variables.size(); ++i) {
		if (variables.at(i)->get_name() == variable->get_name()) {
			return i;
		}
	}

	return -1;
}

template <typename T>
PartialInstantiations<T> get_instantiation(vector<Variable<T>*> variables) {
	return get_instantiation(variables, vector<Variable<T>*>());
}

//Returns a vector of paritally instantiated variables where uninstantiate is the variables to leave out of instantiation
template<typename T>
PartialInstantiations<T> get_instantiation(vector<Variable<T>*> variables, vector<Variable<T>*> uninstantiate) {
	PartialInstantiations<T> partial_instantiations;

	if (!variables.empty()) {
		//Initialize partial_instantiations
		if (variable_in_variables(variables.at(0), uninstantiate) != -1) {
			partial_instantiations.instantiations.push_back(vector<T>());
			partial_instantiations.instantiations.at(0).resize(1);

			partial_instantiations.index_of_uninstantiated_variables.push_back(0);
		}
		else {
			for (unsigned int i = 0; i < variables.at(0)->get_domain().size(); ++i) {
				vector<T> instantiation;
				instantiation.push_back(variables.at(0)->get_domain().at(i));

				partial_instantiations.instantiations.push_back(instantiation);
			}
		}
		
		//Append to partial_instantiations
		for (unsigned int i = 1; i < variables.size(); ++i) {
			Variable<T>* variable = variables.at(i);

			//Keep a copy of the instantiations of the previous variable
			vector<vector<T>> old_instantiations = vector<vector<T>>(partial_instantiations.instantiations);

			//Clear the previous variable's partial_instantiations so we may add the new instantiations
			partial_instantiations.instantiations.clear();

			if (variable_in_variables(variable, uninstantiate) != -1) {
				//Add to previous instantiations a placeholder value
				vector<vector<T>> new_instantiations = vector<vector<T>>(old_instantiations);

				for (unsigned int k = 0; k < new_instantiations.size(); ++k) {
					new_instantiations.at(k).resize(new_instantiations.at(k).size() + 1);
				}

				//Add the new instantiations to the list of instantiations
				partial_instantiations.instantiations.insert(partial_instantiations.instantiations.end(), new_instantiations.begin(), new_instantiations.end());

				partial_instantiations.index_of_uninstantiated_variables.push_back(i);
			}
			else {
				for (unsigned int j = 0; j < variable->get_domain().size(); ++j) {
					//Copy previous variable's instantiations
					vector<vector<T>> new_instantiations = vector<vector<T>>(old_instantiations);

					//Append the current variable's instantiation to the previous variable's instantiations
					for (unsigned int k = 0; k < new_instantiations.size(); ++k) {
						new_instantiations.at(k).push_back(variable->get_domain().at(j));
					}

					//Add the new instantiations to the list of instantiations
					partial_instantiations.instantiations.insert(partial_instantiations.instantiations.end(), new_instantiations.begin(), new_instantiations.end());
				}
			}
		}
	}
	
	return partial_instantiations;
}

template<typename T>
Factor<T>* Factor<T>::sumout(Factor<T>& f, Variable<T>* variable)
{
	vector<Variable<T>*> new_variables = vector<Variable<T>*>(f.get_variables());	//Copy variables

	//Remove variable to be summed out
	for (vector<Variable<T>*>::iterator it = new_variables.begin(); it != new_variables.end(); ++it) {
		if ((*it)->get_name() == variable->get_name()) {
			new_variables.erase(it);
			break;
		}
	}

	//Create new factor
	Factor<T>* new_factor = new Factor<T>(new_variables);

	//Get all instantiations of the factor to sumout
	vector<Variable<T>*> uninstantiate;
	uninstantiate.push_back(variable);
	PartialInstantiations<T> partial_instantiations = get_instantiation(f.get_variables(), uninstantiate);

	for (vector<vector<T>>::iterator it = partial_instantiations.instantiations.begin(); it != partial_instantiations.instantiations.end(); ++it) {
		//Get the instantiation without the uninstantiated variable
		vector<T> instantiation = vector<T>(*it);
		instantiation.erase(next(instantiation.begin(), partial_instantiations.index_of_uninstantiated_variables.at(0)));

		double sum = 0;

		for (vector<T>::iterator dit = variable->get_domain().begin(); dit != variable->get_domain().end(); ++dit) {
			vector<T> filled_instantiation = vector<T>(*it);
			filled_instantiation.at(partial_instantiations.index_of_uninstantiated_variables.at(0)) = *dit;
			sum += f.get_value(filled_instantiation);
		}

		new_factor->set_value(instantiation, sum);
	}

	return new_factor;
}

template<class T>
Factor<T>* Factor<T>::multiply(Factor<T> &f1, Factor<T> &f2)
{
	//Create variable list with the variables from f1 and f2
	vector<Variable<T>*> new_variables = vector<Variable<T>*>(f2.get_variables());

	//Get the list of common variables and add uncommon variables to new variable list
	for (vector<Variable<T>*>::iterator it1 = f1.get_variables().begin(); it1 != f1.get_variables().end(); ++it1) {
		if (variable_in_variables(*it1, f2.get_variables()) == -1) {
			new_variables.push_back(*it1);
		}
	}

	//Create the factor with the new variable list
	Factor<T>* new_factor = new Factor<T>(new_variables);
	
	//Get all the instantiations of new_variables
	PartialInstantiations<T> new_instantiations = get_instantiation(new_variables);

	for (vector<vector<T>>::iterator it = new_instantiations.instantiations.begin(); it != new_instantiations.instantiations.end(); ++it) {
		vector<T> partial_instantiation = *it;

		//Get the product
		double factor1, factor2;
		vector<T> factor_instantiation = vector<T>(f1.get_variables().size());

		for (unsigned int i = 0; i < new_variables.size(); ++i) {
			int index = variable_in_variables(new_variables.at(i), f1.get_variables());
			if (index != -1) {
				factor_instantiation.at(index) = partial_instantiation.at(i);
			}
		}

		factor1 = f1.get_value(factor_instantiation);

		factor_instantiation.clear();
		factor_instantiation.insert(factor_instantiation.begin(), partial_instantiation.begin(), next(partial_instantiation.begin(), f2.get_variables().size()));

		factor2 = f2.get_value(factor_instantiation);

		new_factor->set_value(partial_instantiation, factor1 * factor2);
	}

	return new_factor;
}

//Note: modifies factor given
template<class T>
Factor<T>& Factor<T>::normalize(Factor<T>& f)
{
	PartialInstantiations<T> partial_instantiations = get_instantiation(f.get_variables());

	double sum = 0;

	for (vector<vector<T>>::iterator instantiation = partial_instantiations.instantiations.begin(); instantiation != partial_instantiations.instantiations.end(); ++instantiation) {
		sum += f.get_value(*instantiation);
	}

	for (vector<vector<T>>::iterator instantiation = partial_instantiations.instantiations.begin(); instantiation != partial_instantiations.instantiations.end(); ++instantiation) {
		double normalized_value = f.get_value(*instantiation) / sum;
		f.set_value(*instantiation, normalized_value);
	}

	return f;
}

template <typename T>
Factor<T>* Factor<T>::inference(vector<Factor<T>*> factor_list, Variable<T>* query_variable, vector<Variable<T>*> ordered_list_of_hidden_variables, vector<Variable<T>*> variables_with_evidence, vector<T> evidence_list) {
	vector<Factor<T>*> factors_to_delete;
	vector<Factor<T>*> factors_to_keep;
	
	//Restrict factors in factor_list according to the evidence in evidence_list
	for (vector<Factor<T>*>::iterator factor = factor_list.begin(); factor != factor_list.end(); ++factor) {
		vector<Variable<T>*> factor_variables = (*factor)->get_variables();
		bool restricted_variable = false;
		bool found_variable_to_restrict = false;

		for (unsigned int i = 0; i < variables_with_evidence.size(); ++i) {
			Variable<T>* variable = variables_with_evidence.at(i);

			if (variable_in_variables(variable, factor_variables) != -1) {
				//Don't bother restricting if the only variable in the factor is the variable to restrict
				found_variable_to_restrict = true;
				if (factor_variables.size() > 1) {
					Factor<T>::restrict(**factor, variable, evidence_list.at(i));
					restricted_variable = true;
				}
			}
		}

		if (restricted_variable || !found_variable_to_restrict) {
			factors_to_keep.push_back(*factor);
		}
	}

	factor_list = factors_to_keep;

	//Sum out the hidden variables from the product of the factors in factor_list
	for (vector<Variable<T>*>::iterator variable = ordered_list_of_hidden_variables.begin(); variable != ordered_list_of_hidden_variables.end(); ++variable) {
		cout << "Eliminate: " << (*variable)->get_name() << endl;
		//for each variable to eliminate
		bool first_factor = true;
		bool second_factor = true;
		Factor<T>* product = factor_list[0];	//Initialize product to some random factor
		vector<Factor<T>*> retained_factors;
		
		for (unsigned int i = 0; i < factor_list.size(); ++i) {
			Factor<T>* factor = factor_list.at(i);
			//test if variable is in the factor
			if (variable_in_variables(*variable, factor->get_variables()) != -1) {
				//if it is then update the product of the factors
				if (first_factor) {
					//Initialize product to the proper factor
					first_factor = false;
					product = factor;
				}
				else if (second_factor) {
					second_factor = false;
					product = Factor<T>::multiply(*product, *factor);
				}
				else {
					Factor<T>* old_product = product;
					product = Factor<T>::multiply(*product, *factor);

					delete old_product;
				}
			}
			else {
				retained_factors.push_back(factor);
			}
		}

		//finally, sumout the variable to eliminate from the product
		Factor<T>* new_factor = Factor<T>::sumout(*product, *variable);
		if (!second_factor) {
			delete product;
		}

		cout << "Computed factor:\n";
		new_factor->print_table();
		cout << "==============" << endl;

		//add the new factor into the factor list and remove the eliminated factors
		retained_factors.push_back(new_factor);
		factor_list = retained_factors;

		//don't lose the references to new factors
		factors_to_delete.push_back(new_factor);
	}
	
	//What's left in the factor_list is the query_variable if all other variables were present in the elimination order
	//Get the product for the query_variable
	Factor<T>* product = factor_list[0];
	bool first_factor = true;
	bool second_factor = true;
	for (unsigned int i = 0; i < factor_list.size(); ++i) {
		Factor<T>* factor = factor_list.at(i);
		//test if variable is in the factor
		if (variable_in_variables(query_variable, factor->get_variables()) != -1) {
			//if it is then update the product of the factors
			if (first_factor) {
				first_factor = false;
				product = factor;
			}
			else if (second_factor) {
				second_factor = false;
				product = Factor<T>::multiply(*product, *factor);
			}
			else {
				Factor<T>* old_product = product;
				product = Factor<T>::multiply(*product, *factor);

				delete old_product;
			}
		}
	}

	//Normalize the resulting factor
	Factor<T>::normalize(*product);

	//Clean up around the house
	for (unsigned int i = 0; i < factors_to_delete.size(); ++i) {
		delete factors_to_delete.at(i);
	}

	return product;
}