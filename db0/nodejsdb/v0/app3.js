// ----------------------------------------------------------------------
// npm install nano
// npm install config
// [edit local.json with username, password, host, and port]
// node app3.js
// ----------------------------------------------------------------------
const http = require('http');
var config = require('config');

var dt = require('./myfirstmodule');


var dbhoststring = 'http://' + config.get('db.username') + ":" + config.get('db.password')
    + "@" + config.get('db.host') + ":" + config.get('db.port')
var nano = require('nano')(dbhoststring);


const hostname = '127.0.0.1';
const port = 3000;

const server = http.createServer((req, res) => {
    res.statusCode = 200;
    res.setHeader('Content-Type', 'text/plain');
    res.end('Hello World!!!! date and time: '+ dt.myDateTime());
});

server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
});
