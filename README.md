# OECG
Open source Edge Computing Gateway


1. clone
```bash
git clone https://github.com/dissor/oecg.git
cd oecg
git submodule init
git submodule update
```

2. build
```bash
cd oecg
cmake -H. -B build
make -C build
```

3. run
```bash
cd oecg
. build/components/main.code/demo
```
