#include<iostream>
#include<queue>
#include<string>
#include<cstdlib>
#include<ctime>
#include<semaphore>
#include<chrono>
#include<thread>

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
        int queue[100] = {0}; // 100 is the max value of the queue
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
				 binary_semaphore& cs, counting_semaphore<100>& empty_spots, counting_semaphore<100>& jobs_in_queue){
	srand((int) time(0) + producer_id); // Generate a seed
	for (int i=0; i<max_jobs; i++){
		bool job_placed = empty_spots.try_acquire_for(chrono::seconds(10));
		if (!job_placed){
			cout << "Producer " << producer_id << " quit after a spot did not become available for 10 seconds." << endl;
			return;
		}
		int* job_value = new int;
		//cout << "Made it here " << endl;
		bool critical_section_entry = cs.try_acquire_for(chrono::seconds(10));
		if (!critical_section_entry){
			cout << "Producer " << producer_id << " quitting" << endl;
			return;
		}
		//for (int i=0; i<queue_size; i++){
		//	if (queue_ptr[i] == 0){
		//		*job_value = rand()%10 + 1;
		//		queue_ptr[i] = *job_value;
		//		break;
		//	}
		//}
		q.placeJob(rand()%10 + 1);
		q.printQueue();
		cs.release(); // Release critical section semaphore
		jobs_in_queue.release(); // Increase the jobs in queue semaphore by 1
		//cerr << "Producer " << producer_id << " placed a job of value " << *job_value << endl;
		delete job_value;
	}
}

void process_job(CircularQueue& q, int consumer_id, 
				 binary_semaphore& cs, counting_semaphore<100>& empty_spots, counting_semaphore<100>& jobs_in_queue){
	while(true){
		bool job_retrieved = jobs_in_queue.try_acquire_for(chrono::seconds(10));
		if (!job_retrieved){
			cout << "Consumer " << consumer_id << " quit after no jobs appeard in 10 seconds" << endl;
			return;
		}
		//int* job_value = new int;
		cs.acquire();
		//for (int i=0; i<queue_size; i++){
			//if (queue[i] != 0){
				//*job_value = queue[i];
				//queue[i] = 0;
				//break;
			//}
		//}
		int job_value = q.retrievejob();
		q.printQueue();
		cs.release();
		empty_spots.release(); // Increase the empty spots semaphore by 1
		//cerr << "Consumer " << consumer_id << " retrieved a job of value " << job_value << " from the queue" << endl;
		this_thread::sleep_for(chrono::seconds(job_value));
		cerr << "Consumer " << consumer_id << " processed a job of value " << job_value << endl;
		//delete job_value;

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
	counting_semaphore<100> numberOfJobsInTheQueue(0);

	CircularQueue q(parameters[0]);

	// Generate all the threads
	thread producers[100];
	thread consumers[100];
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
