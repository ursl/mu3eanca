const path = require('path');
const Shark = require('../models/sharks');

exports.index = function (req, res) {
    res.sendFile(path.resolve('views/sharks.html'));
};


exports.create = function (req, res) {
    var newShark = new Shark(req.body);
    console.log(req.body);
    newShark.save()
        .then(function () {
            res.redirect('/sharks/getshark');
        })
        .catch(function(err) {
            res.status(400).send('Unable to save shark to database');
        });
}


exports.list = function(req, res) {
    Shark.find({})
        .then(function(sharks) {
            res.render('getshark', {
                sharks: sharks
            })
        })
        .catch(function(err) {
            res.send(500, err);
        });
};
