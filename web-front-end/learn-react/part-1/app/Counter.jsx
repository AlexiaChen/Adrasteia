import React from 'react';
import PropTypes from 'prop-types';

class Counter extends React.Component {
    constructor(props){
        super(props);

        this.state = {
            count: -1
        };

        ////////function bind//////////
        this.handleAdd = this.handleAdd.bind(this);
        this.handleMin = this.handleMin.bind(this);
    }

    //当render完成并且该组件在DOM树中装载完毕之后调用，有且只有一次
    //所以可以在这里获取组件的DOM结构，也可以用Ajax加载服务端数据
    componentDidMount() {
        this.timeID =  setTimeout(() => {
            this.handleAdd();
        },3000);
    }

    //在render之前被调用，可以在渲染之前做一些准备工作
    componentWillMount() {

    }

    //当组件接收到新的props被调用，nextProps就是新传输的props，
    //有些时候可以用它和this.props比较，来决定是否用this.setState实现重新渲染UI
    componentWillReceiveProps(nextProps){

    }

    //在重新render之前被调用，可以返回一个bool值来决定是否更新，默认返回true
    shouldComponentUpdate() {
        return true;
    }

    //在重新render之前被调用，可以在渲染之前做一些准备工作，与componentWillMount类似
    componentWillUpdate() {

    }

    //在重新render之后被调用，与componentDidMount类似
    componentDidUpdate() {

    }

    //在组件被卸载和销毁之前调用的方法，可以在这里做一些清理工作，相当于C++的析构函数
    componentWillUnmount() {
        clearTimeout(this.timeID);
    }

    handleAdd() {
        let count = this.state.count;
        count++;
        this.setState({
           count
        });
    }

    handleMin() {
        let count = this.state.count;
        if(count > 0)
        {
            count--;
            this.setState({
                count
            });
        }
       
    }
    //必须要实现的方法，最好是纯函数 UI = render(data)
    render() {
        return (
            <div className="counter-componet">
                <button onClick={this.handleMin}>-</button>
                    <span> 当前计数为： { this.state.count }</span> 
                <button onClick={this.handleAdd}>+</button>
            </div>
        );
    }
}

export default Counter;