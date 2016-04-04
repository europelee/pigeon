"use strict";

var zookeeper = require('node-zookeeper-client');
var Event = require('node-zookeeper-client/lib/Event.js');
var LOG_TAG = "DevCtlSvcDiscovery";

function DevCtlSvcDiscovery(zkAddrList, svcPath) {
    this.zkAddrList = zkAddrList;
    this.svcPath = svcPath;
    this.svcCallBack = null;
}


module.exports = DevCtlSvcDiscovery;

DevCtlSvcDiscovery.prototype.listChildren = function listChildren(client, path) {
    var self = this;
    client.getChildren(
            path,
            function (event) {
                console.log('Got watcher event: %s', event);
                self.listChildren(client, path);
            },
            function (error, children, stat) {
                if (error) {
                    console.log(
                        'Failed to list children of %s due to: %s.',
                        path,
                        error
                        );
                    return;
                }

                console.log('Children of %s are: %j.', path, children);
                for (var i=0; i < children.length; ++i) {
                    self.getNodeData(client, path+'/'+children[i]);
                }
            }
    );
}

DevCtlSvcDiscovery.prototype.getNodeData = function getNodeData(client, path) {
    var self = this;    
    client.getData(
            path,
            function (event) {
                console.log('Got event: %s. %s %s', event, event.getName(), event.getPath());
                if (event.getType() == Event.NODE_DATA_CHANGED) {
                    self.getNodeData(client, path);
                }   
            },  
            function (error, data, stat) {
                if (error) {
                    console.log(error.stack);
                    return;
                }   

                console.log('Got data: %s', data.toString('utf8'));
                self.svcCallBack(data.toString('utf8'));
            }   
            );  
}
DevCtlSvcDiscovery.prototype.start = function start(cb) {

    var self = this;
    self.svcCallBack = cb;
    self.client = zookeeper.createClient(self.zkAddrList, {
        sessionTimeout: 30000,
        spinDelay : 1000,
        retries : 0
    });

    self.client.on('disconnected', function () {                                                                                        
        console.log('Client state is changed to disconnected.');
    });

    self.client.on('connected', function () {
        console.log('Connected to ZooKeeper.');
        self.listChildren(self.client, self.svcPath);
    });

    self.client.connect();
}
