"use strict";

var express = require('express');
var router = express.Router();
var devstore = require('../../nodejs_lib/model/devstore');
var consts = require('../consts');
var redis_dbconf = require('../../nodejs_lib/dbconf/redis_dbconf');
var util   = require('../../nodejs_lib/lib/util');

//invoked by any requests passed to the gateway router
router.use(function devLog(req, res, next) {
    console.log("enter gateway Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

//1.reg gateway to piegon
router.post('/', function(req, res) {
    console.log("gateway reg!");

    //valid bodyString
    if (typeof(req.body.macaddr) == "undefined" || null == req.body.macaddr)
    {
        res.status(400);
        res.send('macaddr is invalid');
        return;
    }

    //check gateway authkey
    var key =  req.get(consts.GW_AUTHKEY_HTTPHEADER);
    devstore.checkGWAuth(redis_dbconf, key, function(err, ret) {
        if (err == null && ret == 1) {
            //generate devid
            var timestamp = Date.now();
            var devid = util.createmd5(req.body.macaddr+timestamp+util.randomMix(8));
            devstore.saveGWDevId(redis_dbconf, consts.GW_SET_KEY, devid, consts.GW_MAC_KEY, req.body.macaddr, function(err, sret) {
                if (err == null) {
                    var rspInfo = new Object();
                    rspInfo['gwId'] = devid;
                    res.type('json');
                    res.status(201);
                    res.send(rspInfo);                    
                }
                else {
                    res.status(500);
                    res.send(err);
                }       
            });
        }
        else {
            res.sendStatus(403);
        }  
    });   
});

//2.require device plugin download(TBD)

//3.require gateway agent upgraded(TBD)

module.exports = router;
