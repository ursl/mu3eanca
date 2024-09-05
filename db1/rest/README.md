# Simple REST server
based on https://www.mongodb.com/languages/express-mongodb-rest-api-tutorial

# Setup
```
npm install express cors express-async-errors dotenv mongodb
```

# Run it
By default, it is running on port 80 (no good reason). If apache is running, that needs to be stopped, e.g.
```
sudo systemctl stop apache2.service
```

Port 80 also implies that you need to run it with sudo:
```
pc11740>sudo npm run start
pc11740>sudo /usr/local/bin/pm2 start index.mjs 
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

