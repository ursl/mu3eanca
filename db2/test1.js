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

const pars = ['pix', 'sci', 'daq'];
listMyProducts(pars);


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
    var myprods = [];
    var alltypes = [];
    var alltags = [];
    var output = "[";

    const doclist = await alice.list({include_docs: true}).then((body)=>{
        // -- find all products containing myprod[i] among the tags
        body.rows.forEach((doc) => {
            if (!alltypes.includes(doc.doc.type)) alltypes.push(doc.doc.type);
            if (typeof doc.doc.type == "undefined") return;
            if (typeof doc.doc.tags == "undefined") return;
            if (!doc.doc.type.localeCompare("product")) {
                var lctags = [];
                const words = doc.doc.tags.split(',');
                words.forEach((word) => {
                    word = word.trim();
                    word = word.toLowerCase();
                    lctags.push(word);
                    if (!alltags.includes(word)) {
                        alltags.push(word);
                    }
                });

                for (i=0; i < myprod.length; ++i) {
                    var lcword  = myprod[i].toLowerCase();
                    for (j = 0; j < lctags.length; ++j) {
                        var lctag   = lctags[j].toLowerCase();
                        if (!lctag.indexOf(lcword)) {
                            myprods.push(doc.doc.pn);
                        }
                    }
                };
            }
        })

        if (0) {
            console.log("myprods = " + myprods);
            console.log(myprods.length);
            return;
        }

        let maxcnt = body.rows.length;
        // -- now find all items with pn among myprods
        for (var i = 0; i < maxcnt; i++) {
            var doc = body.rows[i];
            if (typeof doc.doc.type == "undefined") continue;
            if (!doc.doc.type.localeCompare("attitem")) continue;
            if (!doc.doc.type.localeCompare("productdoc")) continue;
            if (!doc.doc.type.localeCompare("item")) {
                if (myprods.includes(doc.doc.pn)) {
                    output += JSON.stringify(doc) + ", \n";
                }
            }
            if (!doc.doc.type.localeCompare("lot")) {
                if (myprods.includes(doc.doc.pn)) {
                    output += JSON.stringify(doc) + ", \n";
                }
            }
            if (!doc.doc.type.localeCompare("product")) {
                if (myprods.includes(doc.doc.pn)) {
                    output += JSON.stringify(doc) + ", \n";
                }
            }
        }

        // -- remove trailing ,
        output = output.replace(/,\s*$/, "");

        output += "]";

        console.log(output);

        // console.log("*** myprods = " + myprods);
        // console.log("*** found " + cnt + " items");
        // console.log("*** all types: " + alltypes);
        // console.log("*** all tags: " + alltags);

    });


}
