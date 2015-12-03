"use strict";

var express = require('express');
var router = express.Router();
var consts = require('../consts');
var logstat = require('../base/logstat');
var settings = require('../settings');

//invoked by any requests passed to the reg router
router.use(function devLog(req, res, next) {
    console.log("enter logout Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/', logstat.onLine);
router.get('/', function(req, res) {
    req.flash('success', 'logout succ');
    req.session.destroy();
    //req.session.user = null;
    res.clearCookie(settings.authcookiename, { path: '/' });
    res.redirect('/');
});

module.exports = router;
