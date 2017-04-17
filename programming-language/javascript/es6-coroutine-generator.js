/* 
Generator 是协程在ES6上的实现，协程的本意是想用单线程来模拟多线程任务，所以可以切换执行权
*/

//必须加星号，以标识是Generator，yield关键字必须在Generator中
function* gen1(x){
    var y = yield x + 2;
    return y;
}

var g = gen1(1);
console.log('g1', g.next()); // { value: 3, done: false }
console.log('g1', g.next()); // { value: undefined, done: true }

function* gen2(){
    var num = 0;
    yield ++num;
    return () => {return num;}; 
}

var g2 = gen2();
console.log('g2', g2.next()); //{ value: 1, done: false }
console.log('g2', g2.next()); // { value: [Function], done: true }
console.log('g2', g2.next()); // { value: undefined, done: true }
console.log('g2', g2.next()); // { value: undefined, done: true }

function* gen3(){
    var step0 = "Step zero";
    yield;
    var step1 = yield "Step one";
    var step2 = yield "Step two";
    var step3 = yield "Step three";
    return "done";
}

var g3 = gen3();
console.log('g3', g3.next()); // g3 { value: undefined, done: false }
console.log('g3', g3.next()); // g3 { value: 'Step one', done: false }
console.log('g3', g3.next()); // g3 { value: 'Step two', done: false }
console.log('g3', g3.next()); // g3 { value: 'Step three', done: false }
console.log('g3', g3.next()); // g3 { value: 'done', done: true }

/////////////Generator函数的数据交换和错误处理////////////////
/*
next 方法返回值的 value 属性，是 Generator 函数向外输出数据；
next 方法还可以接受参数，这是向 Generator 函数体内输入数据。
*/
function* gen4(x){
  var y = yield x + 2;
  return y;
}

var g4 = gen4(1);
console.log('g4', g4.next()); // { value: 3, done: false }
console.log('g4', g4.next(2)); // { value: 2, done: true }

//Generator 函数内部还可以部署错误处理代码，捕获函数体外抛出的错误
function* gen5(x){
  try {
    var y = yield x + 2;
  } catch (e){ 
    console.log(e + 'Fuck');
  }
  return y;
}

var g5 = gen5(1);
console.log('g5', g5.next());
g5.throw('出错了');