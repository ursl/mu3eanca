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
    counter++;
    const click = {clickTime: new Date()};
    console.log(click);
    insert_doc({nano: true, clicktime: click, counter: counter}, 0);
    console.log('click added to db');
});



// ----------------------------------------------------------------------
function insert_doc(doc, tried) {
    db.insert(doc,
              function (error,http_body,http_headers) {
                  if(error) {
                      if(error.message === 'no_db_file'  && tried < 1) {
                          // create database and retry
                          return nano.db.create(db_name, function () {
                              insert_doc(doc, tried+1);
                          });
                      }
                      else { return console.log(error); }
                  }
                  console.log(http_body);
              });
}

// ----------------------------------------------------------------------
// -- get the click data from the database
app.get('/counter', (req, res) => {
    console.log("read SOMETHING SOMEHOW from db");
    
    // db.collection('clicks').find().toArray((err, result) => {
    //     if (err) return console.log(err);
    //     res.send(result);
    // });
});
