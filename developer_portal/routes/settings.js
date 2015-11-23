"use strict"

var express = require('express');
var router = express.Router();
var consts = require('../consts');
var account = require('../model/account');
var User    = require('../model/user');
var logstat = require('../base/logstat');
var async = require('async');
var devstore = require('../model/devstore');

//invoked by any requests passed to the reg router
router.use(function devLog(req, res, next) {
    console.log("enter settings  Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/settings', logstat.onLine);
router.get('/settings',function(req,res){
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }
    
    account.getKeySc(consts.ACC_KEY+':'+name+consts.ACC_INFO, consts.ACC_APPKEY_FIELD,
        consts.ACC_APPSEC_FIELD, function(err, ret) {
            if (err == null) {
            res.render('settings',{
                    title:"account settings", accname:name,appkey: ret[0], appsecret:ret[1]
            });
            }
        });
});

router.get('/settings/profile', logstat.onLine);
router.get('/settings/profile', function(req, res) {
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }

    account.getKeySc(consts.ACC_KEY+':'+name+consts.ACC_INFO, consts.ACC_APPKEY_FIELD,
        consts.ACC_APPSEC_FIELD, function(err, ret) {
            if (err == null) {
            res.render('settings',{
                    title:"account settings", accname:name,appkey: ret[0], appsecret:ret[1]
            });
            }
        });

});

router.get('/settings/devices', logstat.onLine);
router.get('/settings/devices', function(req, res) {
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }

    account.listOwnDev(consts.ACC_KEY+':'+name+consts.ACC_DEVTYPE, function(err, ret){
        if (err==null) {
            async.map(ret, function(item, callback) {
                console.log('get info from:'+item);
                async.parallel([
                    function(cb) {
                    
                        devstore.listDev(consts.DEV_TYPE_KEY+':'+item+consts.DEV_TYPE_INFO, consts.DEV_TYPE_INFO, function(err, retInfo) {
                            cb(err, retInfo);
                        });                                    
                    }, function(cb) {
                        
                        devstore.queryDevMeta(consts.DEV_TYPE_KEY+':'+item+consts.DEV_META, function(err, retMeta) {
                            cb(err, retMeta);           
                        });
                    }                
                ], function (err, result) {
                        var itemSet = new Object();
                        itemSet[consts.DEV_TINFO_NLIST.d] = item;
                        var dinfo = JSON.parse(result[0]);
                        if (typeof(dinfo[consts.DEV_TINFO_NLIST.m]) != 'undefined')
                            itemSet[consts.DEV_TINFO_NLIST.m] = dinfo[consts.DEV_TINFO_NLIST.m]; 
                        else
                            itemSet[consts.DEV_TINFO_NLIST.m] = null; 
                        
                        if (typeof(dinfo[consts.DEV_TINFO_NLIST.v]) != 'undefined')
                            itemSet[consts.DEV_TINFO_NLIST.v] = dinfo[consts.DEV_TINFO_NLIST.v]; 
                        else
                            itemSet[consts.DEV_TINFO_NLIST.v] = null;

                        if (typeof(dinfo[consts.DEV_TINFO_NLIST.de]) != 'undefined')
                            itemSet[consts.DEV_TINFO_NLIST.de] = dinfo[consts.DEV_TINFO_NLIST.de]; 
                        else
                            itemSet[consts.DEV_TINFO_NLIST.de] = null; 
                        itemSet[consts.DEV_TINFO_NLIST.me] = JSON.stringify(result[1]);
                        callback(null, itemSet); 
                });
            }, function(err, results) {
                console.log(results);

                res.render('settings',{
                        title:"account settings", accname:name, labellist:consts.DEV_TINFO_NLIST, devtypelist:results
                });
            });
        }
    });
    


});

module.exports = router;
