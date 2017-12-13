import React from 'react';
import PropTypes from 'prop-types';

//用纯函数表示一个组件
function Hobby(props) {
    return <li> {props.hobby} </li>;
}


class HobbyList extends React.Component {
    constructor(props){
        super(props);

        this.state = {
            hobbies: ['the lord of ring', 'Jame', 'Harry Potter']
        }
    }

    render() {
        return (
            <div className="Hobby-List">
                <h1>My Hobby List is:</h1>
                <ul>
                    { this.state.hobbies.map( (hobby, i) => 
                        <Hobby key={i} hobby={hobby}/>
                    )}
                </ul>
            </div>
        );
    }
}

export default HobbyList;