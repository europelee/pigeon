"use strict";

function Command(seq, dest, content, callback) {
    this.seq = seq;
    this.dest = dest;
    this.content = content;
    this.callback = callback;
}

module.exports = Command;
