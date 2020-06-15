#!/bin/bash
gcc -pthread -o DiningPhilosophers DiningPhilosophers.c
./DiningPhilosophers
echo -----------------------------------------------------------------------------
echo Run the program again, printing only moments where philosophers eat
echo -----------------------------------------------------------------------------
./DiningPhilosophers | grep "I'm eating"


