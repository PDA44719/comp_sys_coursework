#include<iostream>
#include<queue>
#include<string>

using namespace std;

void parseArgument(int index, char* arguments[], int& params[4]){
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

int main(int argc, char* argv[]){
	cout << "The arguments entered were: ";
	int parameters[4];
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-')
			parseArgument(i, argv, parameters);
	}
	for (int j=0; j<4; j++){
		cout << parameters[j] << endl;
	}
	return 0;
}
