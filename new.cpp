#include<iostream>
#include<queue>
#include<string>
#include<cstdlib>
#include<ctime>

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

void produce_job(int* queue_ptr, int size){
	srand((int) time(0)); // Generate a seed
	for (int i=0; i<size; i++){
		if (queue_ptr[i] == 0){
			queue_ptr[i] = rand()%10 + 1;
			return;
		}
	}
}

void process_job(int* queue){

}

int main(int argc, char* argv[]){
	int parameters[4];
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-')
			parseArgument(i, argv, &parameters[0]);
	}
	int queue_size = parameters[0];
	int queue[100] = {0}; // 100 would be the max size of the buffer
	produce_job(queue, queue_size);
	cout << "First job finished" << endl;
	cout << "Second job starting" << endl;
	produce_job(queue, queue_size);
	for (int i=0; i<queue_size; i++)
		cout << queue[i] << endl;
	return 0;
}
