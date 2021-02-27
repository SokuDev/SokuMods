#pragma once
namespace list {

template<typename Data> struct Node {
	Data data;
	Node *next;
};

template<typename Data> static Node<Data> *cons(Data head, Node<Data> *tail) {
	auto node = new Node<Data>;
	node->data = head;
	node->next = tail;
	return node;
}

template<typename Data> static void free(Node<Data> *s) {
	Node<Data> *next;

	for (Node<Data> *e = s; e; e = next) {
		next = e->next;
		delete e;
	}
}

template<typename Data> struct List {
	Node<Data> *_head;

public:
	List(): _head(NULL) {}

	~List() {
		list::free(_head);
	}

	void clear() {
		list::free(_head);
		_head = NULL;
	}

	Node<Data> *head() {
		return _head;
	}

	void push(Data data) {
		_head = cons(data, _head);
	}

	List &operator=(Node<Data> *node) {
		if (_head) {
			list::free(_head);
		}

		_head = node;
		return *this;
	}
};

} // namespace list