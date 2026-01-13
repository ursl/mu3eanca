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

## Running
First steps
```
node test1.js
```

## NGINX configuration for mu3edb0
```
   sudo cp /home/mu3e/mu3eanca/db2/nginx.conf /etc/nginx/sites-available/mu3edb0.conf
   sudo mkdir -p /etc/nginx/snippets
   sudo cp /home/mu3e/mu3eanca/db2/nginx-proxy-common.conf /etc/nginx/snippets/mu3edb0-proxy-common.conf

   sudo nginx -t

   sudo systemctl reload nginx

```