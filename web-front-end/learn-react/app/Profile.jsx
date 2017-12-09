import React from 'react';
import PropTypes from 'prop-types';

const user = {
    firstName: "MathxH",
    lastName: "Chen",
    avatar: ""
};

function formatName(user){
    return user.firstName + ' ' + user.lastName;
}

let addFunc = (a, b) => {return a + b;};
let minFunc = (a,b) => a - b;

function getGreeting(user){
    if(user){
        return <h1> Hello, {formatName(user)}!</h1>;
    }else{
        return <h1> Hello, Stranger</h1>;
    }
}


class Profile extends React.Component {
    constructor(props){
        super(props);
    }

    render() {
        return (
            <div className="profile-component">
                {getGreeting(user)}
                <h1> My Name is: { this.props.name }</h1>
                <h1> Age is: {this.props.age }</h1>
                <h3> {addFunc(1,2)} is equal to {minFunc(5,2)}</h3>
            </div>
        );
    }
}

Profile.propTypes = {
    name: PropTypes.string,
    age: PropTypes.number
};


export default Profile;

