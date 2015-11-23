"use strict";

var consts = require('../consts');
var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
    var name = null;
    if (req.session.user) {
        name = req.session.user.name;
        console.log('name:'+name);
    }
    else {
    
        res.render('index', { title: 'Express', accname:name });
        return;
    }

    var session = req.session;
    session.count = session.count || 0;
    var n = session.count++;
    console.log('welcome session id:'+session.id+' count:'+n);
    res.render('index', { title: consts.DP_TITLE, accname:name });
});

module.exports = router;
