import { MongoClient } from "mongodb";

const connectionString =  "mongodb://127.0.0.1:27017";
//const connectionString =  "mongodb://127.0.0.1:27017";
//const connectionString = process.env.ATLAS_URI || "mongodb://127.0.0.1:27017";
//const connectionString = process.env.ATLAS_URI || "mongodb://pc11740.psi.ch:27017";

const client = new MongoClient(connectionString);

let conn;
try {
  conn = await client.connect();
} catch(e) {
  console.error(e);
}

let db = conn.db("mu3e");

export default db;
