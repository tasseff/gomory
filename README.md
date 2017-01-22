# gomory

## Installation

To retrieve dependencies included as submodules (e.g., rapidjson), run:

```bash
git submodule update --init --recursive
```

Finally, compile the software:

```bash
mkdir build && cd build
cmake .. && make
```

To compile on OSX, try something like

```bash
cmake -DCMAKE_C_COMPILER=/usr/local/bin/gcc-6 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-6
```
