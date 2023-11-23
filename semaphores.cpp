#include<iostream>
#include<thread>
#include<cstdlib>
#include<unistd.h>
#include<queue>

using namespace std;

class Semaphore {
	public:
		Semaphore(int mc_value, int c_value):
			max_count(mc_value), count(c_value) {}

		void increaseCount(){
			count++;
		}

		void decreaseCount(){
			count--;
		}

		int getMaxCount(){
			return max_count;
		}

		int getCount(){
			return count;
		}

	private:
		const int max_count; // max value of the semaphore
		int count; // inital value of the semaphore
};

class Producer {
	public:
		Producer* next; 
		Producer(Semaphore* prod_sem, Semaphore* cons_sem, Semaphore* crit_sem): producer_semaphore(prod_sem), consumer_semaphore(cons_sem), critical_section_semaphore(crit_sem){
			this->produceJob();
			next = NULL;
		}
		void produceJob(){
			job = rand()%10 + 1;
		}
		void placeJobInQueue(){
			critical_section_semaphore->decreaseCount();
			//int index=0;
			//while (queue[index]!=0){
			//	index++;
			//}
			//queue[index] = job;
			critical_section_semaphore->increaseCount();
			producer_semaphore->decreaseCount();
			consumer_semaphore->increaseCount();
		}
	private:
		int job;
		Semaphore* producer_semaphore;
		Semaphore* consumer_semaphore;
		Semaphore* critical_section_semaphore;
		int queue[10];
};

class Consumer {
	public:
		Consumer* next; 
		Consumer(Semaphore* prod_sem, Semaphore* cons_sem, Semaphore* crit_sem): producer_semaphore(prod_sem), consumer_semaphore(cons_sem), critical_section_semaphore(crit_sem){
			next = NULL;
		}
		void retrieveJobFromQueue(){
			critical_section_semaphore->decreaseCount();
			//int index=0;
			//while (queue[index]==0){
			//	index++;
			//}
			//job = queue[index]; 
			critical_section_semaphore->increaseCount();
			producer_semaphore->increaseCount();
			consumer_semaphore->decreaseCount();
		}

		void executeJob(){
			sleep(job);
		}
	private:
		int job;
		Semaphore* producer_semaphore;
		Semaphore* consumer_semaphore;
		Semaphore* critical_section_semaphore;
};

int main(){
	Semaphore producer_semaphore(10, 10);
	Semaphore consumer_semaphore(10, 0);
	Semaphore critical_section_semaphore(1, 1);
	//int queue[10] = {0};
	queue<Producer> producer_queue;
	queue<Consumer> consumer_queue;
	for (int i=0; i<10; i++){
		producer_queue.push(Producer(&producer_semaphore, &consumer_semaphore, &critical_section_semaphore));
		consumer_queue.push(Consumer(&producer_semaphore, &consumer_semaphore, &critical_section_semaphore));
	}

	for (; !producer_queue.empty(); producer_queue.pop())
		cout << "Hey ";

	return 0;
}
