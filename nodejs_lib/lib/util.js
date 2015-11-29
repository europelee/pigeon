"use strict";

var crypto = require('crypto');

exports.createmd5 = function(str) {
    var md5 = crypto.createHash('md5');
    md5.update(str);
    return md5.digest('base64');
};

exports.randomMix = function(len) {
   var lists = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'],
    ret = [],
    total = lists.length;
   for (var i = 0; i < len; i++) {
       ret.push(lists[Math.floor(Math.random() * total)]);
   }

   return ret.join('');
};


