/*put this code block run in the console of browser*/
////////////////////closure 1/////////////
function fn1(){
    var a = 1;
    return function(){
    console.log(++a);
    }
}


var fn2 = fn1();

fn2();        //output 2

fn2();        //output 3

////////////////closure 2/////////////
var name = "window";

var obj = {
　　name : "obj",
    addr: "Brookly",
　　getName : function(){
　　　　return function(){
　　　　　　return this.name;
　　　　};
　　},
    getAddress:function(){
        return this.addr;
    }

};
　
console.log(obj.getName()()); // output window
console.log(obj.getAddress()); // output Brookly
console.log(this.name);// output window

///////////////closure 3/////////////////
var name = "window";

var obj = {
    name: "obj",
    getName: function () {
        console.log('1', this.name, this);
        return function () {
            console.log('2', this.name, this);
            return this.name;
        };
    }
};
/*
1 obj Object { name: "obj", getName: obj.getName() } 
2 window Window → https://juejin.im/post/58de8dadac502e006cfc7b76 
3 window Window → https://juejin.im/post/58de8dadac502e006cfc7b76
*/
console.log('3', obj.getName()(), this);
