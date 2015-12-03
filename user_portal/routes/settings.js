"use strict"

var express = require('express');
var router = express.Router();
var consts = require('../consts');
var account = require('../../nodejs_lib/model/account');
var User    = require('../../nodejs_lib/model/user');
var logstat = require('../base/logstat');
var async = require('async');
var devstore = require('../../nodejs_lib/model/devstore');

//invoked by any requests passed to the reg router
router.use(function devLog(req, res, next) {
    console.log("enter settings  Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/', logstat.onLine);
router.get('/',function(req,res){
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }
    
    res.render('settings',{
            title:consts.UP_TITLE, accname:name, setItem:'profile'
    });
});

router.get('/profile', logstat.onLine);
router.get('/profile', function(req, res) {
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }

    res.render('settings',{
            title:consts.UP_TITLE, accname:name, setItem:consts.PROFILE_ITEM
    });
});

router.get('/devices', logstat.onLine);
router.get('/devices', function(req, res) {
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
    }
    var gwList = null;
    devstore.getGWListBinded(null, consts.ACC_KEY, consts.GWBIND_POSTSETKEY, req.session.user.name, consts.GW_SET_KEY, consts.GW_IDFIELD, [consts.GW_OWNERKEY, consts.GW_MACKEY], function(err, result) {
                      gwList = result;
                      res.render('settings',{
                             title:consts.UP_TITLE, accname:req.session.user.name, setItem:"devices", gwlist:gwList
                      });

                });
});

module.exports = router;
