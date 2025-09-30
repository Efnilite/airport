# Quercus Airport Baggage Handling Software

Welcome to the digital simulation code of Quercus Airport.

# Testing setup

```sh
sudo sysctl -w fs.mqueue.msg_max=100
sudo sysctl -p
```

Since mqueue size is set at 10 every restart