# Simple REST server
based on https://www.mongodb.com/languages/express-mongodb-rest-api-tutorial

# Setup
```
npm install express cors express-async-errors dotenv mongodb
```

# Run it
By default, it is running on port 80 (no good reason). If apache is running, it needs to be stopped, e.g.
```
sudo systemctl stop apache2.service
```

Port 80 also implies that you need to run it with sudo:
```
pc11740>sudo npm run start
pc11740>sudo /usr/local/bin/pm2 start index.mjs 
```

```
moor>curl pc11740/posts/rr/12
{"_id":"659bc705e3099e1b6409e7b1","run":"12","runStart":"2024-01-05 10:49:09","runEnd":"unset","runDescription":"unset","runOperators":"unset","nFrames":"-9999","beamMode":"-9999","beamCurrent":"-9999","magnetCurrent":"-9999","configurationKey":"unset"}

moor>curl pc11740/posts/gt/mcidealv5.1
{"_id":"659bc2c762958113d50ba132","gt":"mcidealv5.1","tags":["pixelalignment_mcidealv5.1","fibrealignment_mcidealv5.1","tilealignment_mcidealv5.1","mppcalignment_mcidealv5.1"]}

moor>curl pc11740/posts/tag/fibrealignment_mcidealv5.0
{"_id":"659bc2605e30961c4e057491","tag":"fibrealignment_mcidealv5.0","iovs":[1]}

moor>curl pc11740/posts/payload/tag_mppcalignment_mcidealv5.1_iov_1
{"_id":"659bc2d672ec635e5d0cad04","hash":"tag_mppcalignment_mcidealv5.1_iov_1","comment":"mcidealv5.1 MPPC detector initialization","schema":"define this","date":"2024-01-05 10:49:09","BLOB":"zvqt3gAAAAAAAAAAAAAAANMAkhp1Iz/AV8buw ...
```

