#include<iostream>
#include<string>
#include<semaphore>
#include<thread>
#include<cstdio>

using namespace std;

const int MAX_SIZE = 100;

class CircularQueue{
    public:
        CircularQueue(int queue_size): size(queue_size) {}
        int retrievejob(){
            int tmp = *front;
            *front = 0;
            if ((front+1)!=&queue[size]) // we have not reached the last cell of the queue
                front++;
            else
                front = &queue[0];
            return tmp;
        }

        void placeJob(int job){
            *back = job;
            if ((back+1)!=&queue[size]) // we have not reached the last cell of the queue
                back++;
            else
                back = &queue[0];
        }

        void printQueue(){
            for (int i = 0; i<size; i++){
                cout << queue[i] << " ";
            }
            cout << endl;
        }

    private:
        int size;
        int queue[MAX_SIZE] = {0}; // 100 is the max value of the queue
        int* back = &queue[0];
        int* front = back;

};

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

void produce_job(CircularQueue& q, int max_jobs, int producer_id,
				 binary_semaphore& cs, counting_semaphore<MAX_SIZE>& empty_spots, counting_semaphore<MAX_SIZE>& jobs_in_queue){
	srand((int) time(0) + producer_id); // Generate a seed
	for (int i=0; i<max_jobs; i++){
		bool job_placed = empty_spots.try_acquire_for(chrono::seconds(10));
		if (!job_placed){
			//cerr << "Producer " << producer_id << " quit after a spot did not become available for 10 seconds." << endl;
			printf("Producer %d quit after a spot did not become available for 10 seconds\n", producer_id);
			return;
		}
		int job_value = rand()%10 + 1;
		cs.acquire();
		//if (!critical_section_entry){
		//	cout << "Producer " << producer_id << " quitting" << endl;
		//	return;
		//}
		q.placeJob(job_value);
		cs.release(); // Release critical section semaphore
		jobs_in_queue.release(); // Increase the jobs in queue semaphore by 1
		q.printQueue();
		//cerr << "Producer " << producer_id << " placed a job of value " << job_value << endl;
		printf("Producer %d placed a job of value %d\n", producer_id, job_value);
		//delete job_value;
	}
	//cerr << "Producer " << producer_id << " quit after submitting all " << max_jobs << " jobs." << endl;
	printf("Producer %d quit after submitting all %d jobs.\n", producer_id, max_jobs);
}

void process_job(CircularQueue& q, int consumer_id, 
				 binary_semaphore& cs, counting_semaphore<MAX_SIZE>& empty_spots, counting_semaphore<MAX_SIZE>& jobs_in_queue){
	while(true){
		bool job_retrieved = jobs_in_queue.try_acquire_for(chrono::seconds(10));
		if (!job_retrieved){
			//cerr << "Consumer " << consumer_id << " quit after no jobs appeard in 10 seconds" << endl;
			printf("Consumer %d quit after no jobs appeared in 10 seconds.\n", consumer_id);
			return;
		}
		cs.acquire();
		int job_value = q.retrievejob();
		cs.release();
		empty_spots.release(); // Increase the empty spots semaphore by 1
		q.printQueue();
		//cerr << "Consumer " << consumer_id << " retrieved a job of value " << job_value << " from the queue" << endl;
		printf("Consumer %d retrieved a job of value %d.\n", consumer_id, job_value);
		this_thread::sleep_for(chrono::seconds(job_value));
		//cerr << "Consumer " << consumer_id << " processed a job of value " << job_value << endl;
		printf("Consumer %d processed a job of value %d.\n", consumer_id, job_value);
	}
}

int main(int argc, char* argv[]){
	int parameters[4];
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-')
			parseArgument(i, argv, &parameters[0]);
	}
	binary_semaphore criticalSection(1);
	counting_semaphore<MAX_SIZE> numberOfEmptySpots(parameters[0]);
	counting_semaphore<MAX_SIZE> numberOfJobsInTheQueue(0);

	CircularQueue q(parameters[0]);

	// Generate all the threads
	thread producers[MAX_SIZE];
	thread consumers[MAX_SIZE];
	for (int i=0; i<parameters[2]; i++){
		producers[i] = thread(produce_job, ref(q), parameters[1], i, ref(criticalSection), ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
	}

	for (int i=0; i<parameters[3]; i++){
		consumers[i] = thread(process_job, ref(q), i, ref(criticalSection), ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
	}

	for (int i=0; i<parameters[2]; i++){
		producers[i].join();
	}

	for (int i=0; i<parameters[3]; i++){
		consumers[i].join();
	}

	return 0;
}
