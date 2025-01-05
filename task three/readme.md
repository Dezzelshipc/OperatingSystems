# Task 3: Shared memory with counter
1. Lib for running other copies of program: [processes.hpp](processes.hpp)
2. Lib for managing shared memory: [shmem.hpp](shmem.hpp)
3. Program: [main.cpp](main.cpp)
```
shmem [program behavior | 0]
```
* BEH 0: endless loop with counter and start of copies
* BEH 1: adds +10 to counter
* BEH 2: doubles counter then halfs it after 2 seconds

Log in format: [**{time}** | **{pid}**] _{Counter}_ / _Started {BEH}_ / _Finished {BEH}_  