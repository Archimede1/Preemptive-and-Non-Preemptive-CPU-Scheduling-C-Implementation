#include <iostream>
#include <string>
#include <fstream>
using namespace std;
 
 //structure that represents the idea of a Process. Has things like burstTime, process id, arrivalTime, etc.
 //all characters of a process
struct Process {
    int pid;
    int burstTime;
    int remBurstTime;
    int arrivalTime;
    int waitTime;
    int executionTime;
    int priority;

    bool operator<(const Process& other)const {
        return priority < other.priority;
    }

    bool operator>(const Process& other)const {
        return priority > other.priority;
    }
};

//A Queue implementation for Process. This was created to help manage processes in different states
struct ProcessQueue {
    int front, back, size, amntElements;
    Process* queue;

    ProcessQueue() {
        (*this).front = (*this).back = (*this).amntElements = 0;
        (*this).size = 0;
        (*this).queue = new Process[size];
    }

    ProcessQueue(int size) {
        (*this).front = (*this).back = (*this).amntElements = 0;
        (*this).size = size;
        (*this).queue = new Process[size];
    }

    ~ProcessQueue() {
        delete[] queue;
    }

    void enqueue(Process newProcess) {
        if (size == amntElements) {
            cout << "Queue is Full!" << endl;
            return;
        }
        else {
            (*this).queue[back] = newProcess;
            amntElements++;
            back++;
        }
        return;
    }

    Process dequeue() {
        Process removed = queue[front];
        if (amntElements == 0) {
            cout << "Queue is Empty!" << endl;
        }
        else {
            for (int i = 0; i < back - 1; i++) {
                queue[i] = queue[i + 1];
            }
            amntElements--;
            back--;
        }
        return removed;
    }

    void sort() {
        bool flag;
        do {
            flag = false;
            for (int i = 0; i < amntElements - 1; i++) {
                if (queue[i] > queue[i + 1]) {
                    Process temp = queue[i];
                    queue[i] = queue[i + 1];
                    queue[i + 1] = temp;
                    flag = true;
                }
            }
        } while (flag);
    }

    bool isEmpty() {
        if (amntElements == 0) {
            return true;
        }
        else {
            return false;
        }
    }
    Process frontElement() {
        return queue[front];
    }

    void resetQueue() {
        (*this).front = (*this).back = (*this).amntElements = 0;
        (*this).queue = new Process;
    }

    void incrementWaitTime(int index) {
        queue[index].waitTime++;
    }

    void incrementExecutionTime(int index) {
        queue[index].executionTime++;
    }
};

//hold all the algorithms for has schedule the process, intended to work in conjuction with each other
//but because of many errors and accumalating overdue time, the method more so work alone or inpedent of each other
struct SchedulingAlgorithms {
    static void roundRobinScheduling(ProcessQueue requestQueue, ProcessQueue& arrival, Process* process, int& executed, int quanTime, int& time) {
        ProcessQueue executing;
        int numbProcesses = requestQueue.amntElements;
        int rrExecuted = 0, quanCounter = 0;
        requestQueue.sort();
        while (!(rrExecuted >= numbProcesses)) {
            //to continue to allows processes to arrive while the roundRobin algorithm is happening between
            //two processes that have similar priority
            for (int i = 0; i < 10; i++) {
                if (time == process[i].arrivalTime) {
                    arrival.enqueue(process[i]);
                    //for debugging reasons
                    // cout << "(" << time << "): Process " << arrival.frontElement().pid << " in Arrival Queue" << endl;  
                }
            }
            //once a process concludes executing for its respective time, another one from the requestqueue is added
            if (executing.isEmpty()) {
                executing.enqueue(requestQueue.dequeue());
                cout << time << "  " << executing.frontElement().pid << endl;
                cout << "(" << time << "): Process " << executing.frontElement().pid << " Started Executing || Wait Time: " << executing.frontElement().waitTime << endl;
            }
            //increments remBurstTime and executionTime
            if (!executing.isEmpty()) {
                executing.incrementExecutionTime(executing.front);
                executing.queue[0].remBurstTime--;
            }
            //increments waitTimes of process in requestQueue
            if (!requestQueue.isEmpty()) {
                for (int i = 0; i < requestQueue.amntElements; i++) {
                    requestQueue.incrementWaitTime(i);
                }
            }
            time++;
            quanCounter++;
            //if remaining burst time falls to 0, the process is completely finished executing 
            if (executing.frontElement().remBurstTime == 0) {
                cout << "(" << time << "): Process " << executing.frontElement().pid << " Finished Executing" << endl;
                executing.dequeue();
                rrExecuted++;
                executed++;
            }
            //if quanTime is at a mulitple of 10, process executing, burst time finished or not, will stop executing
            if (quanCounter % quanTime == 0 && executing.frontElement().remBurstTime > 0 && !executing.isEmpty()) {
                requestQueue.enqueue(executing.dequeue());
                cout << "(" << time << "): Process " << executing.frontElement().pid << " has Stopped Executing with " << executing.frontElement().remBurstTime << " remaining Burst Time" << endl;
                cout << "(" << time << "): Process " << executing.frontElement().pid << " in RR.Request Queue" << endl;
            }
        }
    }

    void preemptiveScheduling(Process* processes, int numbProcesses, int quanTime) {
        ProcessQueue executing, arrival, finished;
        int time = 0, preempCount = 0;
        arrival.size = finished.size = numbProcesses;
        executing.size = 1;
        processSort(processes, numbProcesses);
        for (int i = 0; i < numbProcesses; i++) {
            processes[i].remBurstTime = processes[i].burstTime;
        }
        while (!(preempCount >= numbProcesses)) {
            for (int i = 0; i < numbProcesses; i++) {
                if (time == processes[i].arrivalTime) {
                    arrival.enqueue(processes[i]);
                    //for debugging reasons
                    // cout << "(" << time << "): Process " << arrival.frontElement().pid << " in Arrival Queue" << endl;  
                }
            }
            //removes process from executing if priority time has been met
            if (!executing.isEmpty() && executing.frontElement().executionTime == executing.frontElement().priority) {
                //for debugging reason
                // cout << "(" << time << "): Process " << executing.frontElement().pid << " has Finished with Executing Priority" << endl;
                finished.enqueue(executing.dequeue());
                preempCount++;
            }
            if (time == 0 || executing.isEmpty()) {
                if (!arrival.isEmpty()) {
                    executing.enqueue(arrival.dequeue());
                    cout << time << " " << executing.frontElement().pid << endl;
                    //for debugging reasons
                    // cout << "(" << time << "): Process " << executing.frontElement().pid << " has Started Executing" << endl;
                }
            }
            if (!executing.isEmpty()) {
                executing.incrementExecutionTime(executing.front);
                executing.queue[0].remBurstTime--;
            }
            if (!arrival.isEmpty()) {
                for (int i = 0; i < arrival.amntElements; i++) {
                    arrival.incrementWaitTime(i);
                }
            }
            time++;
        }
        for (int i = 0; i < numbProcesses; i++) {
            processes[i] = finished.dequeue();
        }
        // cout << "done" << endl;
        nonPreemptiveScheduling(processes, numbProcesses, quanTime, time);
    }

    void nonPreemptiveScheduling(Process* process, int numbProcesses, int quanTime, int time) {
        int executed = 0;
        Process compare;
        ProcessQueue executing, arrival, finished, repeats;
        arrival.size = finished.size = repeats.size = numbProcesses;
        executing.size = 1;
        processSort(process, numbProcesses);
        while (!(executed >= numbProcesses)) {
            for(int i = 0; i < numbProcesses; i++) {
                if (time == process[i].arrivalTime) {
                    arrival.enqueue(process[i]);
                    //debugging reasons
                    //cout << "(" << time << "): Process " << process[i].pid << " in Arrival Queue" << endl;
                }
            }
            //removes process if executionTime equals the process burst time
            if (executing.frontElement().burstTime <= executing.frontElement().executionTime && !executing.isEmpty()) {
                cout << time << "  " << executing.frontElement().pid << endl;
                //debugging reasons
                // cout << "(" << time << "): Process " << executing.frontElement().pid << " Finished Executing" << endl;
                finished.enqueue(executing.dequeue());
                executed++;
            }
            //this is used to handle processes with the same priority, within these next lines of code
            //the roundRobinScheduling method is call to handle processes with similar priority

            // if(!arrival.isEmpty()){
            //   compare = arrival.frontElement();
            //   for(int i = 0; i < numbProcesses; i++){
            //     if(compare.priority == arrival.queue[i].priority){
            //       repeats.enqueue(arrival.dequeue());
            //     }
            //   }
            //   if(compare.priority == executing.queue[0].priority){
            //     repeats.enqueue(executing.dequeue());
            //   }
            //   repeats.sort();
            //   roundRobinScheduling(repeats, arrival, process, executed, quanTime, time);
            // }
            
            if (time == 0 || executing.isEmpty() == true) {
                if (arrival.amntElements > 1) {
                    arrival.sort();
                }
                if (!arrival.isEmpty()) {
                    executing.enqueue(arrival.dequeue());
                    //debugging reasons
                    //cout << "(" << time << "): Process " << executing.frontElement().pid << " Started Executing" << endl;
                }
            }
            if (!executing.isEmpty()) {
                executing.incrementExecutionTime(executing.front);
                executing.queue[0].remBurstTime--;
            }
            if (!arrival.isEmpty()) {
                for (int i = 0; i < arrival.amntElements; i++) {
                    arrival.incrementWaitTime(i);
                }
            }
            time++;
        }
        cout << "Time taken: " << time - 1 << endl << endl;
        cout << "Process  Wait" << endl;
        for(int i = 0; i < numbProcesses; i++){
          cout << finished.queue[i].pid << " " << finished.queue[i].waitTime << endl;
        }
    }

    void processSort(Process process[], int numbProcesses) {
        bool flag;
        do {
            flag = false;
            for (int i = 0; i < numbProcesses - 1; i++) {
                if (process[i] > process[i + 1]) {
                    Process temp = process[i];
                    process[i] = process[i + 1];
                    process[i + 1] = temp;
                    flag = true;
                }
            }
        } while (flag);
    }

};
//reads data from file
void readProcessDataFile(Process* process, int numbProcesses, string inputfile) {
    ifstream processData;
    processData.open(inputfile);
    int index = 0;
    while (index != numbProcesses) {
        processData >> process[index].pid >> process[index].priority >> process[index].burstTime >> process[index].arrivalTime;
        index++;
    }
}

int main(int argc, char *argv[]) {
    Process processes[10];
    SchedulingAlgorithms sa;
    int quantumTime = 10, numbProcesses = 10;
    readProcessDataFile(processes, numbProcesses, argv[1]);
    for (int i = 0; i < numbProcesses; i++) {
        processes[i].waitTime = processes[i].executionTime = 0;
    }
    // for(int i = 0; i < numbProcesses; i++){
    //   cout << processes[i].pid << endl;
    // }

    cout << "Part 1 Non-PreemptiveScheduling\n" << endl;
    cout << "Time  Process" << endl;
    sa.nonPreemptiveScheduling(processes, numbProcesses, quantumTime, 0);
    cout << "Part 2 PreemptiveScheduling\n" << endl;
    cout << "Time  Process" << endl;
    sa.preemptiveScheduling(processes, numbProcesses, quantumTime);
    
}