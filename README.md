# Quercus Airport Baggage Handling Software

Welcome to the digital simulation code of Quercus Airport.

To avoid copyright issues, the source code provided by the course is not provided in this repo.
The changes in this code are minor, as it only includes changing the main loop setup when the `Q_TEST` flag is designated.

# Installing dependencies

To install the dependency packages, run the following command. 

```sh
sudo apt update && sudo apt upgrade && sudo apt install libsdl2-dev libsdl2-ttf-dev make cmake
```

Then, in the project directory, run the following to initialize CMake.

```sh
cmake .
```

If you are missing dependencies, you should get an error. If you did everything correctly,
there should be a `CMakeCache.txt` file.

# Before running tests

Before being able to run your tests, you should run the following commands.

```sh
sudo sysctl -w fs.mqueue.msg_max=100
sudo sysctl -p
```

Since the message queue size is set at 10 every restart, you need to update this every time.
If you get an `Invalid argument` error when a queue is attempted to be opened, you forgot this step.

# Running tests

To build a target, use `make <target>` in the project directory. Then, run the appropriate executable.
Below is a list of common commands.

- `make test && ./target/test`. Use this to test the general system with GUI.
- `make pi_single && ./target/pi_single`. Use this to test the Pi on its own.
- `make pico_gate_single && ./target/pico_gate_single`. Use this to test the Pico gate executable on its own.
