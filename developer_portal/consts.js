"use strict"

exports.DP_TITLE = 'developer portal';
exports.ACCOUNT_PATH = '/api/account';
exports.ACC_KEY = 'api:acc';
exports.ACC_INFO = ':info';
exports.ACC_DEVTYPE = ':devtype';
exports.ACC_PASSWD_FIELD = 'passwd';
exports.ACC_APPKEY_FIELD = 'appkey';
exports.ACC_APPSEC_FIELD = 'appsecret';
exports.APP_KEY_PRE = 'api:appkey';
exports.HTTP_HEADER_APIKEY = 'apikey';
exports.DEV_SET_KEY = 'iot:devset';
exports.DEV_TYPE_KEY = 'iot:deves';
exports.DEV_META    = ':meta';
exports.DEV_TYPE_INFO = ':info';
function devTypeInfo(d, m, v, de, me) {
    this.d = d;
    this.m = m;
    this.v = v;
    this.de = de;
    this.me = me;
};
exports.DEV_TINFO_NLIST = new devTypeInfo('devtype', 'manufacturer', 'version', 'description', 'meta');
