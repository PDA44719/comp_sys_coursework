#include<iostream>
#include<queue>
#include<string>
#include<cstdlib>
#include<ctime>
#include<semaphore>
#include<chrono>
#include<thread>

using namespace std;

void parseArgument(int index, char* arguments[], int* params){
	switch (arguments[index][1]){
		case 'q': // queue size
			params[0] = stoi(arguments[index+1]);
			break;
		case 'j': // number of jobs
			params[1] = stoi(arguments[index+1]);
			break;
		case 'p': // number of producers
			params[2] = stoi(arguments[index+1]);
			break;
		case 'c': // number of consumers
			params[3] = stoi(arguments[index+1]);
			break;
	}
}

void produce_job(int* queue_ptr, int queue_size, int max_jobs, int producer_id,
				 counting_semaphore<1>& cs, counting_semaphore<100>& empty_spots, counting_semaphore<100>& jobs_in_queue){
	for (int i=0; i<max_jobs; i++){
		bool job_placed = empty_spots.try_acquire_for(chrono::seconds(10));
		if (!job_placed){
			cout << "Producer " << producer_id << " quit after a spot did not become available for 10 seconds." << endl;
			return;
		}
		int* job_value = new int;
		srand((int) time(0)); // Generate a seed
		cout << "Made it here" << endl;
		bool critical_section_entry = cs.try_acquire_for(chrono::seconds(10));
		if (!critical_section_entry){
			cout << "Producer " << producer_id << " quitting" << endl;
		}
		for (int i=0; i<queue_size; i++){
			if (queue_ptr[i] == 0){
				*job_value = rand()%10 + 1;
				queue_ptr[i] = *job_value;
				return;
			}
		}
		cs.release(); // Release critical section semaphore
		jobs_in_queue.release(); // Increase the jobs in queue semaphore by 1
		cout << "Producer " << producer_id << " placed a job of value " << *job_value << endl;
		delete job_value;
	}
}

void process_job(int* queue, int queue_size, int consumer_id, 
				 counting_semaphore<1>& cs, counting_semaphore<100>& empty_spots, counting_semaphore<100>& jobs_in_queue){
	while(true){
		bool job_retrieved = jobs_in_queue.try_acquire_for(chrono::seconds(10));
		if (!job_retrieved){
			cout << "Consumer " << consumer_id << " quit after no jobs appeard in 10 seconds" << endl;
			return;
		}
		int* job_value = new int;
		cs.acquire();
		for (int i=0; i<queue_size; i++){
			if (queue[i] != 0){
				*job_value = queue[i];
				queue[i] = 0;
				return;
			}
		}
		cs.release();
		empty_spots.release(); // Increase the empty spots semaphore by 1
		cout << "Consumer " << consumer_id << " retrieved a job of value " << *job_value << " from the queue" << endl;
		this_thread::sleep_for(chrono::seconds(*job_value));
		cout << "Consumer " << consumer_id << " processed a job of value " << *job_value << endl;
		delete job_value;

	}
}

int main(int argc, char* argv[]){
	int parameters[4];
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-')
			parseArgument(i, argv, &parameters[0]);
	}
	counting_semaphore<1> criticalSection(1);
	counting_semaphore<100> numberOfEmptySpots(10);
	counting_semaphore<100> numberOfJobsInTheQueue(0);
	int queue[10] = {0}; // 100 would be the max size of the buffer

	// Generate all the threads
	thread producers[10];
	thread consumers[10];
	for (int i=0; i<10; i++){
		producers[i] = thread(produce_job, &queue[0], 10, 10, i, ref(criticalSection), ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
	}

	for (int i=0; i<10; i++){
		consumers[i] = thread(process_job, &queue[0], 10, i, ref(criticalSection), ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
	}

	for (int i=0; i<10; i++){
		producers[i].join();
	}

	for (int i=0; i<10; i++){
		consumers[i].join();
	}

	return 0;
}
