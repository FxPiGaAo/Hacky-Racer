# Hacky Racer-integrated Prime+Scope

This directory is built on top of codes from https://github.com/KULeuven-COSIC/PRIME-SCOPE/tree/main/primescope_demo repository, which implements the state-of-the-art LLC eviction set (EV) generation algorithm (prime + scope, CCS' 21).

In order to demonstrate how our Hacky Racer could be easily embedded into the timing part of those side-channel algorithms, in this directory, we take this prime+scope-based EV set generation algorithm as an example. Specifically, we merged the presence/absence Racing Gadget (section 5.1) into the timing function (`ps_evset()` in `evset/ps_evset_inc.c`), without modification to other parts of the code base.

## Changes

The only change that integrates Hacky Racer into Prime+Scope happens at line 85 - 130 in ./PRIME-SCOPE-main/evsets/ps_evset_inc.c. A transient presence/absence racing gadget is implemented here.

## How to run

For detailed instructions, please refer to the README.md under the `PRIME-SCOPE-main` directory. Here is a brief summary related to our arifact evaluation part, as running the `primescope_demo` part would be enough to verify the funcionality of Hacky Racer.
- `cd ./PRIME-SCOPE-main/primescope_demo`
- Follow the instruction under the `primescope_demo` directory to manually set up the `configuration.h`. This step is essential. Currently our code doesn't support running on a non-inclusive directory although it is trivial to apply Hacky Racer to that part, as we do not have a machine to successfully run the non-inclusive part of the original code.
- execute the Prime+Scope test: `make; ./app --primescope`
- execute the eviction set construction test: `make; ./app --evset`