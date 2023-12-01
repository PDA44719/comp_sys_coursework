#include <iostream>

using namespace std;

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


int main(){
    CircularQueue q(4);
    q.placeJob(5);
    q.placeJob(6);
    q.printQueue();
    cout << q.retrievejob() << " was retrieved" << endl;
    q.printQueue();
    q.placeJob(7);
    q.placeJob(8);
    q.printQueue();
    cout << q.retrievejob() << " was retrieved" << endl;
    q.placeJob(1);
    q.printQueue();
    q.placeJob(2);
    q.printQueue();
    cout << q.retrievejob() << " was retrieved" << endl;
    cout << q.retrievejob() << " was retrieved" << endl;
    cout << q.retrievejob() << " was retrieved" << endl;
    q.printQueue();
    q.placeJob(3);
    q.printQueue();

    return 0;
}