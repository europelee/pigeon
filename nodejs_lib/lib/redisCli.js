"use strict";

var redis = require("redis");
var redisCli = redis.createClient();

redisCli.on('error', function (err) {
    console.log('redis error event - ' + client.host + ':' + client.port + ' - ' + err);
});

module.exports = redisCli;
