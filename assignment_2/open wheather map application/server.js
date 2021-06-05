const API_KEY = 'YOUR_API_KEY';
const SECRET_KEY = 'YOUR_SECRET_KEY';
const OPEN_WEATHER_MAP_API_KEY = 'YOUR_API_KEY';
const YOUR_CHANNEL = 'YOUR_CHANNEL';
const YOUR_RESOURCE = 'YOUR_RESOURCE';

// beebotte setting
var bbt = require('beebotte');
var bclient = new bbt.Connector({apiKey: API_KEY, secretKey: SECRET_KEY});

// open weather map setting
var location = "Kanazawa-shi,jp";
var units = 'metric';
var URL = 'http://api.openweathermap.org/data/2.5/weather?q='+ location +'&units='+ units +'&appid='+ OPEN_WEATHER_MAP_API_KEY;

const express = require('express');
var app = express();
var server = app.listen(3000);
app.use(express.static('public'));

var socket = require('socket.io');
var io = socket(server);

var http = require('http');

function read_sensor_data() {
    bclient.read({
        channel: YOUR_CHANNEL,
        resource: YOUR_RESOURCE,
        limit: 1
    }, function(err, res) {
        if (err) { throw err; }
        // console.log("sensor data : " + res[0].data);
        sendData(res[0].data, true);
    });
}

function read_owm_data() {
    http.get(URL, function(res) {
        var body = '';
        res.setEncoding('utf8');
        res.on('data', function(chunk) {
            body += chunk;
        });
        res.on('data', function(chunk) {
            res = JSON.parse(body);
            // console.log(res);
            sendData(res, false);
        });
    }).on('error', function(e) {
        console.log(e.message);
    });
}

function sendData(data, isSensor) {
    if (isSensor) {
        io.emit("temp", data);
    }
    else {
        var send_data = {
            "temp": data.main.temp,
            "main": data.weather[0].main,
            "clouds": data.clouds.all
        }
        io.emit("open_wether_map", send_data);
    }
}

function new_connection(socket) {
    console.log('new connection: ' + socket.id);

    read_sensor_data();
    read_owm_data();
}

io.sockets.on('connection', new_connection);