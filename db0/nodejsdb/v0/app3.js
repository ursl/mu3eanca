// ----------------------------------------------------------------------
// npm install nano
// npm install config
// [edit local.json with username, password, host, and port]
// node app3.js
// ----------------------------------------------------------------------
var config = require('config');

var dbhoststring = 'http://' + config.get('db.username') + ":" + config.get('db.password')
    + "@" + config.get('db.host') + ":" + config.get('db.port')
var nano = require('nano')(dbhoststring);

// clean up the database we created previously
nano.db.destroy('alice', function() {
    // create a new database
    nano.db.create('alice', function() {
        // specify the database we are going to use
        var alice = nano.use('alice');
        // and insert a document in it
        alice.insert({ crazy: true }, 'rabbit', function(err, body, header) {
            if (err) {
                console.log('[alice.insert] ', err.message);
                return;
            }
            console.log('you have inserted the rabbit.')
            console.log(body);
        });

        alice.insert({ crazy: true }, 'mouse', function(err, body, header) {
            if (err) {
                console.log('[alice.insert] ', err.message);
                return;
            }
            console.log('you have inserted the mouse.')
            console.log(body);
        });

        alice.insert({ crazy: true }, 'badger', function(err, body, header) {
            if (err) {
                console.log('[alice.insert] ', err.message);
                return;
            }
            console.log('you have inserted the badger.')
            console.log(body);
        });
        
    });
});
