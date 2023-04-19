// ----------------------------------------------------------------------
// https://gist.github.com/aerrity/fd393e5511106420fba0c9602cc05d35
// npm install nano
// npm install config
// [edit local.json with username, password, host, and port]
// node app3.js
// ----------------------------------------------------------------------

console.log('Server-side code running');


var config = require('config');
const express = require('express');
const app = express();

// serve files from the public directory
app.use(express.static('public'));

var dbhoststring = 'http://' + config.get('db.username') + ":" + config.get('db.password')
    + "@" + config.get('db.host') + ":" + config.get('db.port')
var nano = require('nano')(dbhoststring);

var db_name = "counter";
var db = nano.use(db_name);


var counter = 0;
var rev;

// -- start the express web server listening on 3000
app.listen(3000, () => {
  console.log('listening on 3000');
});

// -- serve homepage
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});


// -- add a document to the DB collection recording the click event with counter
app.post('/clicked', (req, res) => {
    db.get('cnt', function(err, doc) {
        if (err) {
            console.log(err.message);
            insert_doc({_id: "cnt", nano: true, clicktime: "nada", counter: 0}, 0);
            res.sendStatus(201);
        } else {
            counter = doc.counter;
            rev = doc._rev;
            counter++;
            const click = Date();
            const text = click + " counter ->" + counter + "<- // rev ->" + rev + "<-";
            console.log(text);
            
            insert_doc({_id: "cnt", _rev: rev, nano: true, clicktime: click, counter: counter}, 0);
            console.log('click added to db');
            res.send({rescnt: counter});
//            res.append('rescnt', counter);
        }
    });
});



// ----------------------------------------------------------------------
function insert_doc(doc, tried) {
    db.insert(doc,
              function (error,http_body,http_headers) {
                  if(error) {
                      if(error.message === 'no_db_file' && tried < 1) {
                          // create database and retry
                          return nano.db.create(db_name, function () {
                              insert_doc(doc, tried+1);
                          });
                      }
                      else { return console.log(error); }
                  }
              });
}

// ----------------------------------------------------------------------
// get the click data from the database
app.get('/clicks', (req, res) => {
    db.get('cnt', function(err, doc) {
        if (err) {
            return console.log(err);
        } else {
            console.log("read something " + "cnt = " + doc.counter);
            return doc.counter;
        }
    });
});
