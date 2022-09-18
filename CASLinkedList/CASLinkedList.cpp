#include "CASLinkedList.h"

Node* CASLinkedList::Search(int key, Node* curNode)
{
	RetSearchFrom ret = SearchFrom(key, curNode);
	if (ret.current->_key == key)
		return ret.current;

	return nullptr;
}

bool CASLinkedList::Insert(int key, Node* head, void* value)
{
	RetSearchFrom retSearchFrom = SearchFrom(key, head);
	Node* prevNode = retSearchFrom.current;
	Node* nextNode = retSearchFrom.next;
	Node* prevSucc;

	if (GetNode(prevNode)->_key == key) {
		return false;
	}

	Node* newNode = new Node();
	newNode->_key = key;
	newNode->_value = value;

	while (true) {
		prevSucc = GetNode(prevNode)->_next;
		if (IsFlag(prevSucc)) {
			HelpFlagged(prevNode, GetNode(prevSucc)->_next);
		}
		else {
			newNode->_next = nextNode;
			Node* next = ConstructArgs(nextNode, 0, 0);

			if (std::atomic_compare_exchange_strong(&GetNode(prevNode)->_next, &next, newNode))
				return true;
			else {
				if (IsFlag(next)) {
					HelpFlagged(prevNode, next->_next);
				}

				while (IsMark(prevNode))
					prevNode = GetNode(prevNode)->_backLink;
			}
		}

		retSearchFrom = SearchFrom(key, prevNode);
		prevNode = retSearchFrom.current;
		nextNode = retSearchFrom.next;
		if (GetNode(retSearchFrom.current)->_key == key) {
			delete newNode;
			return false;
		}
	}

	return true;
}

bool CASLinkedList::Remove(int key, Node* head)
{
	RetSearchFrom ret = SearchFrom(key, head);
	Node* prev = ret.current;
	Node* delNode = ret.next;

	if (GetNode(delNode)->_key != key)
		return false;

	RetTryFlag tf = TryFlag(prev, delNode);
	prev = tf.node;
	if (tf.delNode != nullptr)
		delNode = tf.delNode;
	if (prev != nullptr)
		HelpFlagged(prev, delNode);
	if (tf.result == 0)
		return false;

	return true;
}

void CASLinkedList::PrintList()
{
	Node* curNode = _head->_next;
	while (GetNode(curNode) != _tail) {
		std::printf("%d\t", GetNode(curNode)->_key);
		curNode = curNode->_next;
	}
	std::printf("\n");
}

void CASLinkedList::Destroy(Node* node)
{
	Node* next;
	Node* curNode = GetNode(node);

	while (GetNode(curNode) != nullptr) {
		next = GetNode(curNode)->_next;
		delete curNode;
		curNode = next;
	}
}

void CASLinkedList::HelpFlagged(Node* prevNode, Node* delNode)
{
	GetNode(delNode)->_backLink = prevNode;
	if (IsMark(GetNode(delNode)->_next) == false) {
		TryMark(delNode);
	}
	HelpMarked(prevNode, delNode);
}

Node* CASLinkedList::ConstructArgs(Node* node, int mark, int flag)
{
	Node* temp = GetNode(node);
	if (mark == 1)
		temp = SetMark(node);
	if (flag == 1)
		temp = SetFlag(node);

	return temp;
}

void CASLinkedList::HelpMarked(Node* prevNode, Node* delNode)
{
	Node* next = GetNode(delNode)->_next;

	Node* del = ConstructArgs(delNode, 0, 1);
	Node* nextNode = ConstructArgs(next, 0, 0);
	
	std::atomic_compare_exchange_strong(&GetNode(prevNode)->_next, &del, nextNode);

	delete GetNode(delNode);
}

RetSearchFrom CASLinkedList::SearchFrom(int key, Node* curNode)
{
	RetSearchFrom ret;

	Node* next = curNode->_next;
	while (GetNode(next)->_key <= key) {
		while (IsMark(next) == 1 && (IsMark(curNode) == 0) || GetNode(curNode)->_next != GetNode(next)) {
			if (GetNode(curNode)->_next == GetNode(next)) {
				HelpMarked(curNode, next);
			}
			next = GetNode(curNode)->_next;
		}

		if (GetNode(next)->_key <= key) {
			curNode = next;
			next = GetNode(curNode)->_next;
		}
	}
	ret.current = curNode;
	ret.next = next;

	return ret;
}

void CASLinkedList::TryMark(Node* delNode)
{
	Node* next;

	do {
		next = GetNode(delNode)->_next;

		Node* nextNode = ConstructArgs(next, 0, 0);
		Node* nextNodeMarked = ConstructArgs(next, 1, 0);
		
		std::atomic_compare_exchange_strong(&GetNode(delNode)->_next, &nextNode, nextNodeMarked);

		if (IsMark(nextNode) == false && IsFlag(nextNode) == true)
			HelpFlagged(delNode, nextNode);
	} while (IsMark(GetNode(delNode)->_next) != true);
}

RetTryFlag CASLinkedList::TryFlag(Node* prevNode, Node* target)
{
	RetTryFlag ret;

	while (true) {
		if (GetNode(prevNode)->_next == target && !IsMark(GetNode(prevNode)->_next) && IsFlag(GetNode(prevNode)->_next)) {
			ret.node = prevNode;
			ret.result = 0;
			ret.delNode = nullptr;
			return ret;
		}

		Node* targetNode = ConstructArgs(target, 0, 0);
		Node* targetNodeFlag = ConstructArgs(target, 0, 1);
		std::atomic_compare_exchange_strong(&GetNode(prevNode)->_next, &targetNode, targetNodeFlag);

		if (target == targetNode && !IsMark(targetNode) && !IsFlag(targetNode)) {
			ret.node = prevNode;
			ret.result = 1;
			ret.delNode = nullptr;
			return ret;
		}
		if (GetNode(prevNode)->_next == target && !IsMark(GetNode(prevNode)->_next) && IsFlag(GetNode(prevNode)->_next)) {
			ret.node = prevNode;
			ret.result = 0;
			ret.delNode = nullptr;
			return ret;
		}
		
		while (IsMark(GetNode(prevNode)->_next))
			prevNode = GetNode(prevNode)->_backLink;

		RetSearchFrom retSearchFrom = SearchFrom((GetNode(target)->_key), prevNode);
		ret.node = retSearchFrom.current;
		ret.prevNode = retSearchFrom.current;
		ret.delNode = retSearchFrom.next;
		prevNode = retSearchFrom.current;
		if (retSearchFrom.next != target) {
			ret.node = nullptr;
			ret.result = 0;
			return ret;
		}
	}
	return ret;
}

Node* CASLinkedList::SetMark(Node* node)
{
	return ((Node*)((uintptr_t)node | 2));
}

Node* CASLinkedList::SetFlag(Node* node)
{
	return ((Node*)((uintptr_t)node | 1));
}

bool CASLinkedList::IsMark(Node* node)
{
	return (((uintptr_t)node & 2) == 2) ? true : false;
}

bool CASLinkedList::IsFlag(Node* node)
{
	return (((uintptr_t)node & 1) == 1) ? true : false;
}

Node* CASLinkedList::GetNode(Node* node)
{
	return ((Node*)((uintptr_t)node & -4));
}
