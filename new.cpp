#include<iostream>
#include<string>
#include<semaphore>
#include<thread>
#include<cstdio>

using namespace std;

const int MAX_SIZE = 100;

class CircularQueue{
    public:
        CircularQueue(int queue_size): size(queue_size) {} // Constructor
		
		/**
		 * @brief Get a job from the front of the queue
		 * 
		 * @return int: the value of the retrieved job 
		 */
        int retrievejob(){
			// Store the value from the front and set it to 0
            int tmp = *front;
            *front = 0;

			// If we are not at the end of the queue, move front to the next position
            if ((front+1)!=&queue[size]) 
                front++;
            else // If we have reached the end of the queue, move front to position 0
                front = &queue[0];
            return tmp;
        }

		/**
		 * @brief Place a job at the back of the queue
		 * 
		 * @param job: And int representing the value of the job to be inserted 
		 */
        void placeJob(int job){
            *back = job;
			// Move back pointer (same process as in retrieveJob)
            if ((back+1)!=&queue[size]) 
                back++;
            else
                back = &queue[0];
        }

        /**
         * @brief Print the elements in the queue
         */
        void printQueue(){
            for (int i = 0; i<size; i++){
                cout << queue[i] << " ";
            }
            cout << endl;
        }

    private:
	    // Initialize attributes
        int size; // Chosen size of the queue
        int queue[MAX_SIZE] = {0}; 
        int* back = &queue[0];
        int* front = back;

};

/**
 * @brief Parse an argument from the command line
 * 
 * @param index: the index of the argument to be parsed
 * @param arguments: the array containing the command line arguments 
 * @param params: the array where the 4 parameters will be stored
 */
void parseArgument(int index, char* arguments[], int* params){
	switch (arguments[index][1]){
		case 'q': // queue size
			params[0] = stoi(arguments[index+1]); // Convert sting to int
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

/**
 * @brief Display error message if there are any issues parsing the arguments
 */
void error_message(){
	cerr << "Error parsing the arguments. They must be integers between 1 and 100.\n";
	cerr << "Usage: -q (Queue size)\n       -j (Number of jobs per producers)\n";
	cerr << "       -p (Number of producers)\n       -c (Number of consumers)\n";
}

/**
 * @brief Attempt to produce a job and place it inside the circular queue. Keep 
 * repeating this process until the empty spots semaphore cannot be acquired for
 * 10 seconds or until all the jobs have been produced
 * 
 * @param q: Circular queue where the jobs will be stored 
 * @param max_jobs: Int representing the max number of jobs each producer can produce 
 * @param producer_id: An int representing the ID of the producer 
 * @param cs: Critical section binary semaphore
 * @param empty_spots: Counting semaphore representing the empty spots in the queue 
 * @param jobs_in_queue: Counting semaphore representing the number of jobs in the queue 
 */
void produce_job(CircularQueue& q, int max_jobs, int producer_id, binary_semaphore& cs,
				 counting_semaphore<MAX_SIZE>& empty_spots, counting_semaphore<MAX_SIZE>& jobs_in_queue){

	srand((int) time(0) + producer_id); // Generate a seed for the random number generation
	for (int i=0; i<max_jobs; i++){
		// Attempt to acquire the semaphore for 10 seconds. Exit if not acquired
		bool semaphore_acquired = empty_spots.try_acquire_for(chrono::seconds(10));
		if (!semaphore_acquired){
			printf("Producer %d quit after a spot did not become available for 10 seconds\n", producer_id);
			return;
		}
		int job_value = rand()%10 + 1; // Generate random job
		
		// Acquire critical section semaphore, place the job inside the queue and release it
		cs.acquire();
		q.placeJob(job_value);
		cs.release(); 

		jobs_in_queue.release(); // Increase the jobs in queue semaphore by 1
		//q.printQueue();
		printf("Producer %d placed a job of value %d\n", producer_id, job_value);
	}
	printf("Producer %d quit after submitting all %d jobs.\n", producer_id, max_jobs);
}

/**
 * @brief Attempt to retrieve a job from the circular queue. Keep repeating this process until
 * jobs semaphore cannot be acquired for 10 seconds
 * 
 * @param q: Circular queue where the jobs will be stored 
 * @param consumer_id: An int representing the ID of the consumer 
 * @param cs: Critical section binary semaphore 
 * @param empty_spots: Counting semaphore representing the empty spots in the queue  
 * @param jobs_in_queue: Counting semaphore representing the number of jobs in the queue  
 */
void process_job(CircularQueue& q, int consumer_id, binary_semaphore& cs,
				 counting_semaphore<MAX_SIZE>& empty_spots, counting_semaphore<MAX_SIZE>& jobs_in_queue){

	while(true){
		// Attempt to acquire the semaphore for 10 seconds. Exit if not acquired
		bool job_retrieved = jobs_in_queue.try_acquire_for(chrono::seconds(10));
		if (!job_retrieved){
			printf("Consumer %d quit after no jobs appeared in 10 seconds.\n", consumer_id);
			return;
		}

		// Acquire critical section semaphore, retrieve job and release it
		cs.acquire();
		int job_value = q.retrievejob();
		cs.release();

		empty_spots.release(); // Increase the empty spots semaphore by 1
		printf("Consumer %d retrieved a job of value %d.\n", consumer_id, job_value);
		//q.printQueue();

		// Process the job
		this_thread::sleep_for(chrono::seconds(job_value));
		printf("Consumer %d processed a job of value %d.\n", consumer_id, job_value);
	}
}

int main(int argc, char* argv[]){
	int parameters[4] = {0}; // Initialize parameters to 0
	
	// Parse the command line arguments
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-'){
			try{
				parseArgument(i, argv, &parameters[0]);
			}

			// Exit the program if there is any exception when parsing the arguments
			catch (const exception& e) { 
				error_message();
				exit(1);
			}
		}

	}

	// If any of the parameters are not from 1 to MAX_SIZE, exit the program
	for (int j=0; j<4; j++){
		if (parameters[j] < 1 || parameters[j] > MAX_SIZE){
			error_message();
			exit(1);
		}
	}

	binary_semaphore criticalSection(1);
	counting_semaphore<MAX_SIZE> numberOfEmptySpots(parameters[0]);
	counting_semaphore<MAX_SIZE> numberOfJobsInTheQueue(0);
	CircularQueue q(parameters[0]);

	// Generate all the threads
	thread producers[MAX_SIZE];
	thread consumers[MAX_SIZE];
	try{
		for (int i=0; i<parameters[2]; i++){
			producers[i] = thread(produce_job, ref(q), parameters[1], i, ref(criticalSection),
								  ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
		}
		for (int i=0; i<parameters[3]; i++){
			consumers[i] = thread(process_job, ref(q), i, ref(criticalSection),
								  ref(numberOfEmptySpots), ref(numberOfJobsInTheQueue));
		}
	}
	catch (const exception& e){
		cerr << "Error occurred when creating the threads\n";
		exit(1);
	}

	// Join the threads
	try{
		for (int i=0; i<parameters[2]; i++){
			producers[i].join();
		}
		for (int i=0; i<parameters[3]; i++){
			consumers[i].join();
		}
	}
	catch (const exception& e) {
		cerr << "Error occurred when joining the threads\n";
		exit(1);
	}
	
	return 0;
}
