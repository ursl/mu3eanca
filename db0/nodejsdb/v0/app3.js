// ----------------------------------------------------------------------
// https://gist.github.com/aerrity/fd393e5511106420fba0c9602cc05d35
// npm install nano
// npm install config
// [edit local.json with username, password, host, and port]
// node app3.js
// ----------------------------------------------------------------------




console.log('Server-side code running');

const express = require('express');

const app = express();

// serve files from the public directory
app.use(express.static('public'));

// start the express web server listening on 3000
app.listen(3000, () => {
  console.log('listening on 3000');
});

// serve the homepage
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// const http = require('http');
// var config = require('config');

// var dt = require('./myfirstmodule');


// var dbhoststring = 'http://' + config.get('db.username') + ":" + config.get('db.password')
//     + "@" + config.get('db.host') + ":" + config.get('db.port')
// var nano = require('nano')(dbhoststring);


// const hostname = '127.0.0.1';
// const port = 3000;

// const server = http.createServer((req, res) => {
//     res.statusCode = 200;
//     //    res.setHeader('Content-Type', 'text/plain');
//     res.setHeader('Content-Type', 'text/html');
//     res.write('Hello World!!!! date and time: ' + dt.myDateTime() + '\n');
//     res.write('req.url =  ' + req.url + '\n') ;
//     res.write('<h2>The Button Element</h2>');
//     res.write('<form action="" method="post">');
//     res.write('<button name="foo" value="send">Send</button>');
//     res.write('</form>');
//     res.end('Good bye');
// });


// server.listen(port, hostname, () => {
//     console.log(`LOG: Server running at http://${hostname}:${port}/`);
// });
