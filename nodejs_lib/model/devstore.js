"use strict";

var redisCli = require('../lib/redisCli');
var async = require('async');
var redis = require('redis');

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
    multi.srem(reqDevSetKey, reqType, redis.print);
    multi.del(reqDevInfoKey, redis.print);
    multi.del(reqDevMetaKey, redis.print);
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


exports.checkGWAuth = function(redis_dbconf, key, devices_cb) {
    //not support now
    devices_cb(null, 1);
}

exports.saveGWDevId = function(redis_dbconf, gwKey, gwId, macKey, macValue, devices_cb) {
    var mSet = new Array();
    mSet.push(macKey, macValue);

    var multi = redisCli.multi();
    multi.sadd(gwKey, gwId, redis.print);
    multi.hmset(gwKey+':'+gwId, mSet, redis.print);
    multi.exec(function(err, ret) {
        console.log(ret);    
        devices_cb(err, ret);
    });
}

exports.checkGWId = function(redis_dbconf, gwSetKey, gwId, cb) {
    redisCli.sismember(gwSetKey, gwId, function(err, ret) {
        cb(err, ret);       
    });
}

exports.checkGWBind = function(redis_dbconf, accKey, gwSetKey, gwId, ownerField, cb) {
    redisCli.hget(gwSetKey+':'+gwId, ownerField, function(err, ret) {
        if (err == null && ret != null) {
            //check user valid
            redisCli.sismember(accKey, ret, function(err, sret){
                if (err == null && sret == 1) {
                    cb(err, true);
                }
                else {
                    if (err == null) {
                        console.log('it is binded by one who is not exist');
                    }
                    cb(err, false);
                }


            });
        }
        else {
            if (err == null) {
                console.log('it is binded by noone');
            }
            cb(err, false);
        }
    });
}

exports.BindGateWay = function(redis_dbconf, accKey, gwBindPostKey, userName, gwSetKey, gwId, ownerField, devices_cb) {
        
    var mSet = new Array();
    mSet.push(ownerField, userName);
    var mIdSet = new Array();
    mIdSet.push(gwId);
    console.log('gwid:'+gwId+' typeof:'+typeof(gwId));    
    var multi = redisCli.multi();
    multi.sadd(accKey+':'+userName+':'+gwBindPostKey, mIdSet, redis.print);
    multi.hmset(gwSetKey+':'+gwId, mSet, redis.print);
    multi.exec(function(err, ret) {
        console.log(ret);    
        devices_cb(err, ret);
    });
}

exports.getGWListBinded = function(redis_dbconf, accKey, gwBindPostKey, userName, gwSetKey, gwIdField, gwInfoKeyList, devices_cb) {
        
    redisCli.smembers(accKey+':'+userName+':'+gwBindPostKey, function (err, replies) {
        if (err != null||typeof(replies)=='undefined'||replies == null) {
            devices_cb(err,null);
            return;
        }
        
        var gwArray = new Array();
        async.map(replies, function(item, callback) {
            redisCli.hmget(gwSetKey+':'+item, gwInfoKeyList, function(err, valueList) {
                var itemO = null;
                if (err == null) {
                    var o = '{'+"\""+gwIdField+"\""+':'+"\""+item+"\""+',';
                    for (var i in gwInfoKeyList) {
                        o = o + "\"" + gwInfoKeyList[i] +"\""+ ':'+"\""+valueList[i]+"\"";
                        if (i < gwInfoKeyList.length - 1) {
                            o = o + ',';
                        }
                    }
                    o = o + '}';
                    console.log('o is '+o);
                    itemO = JSON.parse(o);
                }
                callback(err, itemO);
            });
        }, function(err,results) {
            devices_cb(err, results);
        });        
        console.log('err '+ err + ', ' + replies.length + " replies:");
    });
}
