import React from 'react';
import uuid from 'uuid';
import CreateButton from './CreateButton';
import List from './List';
import TodoAction from '../actions/TodoAction';
import TodoStore from '../stores/TodoStore';

class ToDo extends React.Component{
    constructor(props){
        super(props);
       this.state = {
           todos: TodoStore.getAll()
       };

        this.createToDo = this.createToDo.bind(this);
        this.deleteToDo = this.deleteToDo.bind(this);
        this.onChange = this.onChange.bind(this);
    }

    componentDidMount() {
        TodoStore.addChangeListener(this.onChange);
    }

    componentWillUnmount() {
        TodoStore.removeChangeListener(this.onChange);
    }

    onChange() {
        this.setState({
            todos: TodoStore.getAll()
        });
    }

    createToDo() {
        TodoAction.create({id: uuid.v4(), content: '3rd Stuff'});
    }

    deleteToDo(id){
        TodoAction.delete(id);
    }

    render() {
        return (
            <div>
                <List items={this.state.todos} onDelete={this.deleteToDo}/>
                <CreateButton onClick={this.createToDo} />
            </div>
        );
    }

}

export default ToDo;