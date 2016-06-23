"use strict";

var express = require('express');
var router = express.Router();
var devstore = require('../../nodejs_lib/model/devstore');
var consts = require('../consts');
var redis_dbconf = require('../../nodejs_lib/dbconf/redis_dbconf');
var util   = require('../../nodejs_lib/lib/util');
var uuid = require('node-uuid');

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
            var devid = uuid.v1();
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
router.get('/:gwid/devplug/:devplugid', function(req, res) {
    console.log("gateway device plugins download!");
    if (req.params.gwid == null || req.params.devplugid == null) {
        console.log("either gwid or devplugid is null");
        res.status = 400;
        res.send("gwid or devplugid is null");
        return;
    }

    if (req.query.version == null) {
        console.log("not version was set");
        res.status = 400;
        res.send("version not set");
        return;
    }

    console.log("gwid:"+req.params.gwid+" devplugid:"+req.params.devplugid+" version:"+req.query.version);

    //todo: check if gwid,devplugid,version is valid

    res.download("/root/"+req.params.devplugid+"-"+req.query.version, function(err){
        if (err) { 
            console.log("file download fail "+err);
        }
    });    
});
//3.require gateway agent upgraded(TBD)

module.exports = router;
