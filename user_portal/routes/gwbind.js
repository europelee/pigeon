"use strict";

var express = require('express');
var router = express.Router();
var devstore = require('../../nodejs_lib/model/devstore');
var consts = require('../consts');
var redis_dbconf = require('../../nodejs_lib/dbconf/redis_dbconf');
var util   = require('../../nodejs_lib/lib/util');
var logstat = require('../base/logstat');
var async = require('async');

//invoked by any requests passed to the gateway router
router.use(function devLog(req, res, next) {
    console.log("enter gateway Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/', logstat.onLine);
router.get('/', function(req, res) {
    console.log('gateway get method');
});

//1.bind gateway to user 
router.post('/', logstat.onLine);
router.post('/', function(req, res) {
    console.log("gateway bind! for "+ req.session.user.name);

    //valid bodyString
    if (typeof(req.body.gwid) == "undefined" || null == req.body.gwid)
    {
        res.status(400);
        res.send('gw is invalid');
        return;
    }
    var gwList = null;
    async.waterfall([
           function(cb) {
               devstore.checkGWId(null, consts.GW_SET_KEY, req.body.gwid, function(err, ret1) {
                   if (ret1 == 1) {
                       cb(null);
                   }
                   else {
                       if (ret1 == 0 && err == null) {
                           err = req.body.gwid + ' does not exist!';
                       }
                       cb(err,null);
                   }
               }); 
           },
           
           function(cb) {

               devstore.checkGWBind(null, consts.ACC_KEY, consts.GW_SET_KEY, req.body.gwid, consts.GW_OWNERKEY, function(err, ret2) {
                   if (err == null && ret2 == false) {
                       cb(null);
                   }
                   else {
                       if (err == null && ret2 == true) {
                           cb(req.body.gwid+' already binded', null);
                       } 
                   }
               
               }); 
           },

           function(cb) {
               devstore.BindGateWay(null, consts.ACC_KEY, consts.GWBIND_POSTSETKEY, req.session.user.name, consts.GW_SET_KEY, req.body.gwid, consts.GW_OWNERKEY, function(err, ret3) {
               
                    cb(err, ret3);
               }); 
           } 
            
            ], function(err, ret) {
                var info = null;
                var rstatus = null;
                if (err == null) {
                    rstatus = 'success';
                    info = 'bind gateway '+req.body.gwid+' succ!';
                    console.log(info);
                }
                else {
                    rstatus = 'error';
                    info = 'bind gateway '+req.body.gwid+' fail:'+err;
                    console.log(info);
                }
                //render view
                devstore.getGWListBinded(null, consts.ACC_KEY, consts.GWBIND_POSTSETKEY, req.session.user.name, consts.GW_SET_KEY, consts.GW_IDFIELD, [consts.GW_OWNERKEY, consts.GW_MACKEY], function(err, result) {
                      gwList = result;
                      if (rstatus == 'success')
                      res.render('settings',{
                              success:info, title:consts.UP_TITLE, accname:req.session.user.name, setItem:"devices", gwlist:gwList
                      });
                      else

                      res.render('settings',{
                              error:info, title:consts.UP_TITLE, accname:req.session.user.name, setItem:"devices", gwlist:gwList
                      });
                      
                });
    });
});


module.exports = router;
