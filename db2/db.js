const mongoose = require('mongoose');

//const MONGO_USERNAME = 'ursl';
//const MONGO_PASSWORD = 'your_password';
const MONGO_HOSTNAME = '127.0.0.1';
const MONGO_PORT = '27017';
const MONGO_DB = 'sharks';

//const url = `mongodb://${MONGO_HOSTNAME}:${MONGO_PORT}/${MONGO_DB}`;

const url = `mongodb://pc11740:27017`;

mongoose.connect(url, {useNewUrlParser: true});
