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

Should see heartbeat then timeout after 10 counts
