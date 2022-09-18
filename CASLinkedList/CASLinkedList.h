#pragma once
#include <atomic>
#include <iostream>

class Node
{
public:
	int _key;
	void* _value;

	std::atomic<Node*> _next;
	std::atomic<Node*> _backLink;
};

struct RetSearchFrom
{
	Node* current;
	Node* next;
};

struct RetTryFlag
{
	Node* node;
	int result;
	Node* prevNode;
	Node* delNode;
};

class CASLinkedList
{
public:
	CASLinkedList()
	{
		_head = new Node();
		_tail = new Node();
		_head->_next = _tail;
		_head->_key = 0;
		_head->_value = nullptr;

		_tail->_next = nullptr;
		_tail->_key = INT_MAX;
		_tail->_value = nullptr;
	}
	Node* Search(int key, Node* head);
	bool Insert(int key, Node* head, void* value);
	bool Remove(int key, Node* head);
	void PrintList();
	void Destroy(Node* node);

private:
	void HelpFlagged(Node* prevNode, Node* delNode);
	Node* ConstructArgs(Node* node, int mark, int flag);
	void HelpMarked(Node* prevNode, Node* delNode);
	RetSearchFrom SearchFrom(int key, Node* curNode);
	void TryMark(Node* delNode);
	RetTryFlag TryFlag(Node* prevNode, Node* target);

	Node* SetMark(Node* node);
	Node* SetFlag(Node* node);
	bool IsMark(Node* node);
	bool IsFlag(Node* node);
	Node* GetNode(Node* node);

private:
	Node* _head;
	Node* _tail;
};