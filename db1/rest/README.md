# Simple REST server
based on https://www.mongodb.com/languages/express-mongodb-rest-api-tutorial

# Installation
Install `node/npm/pm2` the correct way by using (https://github.com/nvm-sh/nvm)[nvm].

# Setup
```
npm install express cors express-async-errors dotenv mongodb
```

# Run it
By default, the REST interface seems to listen on port 80, though in reality it is on port 5050. If apache is running, that needs to be stopped, e.g. 
```
sudo systemctl stop apache2.service
```

Use nginx to redirect port 80 requests to port 5050. On opensuse (LEAP 15.5.), create the file `/etc/nginx/vhosts.d/pc11740.conf`

```
server {
      listen 80;
      server_name pc11740.psi.ch;
      location / {
          proxy_pass http://localhost:5050;
          proxy_http_version 1.1;
          proxy_set_header Upgrade $http_upgrade;
          proxy_set_header Connection ’upgrade’;
          proxy_set_header Host $host;
          proxy_cache_bypass $http_upgrade;
      }
}
```
where, of course, `pc11740.psi.ch` should be replaced with whatever machine you are running the server on. Test the configuration with `sudo nginx -t` and then `sudo service nginx restart`. 


```
pc11740>npm run start
pc11740>pm2 start index.mjs 
```

The following works within PSI's network. pc11740.psi.ch is not reachable from outside (sigh)!


Use/look at it in a browser:
```
http://pc11740.psi.ch/rdb

http://pc11740.psi.ch/cdb/findOne/globaltags/mcidealv5.1
http://pc11740.psi.ch/cdb/findOne/tags/fibrealignment_mcidealv5.0
http://pc11740.psi.ch/cdb/findOne/payloads/tag_mppcalignment_mcidealv5.1_iov_1

```

Or use curl for direct transfer:

```
moor>curl http://pc11740.psi.ch/cdb/findOne/globaltags/mcidealv5.1
{"_id":"659bc2c762958113d50ba132","gt":"mcidealv5.1","tags":["pixelalignment_mcidealv5.1","fibrealignment_mcidealv5.1","tilealignment_mcidealv5.1","mppcalignment_mcidealv5.1"]}

moor>curl http://pc11740.psi.ch/cdb/findOne/tags/fibrealignment_mcidealv5.0
{"_id":"659bc2605e30961c4e057491","tag":"fibrealignment_mcidealv5.0","iovs":[1]}

moor>curl http://pc11740.psi.ch/cdb/findOne/payloads/tag_mppcalignment_mcidealv5.1_iov_1
{"_id":"659bc2d672ec635e5d0cad04","hash":"tag_mppcalignment_mcidealv5.1_iov_1","comment":"mcidealv5.1 MPPC detector initialization","schema":"define this","date":"2024-01-05 10:49:09","BLOB":"zvqt3gAAAAAAAAAAAAAAANMAkhp1Iz/AV8buw ...
```

