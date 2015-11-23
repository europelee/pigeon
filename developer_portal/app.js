"use strict";

var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var session = require('express-session');
var cookieParser = require('cookie-parser');
var settings = require("./settings");
var bodyParser = require('body-parser');
var flash = require('connect-flash');

var redisCli = require('./lib/redisCli');
var routes = require('./routes/index');
var users = require('./routes/users');
var reg   = require('./routes/reg');
var login = require('./routes/login');
var personSettings = require('./routes/settings');

var sessionStore = require('connect-redis')(session);

var devices = require('./routes/devices.js');
var consts = require('./consts');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');

// uncomment after placing your favicon in /public
//app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(cookieParser(settings.sesssecret));
app.use(flash());
app.use(session({secret:settings.sesssecret, resave:true,
    saveUninitialized:true, store:new sessionStore({host:settings.host, port:settings.port, db:settings.db, prefix:settings.ssprefix})}));
    //saveUninitialized:true, store:new sessionStore({client:redisCli})}));
app.use(express.static(path.join(__dirname, 'public')));

app.use(function(req,res,next){
    console.log("app.usr local");
    res.locals.user = req.session.user;
    res.locals.post = req.session.post;
    var error = req.flash('error');
    res.locals.error = error.length?error:null;
    var success = req.flash('success');
    res.locals.success = success.length?success:null;
    next();
});

app.use('/', routes);
app.use(consts.ACCOUNT_PATH, reg);
app.use(consts.ACCOUNT_PATH, login);
app.use(consts.ACCOUNT_PATH, personSettings);
app.use('/users', users);
app.use('/v0.1/iot/devices', devices);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});


module.exports = app;
