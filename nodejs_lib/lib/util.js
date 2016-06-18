"use strict";

var path = require('path');
var execFile = require('child_process').execFile;
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

exports.listFiles = function(dir_path, cb) {

    execFile('find', [ dir_path, '-type','f'],function(err, stdout, stderr) {
        if (err) {
            throw err;
        }
        var file_list = stdout.split('\n');
        /* now you've got a list with full path file names */
        console.log(file_list);
        var outFileList = [];
        var outLen = 0;
        var len = file_list.length, i=0;
        for (;i<len;++i) {
            var res = path.basename(file_list[i]);
            if (res != '') {
                console.log(res);
                outFileList[outLen++] = res;
            }
        }

        cb(outFileList);
    });
};
