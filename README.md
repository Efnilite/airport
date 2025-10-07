# Quercus Airport Baggage Handling Software

Welcome to the digital simulation code of Quercus Airport.

# Testing setup

Before being able to run your tests, you should run the following commands.

```sh
sudo sysctl -w fs.mqueue.msg_max=100
sudo sysctl -p
```

Since mqueue size is set at 10 every restart, you need to update this every time.
If you get an `Invalid argument` error when a queue is attempted to be opened, you forgot this step.