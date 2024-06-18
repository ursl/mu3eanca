# analysis scripts for the mu3epartsdb

## Setup
```
npm install --save nano
npm install --save config
```

## Prerequisites
Have a file `config/local.json` with
```
{
    "db" :
    {
        "username": "x",
        "password": "y",
        "host": "127.0.0.1",
        "port": "5984"
    }
}
```