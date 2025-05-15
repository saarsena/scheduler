#include <iostream>
#include "../include/EnttEventExample.h"

// === Direction enum (optional movement logic) ===
enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };

// Forward declarations of example classes
class EnttEventExample;

// Define the SchedulerExample class
class SchedulerExample {
public:
    void run();
};

// Implementation of example runner functions
void runSchedulerExamples() {
    SchedulerExample schedulerExample;
    schedulerExample.run();
}

void runEnttEventExamples() {
    EnttEventExample example;
    example.run();
}

int main() {
    runSchedulerExamples();
    runEnttEventExamples();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

