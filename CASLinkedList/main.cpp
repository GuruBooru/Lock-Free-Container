#include <iostream>
#include <thread>
#include "CASLinkedList.h"

int main()
{
	CASLinkedList* list = new CASLinkedList();

	int threadCount = 8;
	std::thread* workers = new std::thread[threadCount];

	return 0;
}
