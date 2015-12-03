"use strict"

exports.UP_TITLE = 'user portal';
exports.ACCOUNT_PATH = '/user/account';
exports.ACC_REGPATH_INDEX = '/reg';
exports.ACC_LOGINPATH_INDEX = '/login';
exports.ACC_LOGOUTPATH_INDEX = '/logout';
exports.ACC_SETTINGSPATH_INDEX = '/settings';
exports.ACC_KEY = 'user:acc';
exports.ACC_INFO = ':info';
exports.ACC_PASSWD_FIELD = 'passwd';
exports.PROFILE_ITEM = 'profile';
exports.DEVICES_ITEM = 'devices';
exports.GWBIND_POSTSETKEY = 'gwlist';
exports.GW_SET_KEY = 'iot:gateway';
exports.GW_MACKEY = 'macaddr';
exports.GW_OWNERKEY = 'owner';
exports.GW_IDFIELD = 'ID';
exports.GWBIND_PATH = '/v0.1/iot/devices/gateway/bind';

function gwInfo(id, macaddr) {
    this.id = id;
    this.macaddr = macaddr;
};

exports.GW_INFO_META = new gwInfo('ID', 'MAC');
