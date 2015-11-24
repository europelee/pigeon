"use strict";

var redisCli = require('../lib/redisCli');

function checkHashSet(reqKey, devices_cb) {
    
    redisCli.exists(reqKey, function(err, res) {
        devices_cb(err, res);
        console.log('checkHashSet: '+'err: ' + err + ' res:'+ res);
 
    });
}

function delKey(reqKey, devices_cb) {
    redisCli.del(reqKey, function(err, res) {
        devices_cb(err, res);
        console.log('del: '+'err: ' + err + ' res:'+ res);
    });
}

exports.checkDevType  = function(reqKey, devices_cb) {
    checkHashSet(reqKey, devices_cb);
}

exports.listDeves = function(reqKey, devices_cb) {
    redisCli.smembers(reqKey, function (err, replies) {
        devices_cb(err, replies);
        console.log('err '+ err + ', ' + replies.length + " replies:");
        replies.forEach(function (reply, i) {
            console.log("    " + i + ": " + reply);
        });
    });

}

exports.listDev = function(reqKey, reqField, devices_cb) {
    
    redisCli.hget(reqKey, reqField, function(err, replies) {
        devices_cb(err, replies);
        console.log('err: ' + err + ' replies: ' + replies);
    });
}

exports.checkDevIsReg = function(reqSetKey, reqType, devices_cb) {
    redisCli.sismember(reqSetKey, reqType, function(err, ret){
        devices_cb(err, ret);
    });
}

exports.createDev = function(reqSetKey, reqDevTypeKey, reqType, reqField, reqValue, devices_cb) {
    
    redisCli.sadd(reqSetKey, reqType, function(err, res) {
        console.log('createDev: '+'err: ' + err + ' res:'+ res);
        if (err == null) {
            redisCli.hset(reqDevTypeKey, reqField, reqValue, function(err, ret){
            
                devices_cb(err, ret);
            });
        }
        else {
            devices_cb(err, res);
        }

    });
}

exports.updateDev = function(reqKey, reqField, reqValue, devices_cb) {
     redisCli.hset(reqKey, reqField, reqValue, function(err, res) {
        console.log('updateDev: '+'err: ' + err + ' res:'+ res);
        devices_cb(err, res);
    });
}   

exports.remDev = function(reqDevSetKey, reqDevInfoKey, reqDevMetaKey, reqType, devices_cb) {
    var multi = redisCli.multi();
    multi.srem(reqDevSetKey, reqType, redisCli.print);
    multi.del(reqDevInfoKey, redisCli.print);
    multi.del(reqDevMetaKey, redisCli.print);
    multi.exec(function(err, ret) {
        console.log(ret);    
        devices_cb(err, ret);
    });
}

exports.delDev = function(reqKey, reqField, devices_cb) {
    redisCli.hdel(reqKey, reqField, function(err, res) {
        console.log('delDev: ' + 'err: ' + err + ' res:' + res);
        devices_cb(err, res);
    });
}

exports.createDevMeta = function(reqKey, reqMeta, devices_cb) {
    var mSet = new Array();
    for(var x in reqMeta) {
         mSet.push(x,  reqMeta[x]);
    }
    console.log('createDevMeta multi prop: ' + mSet);
    redisCli.hmset(reqKey, mSet, function(err, res) {
        console.log('hmset: '+'err:'+err+' res:'+res+' typeof(res):'+typeof(res));
        devices_cb(err, res);
    });
}

exports.queryDevMeta = function(reqKey, devices_cb) {
    redisCli.hgetall(reqKey, function(err, res) {
        console.log('queryDevMeta: ' +'err:'+err+' res:'+res);
        
        for (var x in res) {
            console.log(x+':'+res[x]);
        }   
        devices_cb(err, res);
    });
}


exports.updateDevMeta = function(reqKey, reqMeta, devices_cb) {
    var mSet = new Array();
    for(var x in reqMeta) {
         mSet.push(x,  reqMeta[x]);
    }
    console.log('updateDevMeta multi prop: ' + mSet);
    redisCli.hmset(reqKey, mSet, function(err, res) {
        console.log('hmset: '+'err:'+err+' res:'+res+' typeof(res):'+typeof(res));
        devices_cb(err, res);
    });
}

exports.delDevMeta = function(reqKey, devices_cb) {
    delKey(reqKey, devices_cb);
}
