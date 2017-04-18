/* 
npm install thunkify co
co library is used to run generator automically
*/
var co = require('co');
var thunkify = require('thunkify');
var fs = require('fs');

var read_thunk = thunkify(fs.readFile);

//使用 co 的前提条件是，Generator 函数的 yield 命令后面，只能是 Thunk 函数或 Promise 对象。
var gen = function* (){
    var r1 = yield read_thunk('./ReadMe.md');
    console.log('gen', r1.toString('utf-8'));
    var r2 = yield read_thunk('./thunk.txt');
    console.log('gen', r2.toString('utf-8'));
};

co(gen);

// co will return promise object
co(gen).then(() => {
    console.log('Generator function finished');
});

///////////////////////////////////////////////

//run automically based on promise object
var readFilePromise = (filename) => {
    return new Promise((resolve, reject) => {
        fs.readFile(filename,(err, data) => {
            if(err) reject(err);
            resolve(data);
        });
    });
};

var gen1 = function* (){
    var f1 = yield readFilePromise('./ReadMe.md');
    var f2 = yield readFilePromise('./thunk.txt');
    console.log('gen1', f1.toString('utf-8'));
    console.log('gen1', f2.toString('utf-8'));
}

co(gen1).then(() => {
    console.log('gen1 down');
});