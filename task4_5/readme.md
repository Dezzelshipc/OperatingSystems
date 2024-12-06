# Tepmerature server
1. Server that can recieve temperature data by serial port, work as html server to send html files or raw temperature data.
```
temp [serial port | COM4] [server ip | 127.0.0.1] [server port | 8080]
```

2. Temperature simulator:
```
sim [serial port | COM3]
```

3. Temperature log generator:
```
python sim_logs.py
```

## Requires
* Virtual serial port driver
    * For `Windows` e.g. `com0com`.
    * For `Linux` installing and running `socat.sh`.

## Server
Default server pages:
* `GET` /
    * `GET` /data?log=sec
        * `GET` /sec/raw
    * `GET` /data?log=hour
        * `GET` /hour/raw
    * `GET` /data?log=day
        * `GET` /day/raw
