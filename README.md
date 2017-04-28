# gomory

## Compilation

To retrieve dependencies included as submodules (e.g., rapidjson), run:

```bash
git submodule update --init --recursive
```

If using SoPlex, [download it here](http://soplex.zib.de), then extract it into the `libraries` folder using the subfolder name `soplex`, e.g.,

```bash
tar -xzvf ~/Downloads/soplex-3.0.0.tgz -C libraries/ --transform 's!^soplex-3.0.0\($\|/\)!soplex\1!'
```

Finally, compile the software:

```bash
mkdir build && cd build
cmake .. && make
```
