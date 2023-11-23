#include<iostream>
#include<thread>
#include<cstdlib>
#include<unistd.h>

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
}

class Producer {
	public:
		Producer(){
			this->produceJob();
		}
		void produceJob(){
			job = rand()%10 + 1;
		}
		void placeJobInQueue(){
			critical_section_semaphore.decreaseCount();
			int index=0;
			while (queue[index]!=0){
				index++;
			}
			queue[index] = job;
			critical_section_semaphore.increaseCount();
			producer_semaphore.decreaseCount();
			consumer_semaphore.increaseCount();
		}
	private:
		int job;
		Semaphore* producer_semaphore;
		Semaphore* consumer_semaphore;
		Semaphore* critical_section_semaphore;
		int queue[10];

}

class Consumer {
	public:
		Consumer(){};
		void retrieveJobFromQueue(int queue_index){
			critical_section_semaphore.decreaseCount();
			int index=0;
			while (queue[index]==0){
				index++;
			}
			job = queue[index]; 
			critical_section_semaphore.increaseCount();
			producer_semaphore.increaseCount();
			consumer_semaphore.decreaseCount();
		}

		void executeJob(){
			sleep(job);
		}
	private:
		int job;
	private:
		int job;
		Semaphore* producer_semaphore;
		Semaphore* consumer_semaphore;
		Semaphore* critical_section_semaphore;
		int queue[10];

}

int main(){
	Semaphore producer_semaphore(10, 10);
	Semaphore consumer_semaphore(10, 0);
	Semaphore critical_section_semaphore(1, 1);
	thread first(say_number, 1);
	thread second(say_number, 2);

	first.join();
	second.join();

	return 0;
}
