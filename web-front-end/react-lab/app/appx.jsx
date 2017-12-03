import React from 'react';
import ReactDOM from 'react-dom';

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

const HelloTitle = (
    <h2> Hello, {formatName(user)}</h2>
);

const imageEle =  <img src={user.avatar}> </img>;

function Appx(){
    return (
        <div className="container">
            <h2>
                <font color="red">Appx Page</font>
            </h2>

            <h3> {addFunc(1,2)} is equal to {minFunc(5,2)}</h3>

            when param is valid: {getGreeting(user)}

            when param is null: {getGreeting(null)}
           
        </div>
    );
}
   

const app = document.createElement('div');
const hello = document.createElement('div');
const avatar = document.createElement('div');
document.body.appendChild(app);
document.body.appendChild(hello);
document.body.appendChild(avatar);

ReactDOM.render(<Appx />, app);
ReactDOM.render(HelloTitle,hello);
ReactDOM.render(imageEle,avatar);