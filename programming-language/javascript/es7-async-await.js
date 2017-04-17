///////////////////////////////////////////////
const makeRequest = () =>
    getJSON()  // getJSON会返回一个promise对象
               // 该promise对象会异步把服务器返回的json数据传入它的resolve方法中
        .then(data => {
            console.log(data)
            return "done"
        })

makeRequest()

///////////////////////////////////////////
/* 
use async & await
当makeRequest函数执行的时候，一旦遇到await就会先返回，等到异步操作完成，再接着执行函数体内后面的语句。
await getJSON() 说明console.log的调用，会等到getJSON()返回的promise对象resolve之后触发。
*/ 
const makeRequest = async () => {
    console.log(await getJSON()) // await关键字只能出现在用async声明的函数体内
    return "done"  
}

makeRequest()

/////////////////////////////////////////////
function timeout(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms); 
    });
}

async function asyncPrint(value, ms) {
    await timeout(ms); // 函数执行到这里遇到await立即返回，并返回一个promise对象
    console.log(value); 
}

asyncPrint('hello world', 50);// 50毫秒以后输出hello world

////////////////////////////////////////////////
/*错误处理 
使用try/catch结构，但是在promises的情况下，try/catch难以处理在JSON.parse过程中的问题，
原因是这个错误发生在Promise内部。想要处理这种情况下的错误，我们只能再嵌套一层try/catch
*/
const makeRequest = () => {
    try {
        getJSON()
        .then(result => {
            // this parse may fail
            const data = JSON.parse(result)
            console.log(data)
        })
    } 
    catch (err) {
        console.log(err)
    }
}

//用async & await处理，一切变得简单
const makeRequest = async () => {
      try {
          // this parse may fail
          const data = JSON.parse(await getJSON())
          console.log(data)
      } 
      catch (err) {
          console.log(err)
      }
}

/////////////////////////////////////////////////
/*
想象一下这样的业务需求：我们需要先拉取数据，然后根据得到的数据判断是否输出此数据，或者根据数据内容拉取更多的信息。
以下的代码简直没法看，嵌套过多
*/
const makeRequest = () => {
    return getJSON().then(
        data => {
            if(data.isNeedAnotherData){
                return makeAnotherRequest(data).then(
                    moreData => {
                        console.log(moreData);
                        return moreData;
                    }
                );
            }
            else{
                console.log(data);
                return data;
            }
        }
    );
}

//用async & await改善
const makeRequest = async () => {
    const data = await getJSON();
    if(data.isNeedAnotherData){
        const moreData = await makeAnotherRequest();
        console.log(moreData);
        return moreData;
    }
    else{
        console.log(data);
        return data;
    }
}

/////////////////////////////////////////////////////////////////
/*
中间值Intermediate values 
一个经常出现的场景是，我们先调起promise1，然后根据返回值，调用promise2，之后再根据这两个Promises得值，调取promise3。
这样层层嵌套越来越深，代码没法看
*/
const makeRequest = () => {
    return promise1().then(data1 => {
        return promise2(data1).then(data2 => {
            return promise2(data1,data2);
        });
    });
}

//用Promise.all来避免很深的嵌套
const makeRequest = () => {
    return promise1().then(data1 => {
        // do something
        return Promise.all([data1,promise2(data1)]);
    })
    .then(([data1,data2]) => {
        //do something
        return promise3(data1,data2);
    })
}

//以上方法解决了一些嵌套问题，但是又牺牲了语义，用async & await,最为简洁
const makeRequest = async () => {
    const data1 = await promise1()
    const data2 = await promise2(data1)
    return promise3(data1, data2);
}

///////////////////////////////////////////////////////
/*
错误堆栈信息
想象一下我们链式调用了很多promises，一级接一级。紧接着，这条promises链中某处出错
此链条的错误堆栈信息并没用线索指示错误到底出现在哪里。更糟糕的是，
它还会误导开发者：错误信息中唯一出现的函数名称其实根本就是无辜的。
*/
const makeRequest = () => {
    return callAPromise()
        .then(() => callAPromise())
        .then(() => callAPromise())
        .then(() => callAPromise())
        .then(() => callAPromise())
        .then(() => {
            throw new Error("oops");
        })
}

makeRequest()
    .catch(err => {
        console.log(err);
        // output
        // Error: oops at callAPromise.then.then.then.then.then (xxxx.js:8:13)
    })

//用async & await改善
const makeRequest = async () => {
    await callAPromise()
    await callAPromise()
    await callAPromise()
    await callAPromise()
    await callAPromise()
    throw new Error("oops");
}

makeRequest()
    .catch(err => {
        console.log(err);
        // output
        // Error: oops at makeRequest (xxxx.js:7:9)
    })

///////////////////////////////////////////////////
/*
方便调试 
在一个返回表达式的箭头函数中，我们不能设置断点，我们无法在每一行设置断点
*/
const makeRequest = () => {
    return callAPromise()
        .then(()=>callAPromise())
        .then(()=>callAPromise())
        .then(()=>callAPromise())
        .then(()=>callAPromise())
}

//用async & await就可以方便的设置断点
const makeRequest = async () => {
    await callAPromise()
    await callAPromise()
    await callAPromise()
    await callAPromise()
}
