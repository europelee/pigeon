"use strict";

var crypto = require('crypto');
var util   = require('../lib/util');
var redisCli = require('../lib/redisCli');

exports.getAccount = function(reqKey, reqAccount, account_cb) {
    
    redisCli.sismember(reqKey, reqAccount, function(err, res) {
        account_cb(err, res);
    });
}

exports.addAccount = function(reqKey, reqAccount, account_cb) {
    redisCli.sadd(reqKey, reqAccount, function(err, res) {
        account_cb(err, res);
    });
}

exports.delAccount = function(reqKey, reqAccount, account_cb) {
    redisCli.srem(reqKey, reqAccount, function(err, res) {
        account_cb(err, res);
    });
}

exports.setAccount = function(reqAccountKey, reqField, reqValue, account_cb) {
    redisCli.hset(reqAccountKey, reqField, reqValue, function(err, res) {
        account_cb(err, res);
    });
}

//check password
exports.checkAccount = function(reqAccountKey, reqField, reqValue, account_cb) {
    redisCli.hget(reqAccountKey, reqField, function(err, ret) {
        if (null == err)
        {
            if (ret == reqValue)
            {
                console.log('password is right');
            }
            else
            {
                console.log('password is wrong');
                err = 'password is wrong!';
            }
        }
        account_cb(err, ret);
    });
}

//generate appkey/appsecret
exports.genKeySc = function(reqAccountKey, reqKeyField, reqSecField, acc, passwd, account_cb) {
    var timestamp = Date.now();
    var appKey = util.createmd5(acc+util.randomMix(8));
    var appSecret = util.createmd5(acc+timestamp+util.randomMix(8)+passwd);
    redisCli.hmset(reqAccountKey, reqKeyField, appKey, reqSecField, appSecret, function(err, ret) {
         account_cb(err, ret, appKey);
    }); 

}

//get appkey/appsecret
exports.getKeySc = function(reqAccountKey, reqKeyField, reqSecField, account_cb) {
    console.log(reqAccountKey+' '+reqKeyField+' '+reqSecField);
    redisCli.hmget(reqAccountKey, reqKeyField, reqSecField, function(err, ret) {
    console.log(ret);        
    account_cb(err, ret);
    }); 

}

exports.saveAKUsrMap = function(reqAppKeyPre, reqAppKey, reqUserName, account_cb) {
    redisCli.set(reqAppKeyPre+':'+reqAppKey, reqUserName, function(err, ret) {
        account_cb(err, ret);
    });
}

exports.checkAKUsrMap = function(reqAppKey, account_cb) {
    redisCli.get(reqAppKey, function(err, ret) {
        account_cb(err, ret);
    });
}

exports.listOwnDev = function(reqOwnDevKey, account_cb) {
    redisCli.smembers(reqOwnDevKey, function(err, ret){
        account_cb(err, ret);       
    });
}

exports.checkOwnDev = function(reqOwnDevKey, reqValue, account_cb) {
    redisCli.sismember(reqOwnDevKey, reqValue, function(err, ret){
        account_cb(err, ret);
    });
}

exports.addOwnDev = function(reqOwnDevKey, reqValue, account_cb) {
    redisCli.sadd(reqOwnDevKey, reqValue, function(err, ret){
        account_cb(err, ret);
    });
}

exports.delOwnDev = function(reqOwnDevKey, reqValue, account_cb) {
    redisCli.srem(reqOwnDevKey, reqValue, function(err, ret){
        account_cb(err, ret);
    });
}
