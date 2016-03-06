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

	vector<T> get_domain();
	string get_name();
private:
	vector<T> domain;
	string name;
};

template <typename T>
Variable<T>::Variable(string name, vector<T> domain) : name(name), domain(domain) {

}

template <typename T>
vector<T> Variable<T>::get_domain() {
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
	static Factor<T> multiply(Factor<T> f1, Factor<T> f2);
	static Factor<T>& restrict(Factor<T> &f, Variable<T>* variable, T value);
	static Factor<T>& sumout(Factor<T> &f, Variable<T>* variable);

	Factor(vector<Variable<T>*> variables);
	~Factor();

	vector<Variable<T>*> &get_variables();
	Node<T, int>* get_nary();
	Factor<T>* set_nary(Node<T, int>* nary);

	//Given some instantiation of the random variables (in the order that they were passed), get/set it's value
	int get_value(vector<T> instantiation);
	Factor<T>* set_value(vector<T> instantiation, int value);

	//Debugging
	void print_table();
private:
	vector<Variable<T>*> variables;
	Node<T, int>* nary;	//We represent the multidimensional array as a tree where the leaf nodes contain the value and the path is the instantiation of the random variables
};

template <typename K>
void construct_nary(Node<K, int>* current_node, typename vector<Variable<K>*>::iterator it, typename vector<Variable<K>*>::iterator end) {
	//Construct the nary
	if (it != end) {
		vector<K> current_domain = (*it)->get_domain();
		advance(it, 1);
		for (unsigned int i = 0; i < current_domain.size(); ++i) {
			Node<K, int>* new_node = new Node<K, int>(-1, current_node);
			current_node->add_neighbour(current_domain.at(i), new_node);
			construct_nary(new_node, it, end);
		}
	}
}

template <typename T>
Factor<T>::Factor(vector<Variable<T>*> variables) : variables(variables) {
	this->nary = new Node<T, int>(-1, NULL);
	
	construct_nary(this->nary, variables.begin(), variables.end());
}

template <typename T>
void delete_tree(Node<T, int>* tree) {
	for (map<T, Node<T, int>*>::iterator it = tree->get_neighbours().begin(); it != tree->get_neighbours().end(); ++it) {
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
inline Node<T, int>* Factor<T>::get_nary()
{
	return this->nary;
}

template<class T>
Factor<T>* Factor<T>::set_nary(Node<T, int>* nary)
{
	this->nary = nary;
	return this;
}

template <typename T>
int Factor<T>::get_value(vector<T> instantiation) {
	//Find the appropriate leaf node
	Node<T, int>* current_node = this->nary;
	for (unsigned int i = 0; i < instantiation.size(); i++) {
		current_node = current_node->get_neighbour(instantiation.at(i));
	}

	return current_node->get_data();
}

template <typename T>
Factor<T>* Factor<T>::set_value(vector<T> instantiation, int value) {
	//Find the appropriate leaf node
	Node<T, int>* current_node = this->nary;
	for (unsigned int i = 0; i < instantiation.size(); i++) {
		current_node = current_node->get_neighbour(instantiation.at(i));
	}

	current_node->set_data(value);
	return this;
}

template <class T>
void rprint_table(Node<T, int>* current_node, typename vector<Variable<T>*>::iterator it, typename vector<Variable<T>*>::iterator end, vector<pair<string, T> > path) {
	if (current_node->is_leaf()) {
		for (unsigned int i = 0; i < path.size(); ++i) {
			cout << path.at(i).first << "=" << path.at(i).second << " ";
		}
		cout << ": " << current_node->get_data() << endl;
	}
	else {
		for (map<T, Node<T, int>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
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
void rrestrict(Node<K, int>* current_node, typename vector<Variable<K>*>::iterator it, typename vector<Variable<K>*>::iterator end, Variable<K>* restricted_variable, K restricted_value) {
	if (it == end) {
		//Variable not found
	}
	else if (advance(it, 1), it != end && (*it)->get_name() == restricted_variable->get_name()) {	//Forward lookahead 1 level is necessary so that children don't update parent multiple times
		//Variable found, restrict
		for (map<K, Node<K, int>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
			//Retrieve child and grandchild
			Node<K, int>* old_child = cit->second;
			Node<K, int>* new_child = old_child->get_neighbour(restricted_value);

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
		for (map<K, Node<K, int>*>::iterator cit = current_node->get_neighbours().begin(); cit != current_node->get_neighbours().end(); ++cit) {
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
		Node<T, int>* old_nary = f.get_nary();
		Node<T, int>* new_nary = old_nary->get_neighbour(value);

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
