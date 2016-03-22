"use strict";

var cluster = require("cluster");
var LOG_TAG = '[master]';
function ClusterObserver(workNum) {
        console.log(LOG_TAG + " start master...");
        for (var i = 0; i < workNum; i++) {
                var wk = cluster.fork();
        }
}

module.exports = ClusterObserver;

ClusterObserver.prototype.startEventHandler = function startEventHandler() {
        cluster.on('fork', function (worker) {
                console.log(LOG_TAG + ' fork: worker' + worker.id);
        });

        cluster.on('online', function (worker) {
                console.log(LOG_TAG + ' online: worker' + worker.id);
        });

        cluster.on('listening', function (worker, address) {
                console.log(LOG_TAG + ' listening: worker' + worker.id + ',pid:' + worker.process.pid + ', Address:' + address.address + ":" + address.port);
        });

        cluster.on('disconnect', function (worker) {
                console.log(LOG_TAG + ' disconnect: worker' + worker.id);
        });

        cluster.on('exit', function (worker, code, signal) {
                console.log(LOG_TAG + ' exit worker' + worker.id + ' died');
        });
        
        var self = this;
        Object.keys(cluster.workers).forEach(function(id) {
                cluster.workers[id].on('message', function(msg){
                        console.log(LOG_TAG + ' message ' + msg);
                });
        });
};

ClusterObserver.prototype.sendMsg = function sendMsg(index, msg) {
    cluster.workers[index].send(msg);
};


