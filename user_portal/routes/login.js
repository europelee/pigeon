"use strict";

var express = require('express');
var router = express.Router();
var async = require('async');
var consts = require('../consts');
var crypto = require('crypto');
var account = require('../../nodejs_lib/model/account');
var User    = require('../../nodejs_lib/model/user');
var logstat = require('../base/logstat');
var settings = require('../settings');

function gen_cookie(user, res) {
    var auth_token = user + '$$$$'; 
    var opts = {
        path: '/',
        maxAge: 1000 * 60,
        signed: true,
        httpOnly: true
        };
    res.cookie(settings.authcookiename, auth_token, opts);
}

//invoked by any requests passed to the reg router
router.use(function devLog(req, res, next) {
    console.log("enter login Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/', logstat.offLine);
router.get('/',function(req,res){
            res.render('login',{
                            title:consts.UP_TITLE
                        });
});

router.post('/', logstat.offLine);
router.post('/', function(req, res) {
    console.log('account login');
    var md5 = crypto.createHash('md5');
    var password = md5.update(req.body.password).digest('base64');
    account.getAccount(consts.ACC_KEY, req.body.account, function(err, ret) {
        if (err || ret == 0) {
            if (0 == ret && null == err) {
                err = 'the account not exist!';
            }
            req.flash('error', err);
            return res.redirect(consts.ACCOUNT_PATH+'/login');
        }
        console.log('checkAccount');
        account.checkAccount(consts.ACC_KEY+':'+req.body.account+consts.ACC_INFO, consts.ACC_PASSWD_FIELD,
                password, function(err, ret2) {
                    if (err) {
                        console.log(err);
                        req.flash('error', err);
                        return res.redirect(consts.ACCOUNT_PATH+'/login');
                    }
                    req.session.user = new User({name:req.body.account, passwd:password});
                    req.flash('success', 'login success');
                    //cookie, it not need because of app.use session?
                    //gen_cookie(req.body.account, res);
                    res.redirect('/');

                });
    });
});


module.exports = router;
