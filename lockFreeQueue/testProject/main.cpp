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
		// 1. stack에서 node를 pop
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
		// 2. node의 데이터가 0인지 확인
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
		// 3. 데이터 1 증가
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			InterlockedIncrement((LONG*)&nodes[nodeCnt]->num);
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 4. node의 데이터가 1인지 확인
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
		// 5. 데이터 1 감소
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			InterlockedDecrement((LONG*)&nodes[nodeCnt]->num);
		}
		///////////////////////////////////////////////////

		///////////////////////////////////////////////////
		// 6. node의 데이터가 0인지 확인
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
		// 7. stack에 삽입
		for(int nodeCnt = 0; nodeCnt < NODE_NUM_EACH_THREAD; ++nodeCnt){
			lockFreeQueue->push(nodes[nodeCnt]);
		}
		///////////////////////////////////////////////////
	}

	return 0;
}