"use strict";

var express = require('express');
var router = express.Router();
var util   = require('../../nodejs_lib/lib/util');
var fs = require('fs');

//invoked by any requests passed to the reg router
router.use(function vodLog(req, res, next) {
    console.log("enter vod Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});


router.post('/', function(request, response){

    var dir = request.body.dir;
    var r = '<ul class="jqueryFileTree" style="display: none;">';
    try {
        r = '<ul class="jqueryFileTree" style="display: none;">';
        var files = fs.readdirSync(dir);
        files.forEach(function(f){
            var ff = dir + f;
            var stats = fs.statSync(ff)
            if (stats.isDirectory()) { 
                r += '<li class="directory collapsed"><a href="#" rel="' + ff  + '/">' + f + '</a></li>';
            } else {
                var e = f.split('.')[1];
                r += '<li class="file ext_' + e + '"><a href="#" rel='+ ff + '>' + f + '</a></li>';
            }
        });
        r += '</ul>';
    } catch(e) {
        r += 'Could not load directory: ' + dir + ' error:' + e;
        r += '</ul>';
    }
    response.send(r);
});

router.get('/',function(req,res){
    
    res.render('vod');
});

module.exports = router;
