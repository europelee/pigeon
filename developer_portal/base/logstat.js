"use strict";

var consts = require('../consts');
/**
 *the below function for page visit permission control
 */
exports.onLine = function(req, res, next) {
    if (!req.session.user) {
        req.flash('error', 'login not yet');
        return res.redirect(consts.ACCOUNT_PATH+'/login');
    }

    next();
}

exports.offLine = function(req, res, next) {
    if (req.session.user) {
        req.flash('error', 'already login');
        return res.redirect('/');
    }

    next();
}
