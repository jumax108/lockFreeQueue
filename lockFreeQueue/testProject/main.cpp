#include <stdio.h>
#include <thread>

#include "../headers/lockFreeQueue.h"

constexpr int THREAD_NUM = 2;
constexpr int NODE_NUM_EACH_THREAD = 1;

constexpr int TOTAL_NODE_NUM = THREAD_NUM * NODE_NUM_EACH_THREAD;

CDump dump;

struct stNode{

	stNode(){
		num = 0;
	}

	int num;

};

CLockFreeQueue<stNode*>* lockFreeQueue = new CLockFreeQueue<stNode*>();
CLockFreeQueue<stNode*>* queueForDebug = nullptr;

unsigned __stdcall logicTestFunc(void* args);

int main(){


	for(int nodeCnt = 0; nodeCnt < TOTAL_NODE_NUM; ++nodeCnt){
		lockFreeQueue->push(new stNode);
	}

	for(int threadCnt = 0; threadCnt < THREAD_NUM; ++threadCnt){
		_beginthreadex(nullptr, 0, logicTestFunc, nullptr, 0, nullptr);
	}

	for(;;){
		printf("queue Size: %d\n", lockFreeQueue->getSize());
		Sleep(999);
	}

	return 0;

}

unsigned __stdcall logicTestFunc(void* args){

	stNode* nodes[NODE_NUM_EACH_THREAD];

	for(;;){
		
		ZeroMemory(nodes, sizeof(nodes));

		///////////////////////////////////////////////////
		// 1. stack���� node�� pop
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			if(lockFreeQueue->pop(&nodes[nodeCnt]) == false){
				if(queueForDebug == nullptr)
					queueForDebug = lockFreeQueue;
				lockFreeQueue = nullptr;
				CDump::crash();
			}
		}
		///////////////////////////////////////////////////
		
		///////////////////////////////////////////////////
		// 2. node�� �����Ͱ� 0���� Ȯ��
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			if(nodes[nodeCnt]->num != 0){
				if(queueForDebug == nullptr)
					queueForDebug = lockFreeQueue;
				lockFreeQueue = nullptr;
				CDump::crash();
			}
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 3. ������ 1 ����
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			InterlockedIncrement((LONG*)&nodes[nodeCnt]->num);
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 4. node�� �����Ͱ� 1���� Ȯ��
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			if(nodes[nodeCnt]->num != 1){
				if(queueForDebug == nullptr)
					queueForDebug = lockFreeQueue;
				lockFreeQueue = nullptr;
				CDump::crash();
			}
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 5. ������ 1 ����
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			InterlockedDecrement((LONG*)&nodes[nodeCnt]->num);
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 6. node�� �����Ͱ� 0���� Ȯ��
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			if(nodes[nodeCnt]->num != 0){
				if(queueForDebug == nullptr)
					queueForDebug = lockFreeQueue;
				lockFreeQueue = nullptr;
				CDump::crash();
			}
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 7. stack�� ����
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			lockFreeQueue->push(nodes[nodeCnt]);
		}
		///////////////////////////////////////////////////
	}

	return 0;
}