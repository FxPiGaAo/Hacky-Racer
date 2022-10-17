# Hacky Racer-based SpectreBack Attack JavaScript PoCs

This directory is built on top of codes from https://github.com/google/security-research-pocs/tree/master/spectre.js repository, which implements the first JavaScript [Spectre PoC attack](https://leaky.page).

In this directory, we demonstrate a PoC for SpectreBack Attack (section 7.3) based on PLRU Gadget for Reorder Input (section 6.2).

## How to run
- install pip: `sudo apt install python-pip`
- install flask: `pip install Flask`
- run the server on your local: `cd leaky.page; python main.py`
- copy the output IP and open it on the chrome88 browser (can be download from https://www.slimjet.com/chrome/google-chrome-old-version.php)
- Follow the instruction to run the task on each page. (Don't forget to untick the "stable timer" box from the timer page.) The result of a successful run is shown in the [succeed_result_spectreback.png](./succeed_result_spectreback.png).
- to shutdown the local server, press ctrl+c in that terminal.


# Changes
To comply to the Apach 2.0 license from that repository, a list of changes to that code is presented below. The license is also put under this directory.

## cachetools.wat
This is the webasembly code for PLRU based magnifier. We modify it from present/absence input into reorder input (see section 6.2).

## l1timer.js
This file contained the binary compiled from cachetools.wat. Since we modified cachetools.wat, we recompiled and replaced the binary in this file.

## spectre_worker.js
The spectreGadget function in this file is where the secret is leaked. We replace it with the method in SpectreBack attack. We also made other modifications such as EV list generation stage to make this attack happen.

## *.html
Change those text descriptions for our SpectreBack attack