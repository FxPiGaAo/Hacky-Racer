# Hacky Racers

This repository is the open source component of our paper: Hacky Racers: Exploiting Instruction-Level Parallelism to Generate Stealthy Fine-Grained Timers (ASPLOS 2023 Distinguished Paper Award, Micro Top Picks 2024 Honorable Mention).

It includes following parts of experiments in the paper:
- `Arbitrary Replacement Magnifier Gadget`:  This is an magnifier gadget for set-associative cache with an arbitrary replacement policy. For more details, please refer to  section 6.3 of the paper.
- `Arithmetic-Operation-Only Magnifier Gadget`:  This is an magnifier gadget that doesn't rely on any cache accesses to magnify the timing difference. For more details, please refer to  section 6.4 of the paper.
- `spectre.js`:  This is a JavaScript demo for SpectreBack attack. For more details, please refer to  section 7.3 & 6.2 of the paper.

- `PRIME-SCOPE-hackyracer`:  This is a [Prime+Scope](https://antoonpurnal.github.io/files/pdf/PrimeScope.pdf) based eviction set profiling algoritm whose timer is intergrated with Hacky Racer.

Please follow intructions in each folder's README.md to run each experiment.

The experiment would get more stable result if the CPU frequency is set to a stable value. You can use the [cpuf](https://askubuntu.com/questions/1141605/gui-or-simple-bash-script-to-throttle-the-cpu/1142671#1142671) script here to fix the frequency.
- install yad package: `apt-get install yad`
- execute the cpuf.sh: `chmod a+x ./cpuf; sudo ./cpuf`
- set the new minimal and maximum frequency to the same value (between allowable minimun and maximun frequency); Then if the current frequency equals to that value, the frequency is fixed successfully. Some frequency value might be denied depending on CPU model. Therefore, please try different values, such as 2GHZ, 1GHz, 1.5GHz, until it works. 

All files and folders under this directory follow the Apache 2.0 License located in this directory.
