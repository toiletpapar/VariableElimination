#include <vector>
#include <map>

using namespace std;

template <class K, class T>
class Node {
public:
	Node(T data, Node<K, T>* parent);

	Node<K, T>* add_neighbour(K transition, Node<K, T>* node);
	Node<K, T>* get_neighbour(K transition);
	Node<K, T>* set_neighbour(K transition, Node<K, T>* node);
	Node<K, T>* remove_neighbour(K transition);
	map<K, Node<K, T>*> &get_neighbours();
	bool is_leaf();
	
	T &get_data();
	Node<K, T>* set_data(T data);

	Node<K, T>* get_parent();
	Node<K, T>* set_parent(Node<K, T>* parent);
private:
	Node<K, T>* parent;
	map<K, Node<K, T>*> neighbours;
	T data;
};

//Implementation
template <typename K, typename T>
Node<K, T>::Node(T data, Node<K, T>* parent) : data(data), parent(parent), neighbours(map<K, Node<K, T>*>()) {

}

template <typename K, typename T>
Node<K, T>* Node<K, T>::add_neighbour(K transition, Node<K, T>* node) {
	this->neighbours.insert(pair<K, Node<K, T>*>(transition, node));
	return this;
}

template<class K, class T>
Node<K, T>* Node<K, T>::get_neighbour(K transition)
{
	return this->get_neighbours().find(transition)->second;
}

template<class K, class T>
Node<K, T>* Node<K, T>::set_neighbour(K transition, Node<K, T>* node)
{
	this->get_neighbours().at(transition) = node;
	return this;
}

template<class K, class T>
Node<K, T>* Node<K, T>::remove_neighbour(K transition)
{
	this->get_neighbours().erase(transition);
	return this;
}

template <typename K, typename T>
map<K, Node<K, T>*> &Node<K, T>::get_neighbours() {
	return this->neighbours;
}

template<class K, class T>
bool Node<K, T>::is_leaf()
{
	return this->get_neighbours().empty();
}

template <typename K, typename T>
T &Node<K, T>::get_data() {
	return this->data;
}

template <typename K, typename T>
Node<K, T>* Node<K, T>::set_data(T data) {
	this->data = data;
	return this;
}

template<class K, class T>
Node<K, T>* Node<K, T>::get_parent()
{
	return this->parent;
}

template<class K, class T>
Node<K, T>* Node<K, T>::set_parent(Node<K, T>* parent)
{
	this->parent = parent;
	return this;
}
