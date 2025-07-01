//Chady Oueslati CpuScheduler / Process Management Logic
//University Solo Project

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <queue>

struct Process {
    int pid;                   // process ID
    int arrival_time;       // time when process arrives
    int burst_time;       // total CPU time needed
    int memory_req;        // memory requirement
    int start_time;         // time when process starts execution
    int completion_time;      // time when process completes execution
    int waiting_time;    // total time process spent waiting
    int turnaround_time;    // total time from arrival to completion
    int remaining_time;   // remaining CPU time (for preemptive algorithms)
};

// Function to print scheduling results given a list of processes and a title
void print_results(std::vector<Process> procs, const std::string& title) {
    std::sort(procs.begin(), procs.end(),
              [](const Process &a, const Process &b) {
                  return a.start_time < b.start_time;
              });
    std::cout << "\n" << title << " Results:\n";                                     // header results
    std::cout << std::left
              << std::setw(6)  << "PID"
              << std::setw(14) << "Arrival"
              << std::setw(10) << "Burst"
              << std::setw(10) << "MemReq"  // table column headers SHOWN IN RESULTS
              << std::setw(12) << "Start"
              << std::setw(12) << "Complete"
              << std::setw(12) << "Waiting"
              << std::setw(14) << "Turnaround"
              << "\n";
    std::cout << std::string(90, '-') << "\n";                                          // lines to seperate the coldumns

    double total_wait = 0, total_turn = 0;                                                // accumulators for averages
    for (auto& p : procs) {
        std::cout << std::setw(6)  << p.pid
                  << std::setw(14) << p.arrival_time
                  << std::setw(10) << p.burst_time
                  << std::setw(10) << p.memory_req
                  << std::setw(12) << p.start_time
                  << std::setw(12) << p.completion_time
                  << std::setw(12) << p.waiting_time
                  << std::setw(14) << p.turnaround_time
                  << "\n";                                                              // print each process's metrics
        total_wait += p.waiting_time;                                                     // sum waiting times
        total_turn += p.turnaround_time;                                                  // sum turnaround times
    }

    int n = procs.size();                                                                  // number of processes
    std::cout << "\nAverage waiting time:    " << (total_wait / n) << "\n";              // output avg waiting time
    std::cout << "Average turnaround time: " << (total_turn / n) << "\n";                 // output avg turnaround time
}

// First-Come First-Served scheduling (non-preemptive) algorithm logic
void schedule_FCFS(std::vector<Process> procs) {
    std::sort(procs.begin(), procs.end(),                                                     // sort by arrival time
              [](const Process &a, const Process &b) {
                  return a.arrival_time < b.arrival_time;
              });
    int current_time = 0;                                                                     // tracks current clock
    for (auto &p : procs) {
        if (current_time < p.arrival_time)
            current_time = p.arrival_time;                                                   // idle until next arrival
        p.start_time = current_time;                                                   // start when CPU free
        p.completion_time = p.start_time + p.burst_time;                                      // compute completion
        p.turnaround_time = p.completion_time - p.arrival_time;        // compute turnaround
        p.waiting_time = p.start_time - p.arrival_time;                                 // compute waiting
        current_time = p.completion_time;                                             // advance clock
    }
    print_results(procs, "FCFS");                                                           // display results
}

// Shortest Job First scheduling (non-preemptive)
void schedule_SJF(std::vector<Process>& procs) {
    int n = procs.size();                                                             // total processes
    int completed = 0;                                                           // count completed
    int current_time = 0;                                                              // system clock
    std::vector<bool> done(n, false);                                      // track finished jobs
    std::vector<int> burst_copy(n);                                           // local copy of burst times
    for (int i = 0; i < n; ++i) burst_copy[i] = procs[i].burst_time;

    while (completed < n) {
        int idx = -1;
        int min_burst = std::numeric_limits<int>::max();                              // find shortest among arrived
        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival_time <= current_time &&
                burst_copy[i] < min_burst) {
                min_burst = burst_copy[i];
                idx = i;
                }
        }
        if (idx == -1) {                                                                      // no job ready
            current_time++;                                                                   // idle tick
            continue;
        }
        auto &p = procs[idx];
        p.start_time = current_time;                           // start selected job
        p.completion_time = p.start_time + burst_copy[idx];
        p.turnaround_time = p.completion_time - p.arrival_time;
        p.waiting_time = p.start_time - p.arrival_time;
        current_time = p.completion_time;                                                    // advance clock
        done[idx] = true;                                                                // mark finished
        completed++;                                          // increment count
    }
    print_results(procs, "SJF");                                     // display results
}

// Shortest Remaining Time First scheduling (preemptive)
void schedule_SRTF(std::vector<Process> procs) {
    int n = procs.size();                     // process count
    std::vector<int> remaining(n);   // remaining bursts
    for (int i = 0; i < n; ++i) {
        remaining[i] = procs[i].burst_time;                               // init remaining
        procs[i].start_time = -1;                                 // mark unstarted
    }
    int completed = 0;         // completed count
    int current_time = 0;     // clock

    while (completed < n) {
        int idx = -1;
        int min_rem = std::numeric_limits<int>::max();                                        // find ready job with least rrequired mem
        for (int i = 0; i < n; ++i) {
            if (procs[i].arrival_time <= current_time && remaining[i] > 0 &&
                remaining[i] < min_rem) {
                min_rem = remaining[i];                                                      // update minimum
                idx = i;
            }
        }
        if (idx == -1) {                                                                      // no job ready
            current_time++;                                                                   // idle tick
            continue;
        }
        if (procs[idx].start_time == -1)
            procs[idx].start_time = current_time;                                           // set first start time
        remaining[idx]--;                                           // execute one unit
        current_time++;      // advance clock
        if (remaining[idx] == 0) {                                                            // job finished
            completed++;                                                                      // update count
            procs[idx].completion_time = current_time;
            procs[idx].turnaround_time = procs[idx].completion_time - procs[idx].arrival_time;
            procs[idx].waiting_time = procs[idx].turnaround_time - procs[idx].burst_time;
        }
    }
    print_results(procs, "SRTF");                                                           // display results
}

// Round Robin scheduling (preemptive with time quantum)
void schedule_RR(std::vector<Process> procs, int quantum) {
    int n = procs.size();                                                                     // total processes
    std::queue<int> q;                                                                        // queue of ready indices
    std::vector<int> remaining(n);                                                            // remaining bursts
    std::vector<bool> in_queue(n, false);                                                     // track enqueued
    for (int i = 0; i < n; ++i)                                                               // init remaining
        remaining[i] = procs[i].burst_time;
    int current_time = 0, completed = 0;

    // enqueue arrivals at time 0
    for (int i = 0; i < n; ++i) {
        if (procs[i].arrival_time == 0) {
            q.push(i);
            in_queue[i] = true;                                                               // mark enqueued
        }
    }
    while (completed < n) {
        if (q.empty()) {                                                                      // no ready process
            current_time++;                                                                   // idle tick
            for (int i = 0; i < n; ++i) {
                if (!in_queue[i] && remaining[i] > 0 && procs[i].arrival_time <= current_time) {
                    q.push(i);
                    in_queue[i] = true;                                                       // enqueue new arrival
                }
            }
            continue;
        }
        int idx = q.front(); q.pop();                                                         // dequeue
        auto &p = procs[idx];
        if (p.start_time == 0 && remaining[idx] == p.burst_time)                               // first time running
            p.start_time = std::max(current_time, p.arrival_time);
        current_time = std::max(current_time, p.arrival_time);                                // wait if arrived later
        int exec = std::min(quantum, remaining[idx]);                                         // compute this slice length
        remaining[idx] -= exec;                                                               // deduct executed
        current_time += exec;                                                                 // advance clock
        // enqueue newly arrived during slice
        for (int i = 0; i < n; ++i) {
            if (!in_queue[i] && remaining[i] > 0 && procs[i].arrival_time <= current_time) {
                q.push(i);
                in_queue[i] = true;
            }
        }
        if (remaining[idx] > 0) {                                                              // still has work
            q.push(idx);                                                                      // requeue
        } else {
            completed++;                                                                      // finish
            p.completion_time = current_time;
            p.turnaround_time = p.completion_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
        }
    }
    print_results(procs, "Round Robin (quantum=" + std::to_string(quantum) + ")");
}

int main() {
    int n;
    std::cout << "Enter number of processes: "; std::cin >> n;                               // ask user for count
    std::vector<Process> processes(n);
    for (int i = 0; i < n; ++i) {
        processes[i].pid = i + 1;                                                             // assign PIDs
        std::cout << "\nProcess " << processes[i].pid << " Details:";
        std::cout << "\n  Arrival time: "; std::cin >> processes[i].arrival_time;         // input arrival
        std::cout << "  Burst time: ";    std::cin >> processes[i].burst_time;             // input burst
        std::cout << "  Memory req: ";    std::cin >> processes[i].memory_req;             // input memory
        processes[i].remaining_time = processes[i].burst_time;                               // init remaining
    }
    int choice;
    std::cout << "\nSelect scheduling algorithm:";
    std::cout << "\n  1. FCFS";
    std::cout << "\n  2. SJF (Non-preemptive)";
    std::cout << "\n  3. SRTF (Preemptive)";
    std::cout << "\n  4. Round Robin";
    std::cout << "\nChoice: "; std::cin >> choice;                                       // select algo
    int quantum = 0;
    if (choice == 4) {
        std::cout << "Enter time quantum: "; std::cin >> quantum;                           // input quantum for RR
    }
    switch (choice) {
        case 1: schedule_FCFS(processes); break;                                              // FCFS
        case 2: schedule_SJF(processes);  break;                                              // SJF
        case 3: schedule_SRTF(processes); break;                                              // SRTF
        case 4: schedule_RR(processes, quantum); break;                                       // Round Robin
        default: std::cout << "Invalid choice\n";                                          // error
    }
    return 0;                                                                                // exit
}
