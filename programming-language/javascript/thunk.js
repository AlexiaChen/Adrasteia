/* 
npm install thunkify
Thunk 是把一个多参函数转换为单参函数的函数。
Thunk 是函数柯里化的一个子集
任何函数只要参数有回调函数，就可以写成Thunk函数形式
*/

var thunkify = require('thunkify');
var fs = require('fs');

var read_thunk = thunkify(fs.readFile);
read_thunk('./ReadMe.md')(function(err,buf){
    var str = buf.toString('utf8');
    console.log('part1 ', 'ReadMe content is: ', str);
});

// ES6 有了 Generator 函数，Thunk 函数现在可以用于 Generator 函数的自动流程管理。

var gen1 = function* (){
    var r1 = yield read_thunk('./ReadMe.md');
    console.log('gen1', r1.toString('utf-8'));
    var r2 = yield read_thunk('./thunk.txt');
    console.log('gen1', r2.toString('utf-8'));
};

//手动流程管理
var g = gen1();
var r1 = g.next();
r1.value((err,data) => {
    if(err) throw err;
    var r2 = g.next(data);
    r2.value((err,data) => {
        if(err) throw err;
        g.next(data);
    });
});

//自动流程管理

var gen2 = function* (){
    var r1 = yield read_thunk('./ReadMe.md');
    console.log('gen2', r1.toString('utf-8'));
    var r2 = yield read_thunk('./thunk.txt');
    console.log('gen2', r2.toString('utf-8'));
};

((gen_callback) => {
    console.log('running this machine to simulate  manual execution like co library');

    var gen = gen_callback();
    function next(err,data){
        console.log('called next');
        var result = gen.next(data);
        if(result.done) return ;
        result.value(next);
    }
    console.log('calling next');
    next();
})(gen2);