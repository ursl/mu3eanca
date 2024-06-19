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

//console.log("call searchDoc");
//searchDoc();

//console.log("call OneDoc");
//listOneDoc();

//console.log("call listProducts");
//listProducts();


//console.log("call listPixelProducts");
listMyProducts('pix');


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
                if (doc.doc.tags.indexOf('pixel') > -1){
                    console.log(JSON.stringify(doc));
                }
            }
        })
    });
}

// ----------------------------------------------------------------------
async function listMyProducts(myprod) {
    var cnt = 0;
    var myprods = [];
    var alltypes = [];
    var alltags = [];
    const doclist = await alice.list({include_docs: true}).then((body)=>{
        // -- find all products containing myprod among the tags
        body.rows.forEach((doc) => {
            if (!alltypes.includes(doc.doc.type)) alltypes.push(doc.doc.type);
            if ((doc.doc.type == 'product') && doc.doc.tags) {
                const words = doc.doc.tags.split(',');
                words.forEach((word) => {
                    word = word.trim();
                    if (!alltags.includes(word)) {
                        alltags.push(word);
                    }
                });

                if (doc.doc.tags.includes(myprod)) {
                    myprods.push(doc.doc.pn);
                    // console.log(JSON.stringify(doc));
                    cnt++;
                }
            }
        })

        // console.log("*** found " + cnt + " products");
        // console.log("*** myprods = " + myprods);

        cnt = 0;
        // -- now find all items with pn among myprods
        body.rows.forEach((doc) => {
            if (doc.doc.type == 'attachment') return;
            if (doc.doc.type == 'attitem') return;
            if ((doc.doc.type == 'item') && myprods.includes(doc.doc.pn)) {
                console.log(JSON.stringify(doc));
                cnt++;
            }
        })

        console.log("*** myprods = " + myprods);
        // console.log("*** found " + cnt + " items");
        // console.log("*** all types: " + alltypes);
        // console.log("*** all tags: " + alltags);

    });
}
