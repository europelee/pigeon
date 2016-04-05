"use strict";

/**
 * because in zeromq The REQ-REP socket pair is in lockstep
 * so a next req would not be sended until rep being recved correspend to the last req.
 * so REQ-REP not good, should use zmq 'The Asynchronous Client/Server Pattern' better.
 * now just only use dealer-router for simple.
 */
var zmq = require('zmq');
var Command = require('./base/command');
var os = require('os');
var SvcDis = require('./DevCtlSvcDiscover');
var LOG_TAG = 'UPBackEnd';
var MAX_QUEUE_SIZE = 1024*1024;

function UPBackEnd(upId, commType, zkAddrList, nodePath) {
    console.log('UPBackEnd upId:'+upId+' start ... ...');
    this.upId = upId;
    this.commandQueue = new Map();
    this.socket = zmq.socket(commType);
    this.socket.identity = os.hostname()+'-'+upId;
    this.commAddr = null;
    this.seq = 0;
    this.svcDis = new SvcDis(zkAddrList, nodePath);
}

module.exports = UPBackEnd;

UPBackEnd.prototype.start = function start() {
   var self = this;
   self.svcDis.start(function(info) {
       try {
       var svcInfo = JSON.parse(info);
       self.commAddr = svcInfo.addr;
       self.startHelper();
       }
       catch (e) {
        console.log(e.stack);
       }
   }); 
}

UPBackEnd.prototype.startHelper = function startHelper(commAddr) {
    
    var self = this;
    self.socket.on('connect', function(fd, ep) {console.log('connect, endpoint:', ep);});
    self.socket.on('connect_delay', function(fd, ep) {console.log('connect_delay, endpoint:', ep);});
    self.socket.on('connect_retry', function(fd, ep) {console.log('connect_retry, endpoint:', ep);});
    self.socket.on('listen', function(fd, ep) {console.log('listen, endpoint:', ep);});
    self.socket.on('bind_error', function(fd, ep) {console.log('bind_error, endpoint:', ep);});
    self.socket.on('accept', function(fd, ep) {console.log('accept, endpoint:', ep);});
    self.socket.on('accept_error', function(fd, ep) {console.log('accept_error, endpoint:', ep);});
    self.socket.on('close', function(fd, ep) {console.log('close, endpoint:', ep);});
    self.socket.on('close_error', function(fd, ep) {console.log('close_error, endpoint:', ep);});
    self.socket.on('disconnect', function(fd, ep) {console.log('disconnect, endpoint:', ep);});

    self.socket.on('monitor_error', function(err) {
            console.log('Error in monitoring: %s, will restart monitoring in 5 seconds', err);
            setTimeout(function() { self.socket.monitor(500, 0); }, 5000);
    });

    self.socket.on('message', function(data) {
            console.log(LOG_TAG+' '+self.upId+' : recv data ' + data.toString());
            var rep = JSON.parse(data.toString());
            if (rep != null && rep.seq != undefined && rep.seq != null) {
                    var cmdObj = self.commandQueue.get(rep.seq);
                    console.log("cmdobj:"+cmdObj);
                    if (cmdObj == null) {
                        console.log("cmdObj is null");
                        return;
                    }
                    cmdObj.callback(null, rep.content);
                    var bret = self.commandQueue.delete(rep.seq);
                    if (false == bret)
                        console.log('error: fail delete key '+rep.seq);
            }
            else {
                    console.log('data recved invalid');
                    return;
            }

    });
    self.socket.monitor(500, 0);
    self.socket.connect(self.commAddr);
};

UPBackEnd.prototype.sendMsg = function sendMsg(dest, msg, callback) {
    var self = this;
    
    console.log(LOG_TAG+' '+self.upId+' sendmsg '+msg);
    if (MAX_QUEUE_SIZE <= self.commandQueue.size) {
        console.log('commandQueue is full!');
        return;
    }

    self.seq = self.seq%MAX_QUEUE_SIZE;    
    
    //for exception
    if (self.commandQueue.has(self.seq)==true) {
        console.log('error: command seq '+self.seq+' already exist!');
        return;
    }

    var commandObj = new Command(self.seq, dest, msg, callback); //msg stored for future
    self.commandQueue.set(self.seq, commandObj);
    
    var msgO = new Object();
    msgO.seq = self.seq;
    msgO.dest = dest;
    msgO.content = msg;
    self.seq++;
    var streamMsg = JSON.stringify(msgO);
    console.log('req type:'+typeof(streamMsg)+' '+streamMsg);
    self.socket.send(streamMsg);
};
