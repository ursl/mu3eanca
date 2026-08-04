# Simple REST server
based on https://www.mongodb.com/languages/express-mongodb-rest-api-tutorial

# IMPORTANT: rest vs restJSON sync reminder

There is now a separate standalone JSON-backed browser in `db1/restJSON`.

When you change CDB browser behavior in `db1/rest` (especially UI/API contracts), check whether
the corresponding files in `db1/restJSON` should be updated as well. In particular:

- `rest/public/cdb.html`  <->  `restJSON/public/cdb.html`
- `rest/routes/*` CDB endpoint behavior  <->  `restJSON/routes/cdb.mjs`
- startup/env assumptions in `rest/index.mjs`  <->  `restJSON/index.mjs`

Code duplication is intentional for now, so manual synchronization is expected.
Use `./sync-restjson.sh` (from `db1/rest`) to copy master files into `db1/restJSON`
and re-apply `restJSON`-specific overlays.

# Installation
Install `node/npm/pm2` the correct way by using [nvm](https://github.com/nvm-sh/nvm). Follow the instructions there. If `pm2` is missing, install it with `npm install pm2 -g` after the `npm` installation. 

# Setup
```
npm install express cors express-async-errors dotenv mongodb node-fetch
```

# Run it
By default, the REST interface seems to listen on port 80, though in reality it is on port 5050. If apache is running, that needs to be stopped, e.g. 
```
sudo systemctl stop apache2.service
```

Use nginx to redirect port 80 requests to port 5050. On opensuse (LEAP 15.5.), create the file `/etc/nginx/vhosts.d/pc11740.conf` with 

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

On **mu3edb0**, use the site config in `../../db2/nginx.conf` and
`../../db2/nginx-proxy-common.conf` (see `db2/README.md` for `sudo cp` / reload steps).
That file proxies `/cdb`, `/rdb`, `/detconfigs`, `/detcal`, `/relval` to port 5050.


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


# Detector configs (Mongo `detconfigs`; default via nginx on port 80)
moor>curl -v -F "file=@mask_408_1_12_DS_chip4.bin" -F "tag=5" http://mu3edb0.psi.ch/detconfigs/upload
moor>curl -O -J "http://mu3edb0.psi.ch/detconfigs/downloadTag?tag=5"
moor>curl -v -F "file=@..." -F "file=@..." -F "tag=4" http://mu3edb0.psi.ch/detconfigs/uploadMany
moor>curl -X POST -F "tag=j1" -F "filename=j1/root.json" -F "file=@j1/root.json" http://mu3edb0.psi.ch/detconfigs/uploadJSON
moor>curl http://mu3edb0.psi.ch/detconfigs/downloadJSON/j1 -o root.json
moor>curl -fsS "http://mu3edb0.psi.ch/detconfigs/detconfigTags"
moor>curl -X DELETE "http://mu3edb0.psi.ch/detconfigs/deleteDetconfigTag?tag=j1"
# Direct to Node (local dev): use localhost:5050 or CLI --port 5050
moor>curl -v -F "file=@mask_408_1_12_DS_chip4.bin" -F "tag=5" http://localhost:5050/detconfigs/upload

curl -X PUT -H "Content-Type: application/json" --data-binary @/Users/ursl/tmp/maskfiles/dqm101.json http://pc11740/rdb/addAttribute/7559

# Detector calibrations (Mongo `detcal` + GridFS `detcalBlobs`)
# Upload any file; indexed by (name, run). Download uses IOV-style lookup: most recent run <= RUN.
# Detector calibrations (Mongo `detcal` + GridFS; via nginx on port 80)
moor>curl -X POST -F "name=pixelpedestal" -F "run=7559" -F "file=@pedestal.json" http://mu3edb0.psi.ch/detcal/upload
moor>curl -OJ "http://mu3edb0.psi.ch/detcal/pixelpedestal/7580"
moor>curl "http://mu3edb0.psi.ch/detcal/pixelpedestal"
moor>curl -X DELETE "http://mu3edb0.psi.ch/detcal/pixelpedestal/7559"
# Direct to Node (local dev):
moor>curl -X POST -F "name=pixelpedestal" -F "run=7559" -F "file=@pedestal.json" http://localhost:5050/detcal/upload


```

