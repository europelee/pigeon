"use strict";

var express = require('express');
var router = express.Router();
var devstore = require('../model/devstore');
var async = require('async');
var account = require('../model/account');
var consts  = require('../consts');

function getUsrName(req, usr_cb) {
    var apikey = consts.APP_KEY_PRE + ':' + req.get(consts.HTTP_HEADER_APIKEY);
    account.checkAKUsrMap(apikey, function(err, ret) {
            usr_cb(err, ret);
    });   
}

function getValidDevInfo(reqInfo) {
    
        var itemSet = new Object();
        //itemSet[consts.DEV_TINFO_NLIST.d] = reqDevType;
        if (typeof(reqInfo[consts.DEV_TINFO_NLIST.m]) != 'undefined')
                itemSet[consts.DEV_TINFO_NLIST.m] = reqInfo[consts.DEV_TINFO_NLIST.m]; 
        else
                itemSet[consts.DEV_TINFO_NLIST.m] = null; 

        if (typeof(reqInfo[consts.DEV_TINFO_NLIST.v]) != 'undefined')
                itemSet[consts.DEV_TINFO_NLIST.v] = reqInfo[consts.DEV_TINFO_NLIST.v]; 
        else
                itemSet[consts.DEV_TINFO_NLIST.v] = null;

        if (typeof(reqInfo[consts.DEV_TINFO_NLIST.de]) != 'undefined')
                itemSet[consts.DEV_TINFO_NLIST.de] = reqInfo[consts.DEV_TINFO_NLIST.de]; 
        else
                itemSet[consts.DEV_TINFO_NLIST.de] = null;

       return itemSet; 
}

//invoked by any requests passed to the devices router
router.use(function devLog(req, res, next) {
    console.log("enter devices Router, from " + req.ip + " body:"+JSON.stringify(req.body)); 
    //check apikey valid
    var apikey = consts.APP_KEY_PRE + ':' + req.get(consts.HTTP_HEADER_APIKEY);
    account.checkAKUsrMap(apikey, function(err, ret) {
        if (err != null || ret == null) {
            res.sendStatus(401);
        }
        else {
            next();        
        }
    });    
});

//1.list all devices
router.get('/', function(req, res) {
    console.log("list all devices supported!");
    getUsrName(req, function(err, usrName){
        if (err == null) {
            account.listOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, function(err, ret){
                if (err == null)
                    res.status(200);
                else
                    res.status(204);

                res.send(ret);
            });    
        }
        else {
            res.status(204);
            res.send(err);
        }            
    });
});

//2.create a new device type
router.post('/', function(req, res) {
    console.log("create a new device type: ", req.body);
    //valid bodyString
    if (null == req.body.devtype || typeof(req.body.devtype) == "undefined")
    {
        res.sendStatus(400);
        return;
    }
    
    var devInfo = getValidDevInfo(req.body);
    var bodyString = JSON.stringify(devInfo);
    
    async.waterfall([
         function(cb) {
                 
             //check dev reg
             devstore.checkDevIsReg(consts.DEV_SET_KEY, req.body.devtype, function(err, isReg){
            
                 if (null == err && isReg == 0)
                 {   
                     cb(null);   
                 }
                 else
                     cb("there already exist type:"+req.body.devtype);
             });
         },
         
         function(cb) {
            devstore.createDev(consts.DEV_SET_KEY, consts.DEV_TYPE_KEY+':'+req.body.devtype+consts.DEV_TYPE_INFO, req.body.devtype, consts.DEV_TYPE_INFO, bodyString, function(err, ret) {
                if (err == null)
                {
                    getUsrName(req, function(err, usrName){
                        if (err == null) {
                            //addOwnDev
                            account.addOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.body.devtype, function(err, isAdd){
                                   cb(err, ret);
                            });    
                        }
                        else
                            cb(err, ret);                
                    });
                }
                else
                    cb(err, ret);
                
            });    
         
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(400);
             res.send(err);
         }
         else
         {
             if (ret == 0)    
                 res.sendStatus(200);
             else
                 res.sendStatus(201);
         }
    });


   
});

//3.query a devtype info
router.get('/:devtype_id', function(req, res) {
    console.log('get devtype info from ', req.params.devtype_id);
    getUsrName(req, function(err, usrName){
        if (err == null) {
            account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                if (1 == isOwn) {
                    devstore.listDev(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_TYPE_INFO, consts.DEV_TYPE_INFO, function(err, ret) {
                    if (null == err && ret != null)
                        res.status(200);
                    else
                        res.status(404);

                    res.type('json');
                    res.send(ret);

                    });                                    
                }
                else {
                    res.sendStatus(403);
                }
            });    
        }
        else {
            res.status(500);
            res.send(err);
        }
                            
    });
});

//4.update a devicetype info
router.put('/:devtype_id', function(req, res) {
    
    if (null == req.params.devtype_id || typeof(req.params.devtype_id) == "undefined")
    {
        res.sendStatus(400);
        return;
    }

    console.log("update devicetype ",req.params.devtype_id);

    var devInfo = getValidDevInfo(req.body);
    var bodyString = JSON.stringify(devInfo);
    
    getUsrName(req, function(err, usrName){
        if (err == null) {
            account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                if (1 == isOwn) {
                    devstore.updateDev(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_TYPE_INFO, consts.DEV_TYPE_INFO, bodyString, function(err, ret) {
                        if (err == null)
                        {
                            if (ret == 0)    
                                res.sendStatus(200);
                            else
                                res.sendStatus(201);
                        }
                        else
                            res.sendStatus(400);
                });                                    
                }
                else {
                    res.sendStatus(403);
                }
            });    
        }
        else {
            res.status(500);
            res.send(err);
        }
                            
    });

});

//5.delete a devicetype
router.delete('/:devtype_id', function(req, res) {
    console.log('delete devicetype ', req.params.devtype_id);

    async.waterfall([
         function(cb) {
             getUsrName(req, function(err, usrName){
                if (err == null && usrName != null) {
                    account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                        if (1 == isOwn) {
                            account.delOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isDel){
                                cb(err, isDel);    
                            });
                        }
                        else
                            cb((err==null?('invalid type:'+req.params.devtype_id+' for '+usrName):err), 403);
                        });                                    
                        }
                else {
                    cb((err==null?('invalid user '+usrName):err), 500);        
                }
                });                  
         },
         function(ret, cb) {
             devstore.remDev(consts.DEV_SET_KEY, consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_TYPE_INFO, consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_META, req.params.devtype_id, function(err, ret){
                if (err == null) {
                    //should not check value of every elem of ret equal to 1. 
                }

                cb(err, ret);
             });
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(typeof(ret)=='number'?ret:400);
             res.send(err);
         }
         else
         {
             res.sendStatus(200);
         }
    });
});

//6.create dev meta
router.post('/:devtype_id/meta', function(req, res) {
    console.log('create meta for devtype ' , req.params.devtype_id, ':', req.body);
    async.waterfall([
         function(cb) {
             getUsrName(req, function(err, usrName){
                if (err == null && usrName != null) {
                    account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                        if (1 == isOwn) {
                            cb(null);    
                        }
                        else
                            cb((err==null?('invalid type:'+req.params.devtype_id+' for '+usrName):err), 403);
                        });                                    
                        }
                else {
                    cb((err==null?('invalid user '+usrName):err), 500);        
                }
                });                  
         },
         function(cb) {

             devstore.createDevMeta(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_META, req.body, function(err, ret) {
                 cb(err, ret);           
             });
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(ret);
             res.send(err);
         }
         else
         {
             res.sendStatus(201);
         }
    });
    
});

//7.query dev meta
router.get('/:devtype_id/meta', function(req, res) {
    console.log('query meta for devtype ' + req.params.devtype_id);
    async.waterfall([
         function(cb) {
             getUsrName(req, function(err, usrName){
                if (err == null && usrName != null) {
                    account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                        if (1 == isOwn) {
                            cb(null);    
                        }
                        else
                            cb((err==null?('invalid type:'+req.params.devtype_id+' for '+usrName):err), 403);
                        });                                    
                        }
                else {
                    cb((err==null?('invalid user '+usrName):err), 500);        
                }
                });                  
         },
         function(cb) {

             devstore.queryDevMeta(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_META, function(err, ret) {
                 cb(err, ret);           
             });
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(ret);
             res.send(err);
         }
         else
         {
             if (err==null&&ret != null)
                 res.status(200);
             else
                 res.status(404);
          
             res.type('json');    
             res.send(ret);
         }
    });
});

//8.update dev meta
router.put('/:devtype_id/meta', function(req, res) {
    console.log('update meta for devtype ' , req.params.devtype_id, ':', req.body);
    async.waterfall([
         function(cb) {
             getUsrName(req, function(err, usrName){
                if (err == null && usrName != null) {
                    account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                        if (1 == isOwn) {
                            cb(null);    
                        }
                        else
                            cb((err==null?('invalid type:'+req.params.devtype_id+' for '+usrName):err), 403);
                        });                                    
                        }
                else {
                    cb((err==null?('invalid user '+usrName):err), 500);        
                }
                });                  
         },
         
         function(cb) {

             devstore.updateDevMeta(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_META, req.body, function(err, ret) {
                 cb(err, ret);           
             });
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(ret);
             res.send(err);
         }
         else
         {
             if (ret != 'OK') {
                res.sendStatus(400);
             }
             else {
                res.sendStatus(200);
             }
         }
    });
   
});

//9.delete dev meta
router.delete('/:devtype_id/meta', function(req, res) {
    console.log('delete meta for devtype ' + req.params.devtype_id);

    async.waterfall([
         function(cb) {
             getUsrName(req, function(err, usrName){
                if (err == null && usrName != null) {
                    account.checkOwnDev(consts.ACC_KEY+':'+usrName+consts.ACC_DEVTYPE, req.params.devtype_id, function(err, isOwn){
                        if (1 == isOwn) {
                            cb(null);    
                        }
                        else
                            cb((err==null?('invalid type:'+req.params.devtype_id+' for '+usrName):err), 403);
                        });                                    
                        }
                else {
                    cb((err==null?('invalid user '+usrName):err), 500);        
                }
                });                  
         },
         
         function(cb) {

             devstore.delDevMeta(consts.DEV_TYPE_KEY+':'+req.params.devtype_id+consts.DEV_META, function(err, ret) {
                 cb(err, ret);           
             });
         }   
         ],function(err, ret){
         if (null != err)
         {
             console.log(err);
             res.status(typeof(ret)=='number'?ret:400);
             res.send(err);
         }
         else
         {
             if (ret == 1) {
                 res.sendStatus(200);
             }
             else {
                 res.sendStatus(404);
             }
         }
    });

});


module.exports = router;
