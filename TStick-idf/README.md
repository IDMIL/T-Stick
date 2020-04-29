# T-Stick esp-idf version
* Works with esp-idf v4.0 only
* Uses Arduino as an esp-idf component.

## Compile and run
* Install esp-idf v4.0 as described here: https://docs.espressif.com/projects/esp-idf/en/v4.0/get-started/index.html#installation-step-by-step
* This repository has to be cloned with the `--recursive` flag:
```
git clone --recursive https://github.com/IDMIL/TStick-dev.git
```
* This will clone all the required Arduino libraries
* Build with
```
idf.py build
```
* And flash with
```
idf.py flash
```
