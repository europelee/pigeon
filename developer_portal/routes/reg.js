"use strict";

var express = require('express');
var router = express.Router();
var devstore = require('../../nodejs_lib/model/devstore');
var async = require('async');
var consts = require('../consts');
var crypto = require('crypto');
var account = require('../model/account');
var User    = require('../model/user');
var logstat = require('../base/logstat');

//invoked by any requests passed to the reg router
router.use(function devLog(req, res, next) {
    console.log("enter reg Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    next();
});

router.get('/reg', logstat.offLine);
router.get('/reg',function(req,res){
            res.render('reg',{
                            title:"account register"
                        });
});

//1.create user
router.post('/reg', logstat.offLine);
router.post('/reg', function(req, res) {
    console.log("create account ");
    if(req.body['password-repeat']!=req.body['password']){
        console.log('passwd is wrong!');
        req.flash('error','password is wrong!');
        return res.redirect(consts.ACCOUNT_PATH+'/reg');
    }
    
    if (req.body['password'].length < 6) {
        req.flash('error', 'password too short, must be more than 6 characters!');
        return res.redirect(consts.ACCOUNT_PATH+'/reg');
    }
    
    var md5 = crypto.createHash('md5');
    var password = md5.update(req.body.password).digest('base64');
    account.getAccount(consts.ACC_KEY, req.body.account, function(err, ret) {
        if (ret == 1)
        {
            err = 'the account name already exists!';
        }

        if (err)
        {
            req.flash('error', err);
            console.log(err);
            return res.redirect(consts.ACCOUNT_PATH + '/reg');
        }
        
        console.log('save');
        account.addAccount(consts.ACC_KEY, req.body.account, function(err, ret) {
            if (err != null || ret != 1)
            {
                req.flash('error', err);
                console.log(err);
                return res.redirect(consts.ACCOUNT_PATH + '/reg');
            }
            
            account.setAccount(consts.ACC_KEY+':'+req.body.account+consts.ACC_INFO, consts.ACC_PASSWD_FIELD,
                   password, function(err, ret) {
                if (err != null)
                {
                    //todo: the above oprator need roll back
                    req.flash('error', err);
                    console.log(err);
                    return res.redirect(consts.ACCOUNT_PATH + '/reg');
                }
                
                //genkey, no need sync with the below redirect
                account.genKeySc(consts.ACC_KEY+':'+req.body.account+consts.ACC_INFO, consts.ACC_APPKEY_FIELD,
                    consts.ACC_APPSEC_FIELD, req.body.account, password, function(err, ret, appkeyval) {
                        if (err) {
                            console.log(err);
                        }

                        account.saveAKUsrMap(consts.APP_KEY_PRE, appkeyval, req.body.account, function(err, ret){
                            if (err) {
                                console.log(err);
                            }
                        });
                    });          
                
                var newUser = new User({
                    name : req.body.account,
                    passwd : password
                });

                req.session.user = newUser;
                req.flash('success', 'add account succ!');
                return res.redirect('/');            
            }); 
        });
    }); 
});

module.exports = router;
