
var http = require('http'); // 1 - Import Node.js core module

console.log('Node.js web server at port 80 is running..')

// ----------------------------------------------------------------------
var server = http.createServer(function (req, res) {   // 2 - creating server
    
    if (req.url == '/') { //check the URL of the current request
        res.writeHead(200, { 'Content-Type': 'text/html' }); 
        res.write('<html><body><p>This is home Page.</p></body></html>');
        res.end();
    }  else if (req.url == "/student") {
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.write('<html><body><p>This is student Page.</p></body></html>\n');
        res.end();
    } else if (req.url == "/admin") {
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.write('<html><body><p>This is admin Page.</p></body></html>');
        res.end();
    }
    else
        res.end('Invalid Request!');
});
server.listen(80); //3 - listen for any incoming requests


// ----------------------------------------------------------------------
async function findOneGlobalTagByName(client, nameOfGlobalTag) {
    const result = await client.db("mu3e").collection("globaltags").findOne({ gt: nameOfGlobalTag });

    if (result) {
        console.log(`Found a global tag in the collection with the name '${nameOfGlobalTag}':`);
        console.log(result);
    } else {
        console.log(`No listings found with the name '${nameOfGlobalTag}'`);
    }
}


