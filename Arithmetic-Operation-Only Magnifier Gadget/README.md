### run
execute `chmod a+x ./test.sh; ./test.sh` in the terminal

### result
The script will generate and compile different `arithmetic` programs from arithmetic_source.c to find a path length match. If a match is successfully found, it will output the accumulated timing differences, which should be linear to their magnify rounds, as each round will add the same amount of timing difference. The timing difference might grow slower or stop growing after the round number is bigger than some threshold as is shown in figure 12, since the timer interrupt will stop the timing difference from accumulating. The result of a successful run is shown in the [succeed_result_arithmetic.png](./succeed_result_arithmetic.png).