const API_KEY = 'YOUR_API_KEY';
const SECRET_KEY = 'YOUR_SECRET_KEY';
const YOUR_CHANNEL = 'YOUR_CHANNEL';
const YOUR_RESOURCE = 'YOUR_RESOURCE';

const express = require('express');
var app = express();
var server = app.listen(3000);
app.use(express.static('public'));

var socket = require('socket.io');
var io = socket(server);

// beebotte setting
var bbt = require('beebotte');
var bclient = new bbt.Connector({apiKey: API_KEY, secretKey: SECRET_KEY});

function read_data() {
    bclient.read({
        channel: YOUR_CHANNEL,
        resource: YOUR_RESOURCE,
        limit: 1
    }, function(err, res) {
        if (err) { throw err; }
        // console.log(res[0]);
        // console.log("temperature : " + res[0].data);
        io.emit('data', res[0].data);
    });
}


function new_connection(socket) {
    console.log('new connection: ' + socket.id);
    socket.on('read_data', read_data);
}

io.sockets.on('connection', new_connection);