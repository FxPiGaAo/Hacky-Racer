# Hacky Racers

This repository is the open source component of our paper: Hacky Racers: Exploiting Instruction-Level Parallelism to Generate Stealthy Fine-Grained Timers (ASPLOS 2023).

It includes following parts of experiments in the paper:
- `Arbitrary Replacement Magnifier Gadget`:  This is an magnifier gadget for set-associative cache with an arbitrary replacement policy. For more details, please refer to  section 6.3 of the paper.
- `Arithmetic-Operation-Only Magnifier Gadget`:  This is an magnifier gadget that doesn't rely on any cache accesses to magnify the timing difference. For more details, please refer to  section 6.4 of the paper.

The experiment would get more stable result if the CPU frequency is set to a stable value. You can either use the [cpuf script](https://askubuntu.com/questions/1141605/gui-or-simple-bash-script-to-throttle-the-cpu/1142671#1142671) or follow step 2 in [streamline attack](https://github.com/gururaj-s/streamline).