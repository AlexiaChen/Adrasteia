var world_lib = require('./world.js');
require('style-loader!css-loader!./style.css'); // webpack内置不解析css，需要另外安装css-loader style-loader
/* 
或者以上的语法也可以去掉，用webpack的命令行来实现
webpack hello.js hello.bundle.js --module-bind 'css=style-loader!css-loader'
*/

function hello(str) {
    alert(str);
}

hello('hello world'); // exexute hello function
alert(world_lib().msg);
