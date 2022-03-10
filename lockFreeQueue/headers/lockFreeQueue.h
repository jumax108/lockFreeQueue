#pragma once

#include <Windows.h>

#include "objectFreeListTLS/headers/objectFreeListTLS.h"

static constexpr unsigned __int64 _useCntMask	= 0xFFFFF80000000000;
static constexpr unsigned __int64 _pointerMask	= 0x000007FFFFFFFFFF;

template <typename T>
class CLockFreeQueue{

public:

	CLockFreeQueue();

	void push(T data);
	bool pop(T* data);

	unsigned __int64 getSize();

private:

	struct stNode{
		stNode(){
			_next = nullptr;
			_data = NULL;
		}
		void* _next;
		T _data;
	};
	
	void* _head;
	void* _tail;

	unsigned __int64 _size;

	unsigned __int64 _nodeChangeCnt;

	CObjectFreeListTLS<stNode> _nodeFreeList;

	struct stLogLine{

		int _logCnt = 0;
		int _code = 0;

		long _threadID = 0;
		void* _org = nullptr;
		void* _next = nullptr;
		void* _data = nullptr;

	};

	stLogLine _logLine[65536];
	int _logCnt;

};

template <typename T>
CLockFreeQueue<T>::CLockFreeQueue():
	_nodeFreeList(false, false){

	stNode* node = _nodeFreeList.allocObject();
	_head = node;
	_tail = node;

	_nodeChangeCnt = 0;
	_size = 0;

	ZeroMemory(_logLine, sizeof(_logLine));
	_logCnt = 0;
}

template <typename T>
void CLockFreeQueue<T>::push(T data){
		
	stNode* newNode = _nodeFreeList.allocObject();
	newNode->_data = data;
	newNode->_next = nullptr;

	void* newPtr;
	void* tail;
	void* tailNextPtr;

	stNode* tailNode;
		
	InterlockedAdd64((LONG64*)&_nodeChangeCnt, 0x0000080000000000);
	newPtr = (void*)(_nodeChangeCnt | (unsigned __int64)newNode);

	{
		int logCnt = InterlockedIncrement((LONG*)&_logCnt) - 1;
		stLogLine* logLine = &_logLine[(unsigned short)logCnt];

		logLine->_logCnt = logCnt;
		logLine->_code = 0x10;

		logLine->_threadID = __threadid();
		logLine->_org = _tail;
		logLine->_next = nullptr;
		logLine->_data = (void*)data;

	}

	do{
		
		// tail의 next가 null일 때까지 변경
		do{

			tail = _tail;
			tailNode = (stNode*)((unsigned __int64)tail & _pointerMask);
			tailNextPtr = tailNode->_next;

			if(tailNextPtr == nullptr){
				break;
			}
						

		}while(InterlockedCompareExchange64((LONG64*)&_tail, (LONG64)tailNextPtr, (LONG64)tail) != (LONG64)tail);


		tail = _tail;
		tailNode = (stNode*)((unsigned __int64)tail & _pointerMask);

	} while( InterlockedCompareExchange64((LONG64*)&tailNode->_next, (LONG64)newPtr, (LONG64)nullptr ) != (LONG64)nullptr );
	
	InterlockedCompareExchange64((LONG64*)&_tail, (LONG64)newPtr, (LONG64)tail);
	
	{
		int logCnt = InterlockedIncrement((LONG*)&_logCnt) - 1;
		stLogLine* logLine = &_logLine[(unsigned short)logCnt];

		logLine->_logCnt = logCnt;
		logLine->_code = 0x11;
		
		logLine->_threadID = __threadid();
		logLine->_org = tail;
		logLine->_next = tailNode->_next;
		logLine->_data = (void*)data;
	}

	InterlockedIncrement64((LONG64*)&_size);

}

template <typename T>
bool CLockFreeQueue<T>::pop(T* data){
	
	
	unsigned __int64 size = InterlockedDecrement64((LONG64*)&_size);
	if(size < 0){
		InterlockedIncrement64((LONG64*)&_size);
		return false;
	}

	void* popPtr;
	void* head;

	stNode* popNode;
	stNode* headNode;

	T popData = NULL;
	
	{
		int logCnt = InterlockedIncrement((LONG*)&_logCnt) - 1;
		stLogLine* logLine = &_logLine[(unsigned short)logCnt];

		logLine->_logCnt = logCnt;
		logLine->_code = 0x20;
		
		logLine->_threadID = __threadid();
		logLine->_org = _head;
		logLine->_next = nullptr;
		logLine->_data = nullptr;
	}

	do{
		
		head = _head;
		headNode = (stNode*)((unsigned __int64)head & _pointerMask);
			
		popPtr = headNode->_next;
		if(popPtr == nullptr){
			// haedNode 획득 이후 다른 스레드에서 head를 free 후 재할당 받아서 next를 nullptr로 초기화할 수 있음
			head = nullptr;
			continue;
		}
		
		popNode = (stNode*)((unsigned __int64)popPtr & _pointerMask);
		popData = popNode->_data;

	}while(  InterlockedCompareExchange64((LONG64*)&_head, (LONG64)popPtr, (LONG64)head) != (LONG64)head);
	
	*data = popData;

	{
		int logCnt = InterlockedIncrement((LONG*)&_logCnt) - 1;
		stLogLine* logLine = &_logLine[(unsigned short)logCnt];

		logLine->_logCnt = logCnt;
		logLine->_code = 0x21;
		
		logLine->_threadID = __threadid();
		logLine->_org = head;
		logLine->_next = popPtr;
		logLine->_data = popData;
	}
	
	_nodeFreeList.freeObject(headNode);

	return true;
}

template <typename T>
unsigned __int64 CLockFreeQueue<T>::getSize(){
	return _size;
}