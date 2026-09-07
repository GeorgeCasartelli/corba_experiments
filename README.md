To run, activate CORBA:

```bash
conda activate corba
```

Run python server:

```bash
python3 server.py
```

In different terminal, compile and run client.cpp:

```bash
g++ -o client client.cpp testSK.cc -lomniORB4 -lomnithread
./client
```

Once running heartbeat should register. 

To check timeouts, do 

```bash
ps aux | grep server.py
kill -STOP <pid>
```

And then after 3 ticks you will see the "greet" command is run and the timeout is different.


