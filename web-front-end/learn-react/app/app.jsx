import React from 'react';
import ReactDOM from 'react-dom';
import Hello from './hello';
import Profile from './Profile'

const LynnProps = {
    name: 'Lynn Lee No.2',
    age: 25
};

class App extends React.Component{
    constructor(props){
        super(props);
    }

    render() {
        return (
            <div className="container">
                <Hello />
                <Profile name="Lynn Lee" age={16} />
                <Profile {...LynnProps}/>
            </div>
        );
    }
};


const app = document.createElement('div');
document.body.appendChild(app);

ReactDOM.render(<App />, app);