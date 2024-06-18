// ----------------------------------------------------------------------
// npm install nano
// npm install config
// [edit local.json with username, password, host, and port]
// node test1.js
// ----------------------------------------------------------------------
var config = require('config');
var dbhoststring = 'http://' + config.get('db.username') + ":" + config.get('db.password')
    + "@" + config.get('db.host') + ":" + config.get('db.port')
var nano = require('nano')(dbhoststring);


const alice = nano.db.use('mu3epartdb');

console.log("call searchDoc");
searchDoc();

console.log("call OneDoc");
listOneDoc();

console.log("call listProducts");
listProducts();


// ----------------------------------------------------------------------
async function searchDoc() {
    const doc = alice.get('e592791ea82676939311b7b3d904f4c7', { revs_info: true });
    console.log(JSON.stringify(doc));
}


// ----------------------------------------------------------------------
async function listOneDoc() {
    const doclist = await alice.list({include_docs: true}).then((body)=>{
        body.rows.forEach((doc) => {
            if (doc.id == 'e592791ea82676939311b7b3d904f4c7') {
                console.log(doc);
            }
        })
    });
}


// ----------------------------------------------------------------------
async function listProducts() {
    const doclist = await alice.list({include_docs: true}).then((body)=>{
        body.rows.forEach((doc) => {
            if (doc.doc.type == 'product') {
                console.log(JSON.stringify(doc));
            }
        })
    });
}
