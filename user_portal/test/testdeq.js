var Deque = require("double-ended-queue");

var tdeq = new Deque();
tdeq.push(1);

tdeq.push(2);
tdeq.push(3);
tdeq.push(4);

var i = tdeq.shift();
var j = tdeq.shift();
var k = tdeq.shift();
console.log('i j k:'+i+','+j+','+k);
tdeq.unshift(j);
tdeq.unshift(i);
var array = tdeq.toArray();
console.log(tdeq.length+':'+array);
