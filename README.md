# A PDP8 Emulator

This is a PDP8 emulator written in C.

## Requirements

- `gcc` or another C compiler (this project is made using `gcc`, another compiler may not compile).
- Unix system is require to use the build system. (If you are not on a unix-based machine you can compile the project by yourself).

## How to run the project

### Fast way

1. Run these commands

```
git clone https://github.com/BudinoSurelySweet/pdp8_emulator_in_c.git
cd pdp8_emulator_in_c
mkdir build
gcc build_system/main.c build_system/build_system.c -o builder
```

2. To compile the project use

```
./builder compile
```

To compile and run it use

```
./builder run
```

### Slow way

1. Clone the repository

```
git clone https://github.com/BudinoSurelySweet/pdp8_emulator_in_c.git
```

2. Enter the folder

```
cd pdp8_emulator_in_c
```

3. Create a `build` directory

```
mkdir build
```

4. Create the builder file (the build system used)

```
gcc build_system/main.c build_system/build_system.c -o builder
```

5. To compile the project run this command

```
./builder compile
```

To compile and run it instead:

```
./builder run
```

From now on you can use these two command to compile and/or run the project.

