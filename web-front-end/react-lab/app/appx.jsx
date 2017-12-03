import React from 'react';
import ReactDOM from 'react-dom';


function Appx(){
    return (
        <div className="container">
            <h2>
                <font color="red">Second React Page</font>
            </h2>
        </div>
    );
}
   

const app = document.createElement('div');
document.body.appendChild(app);

ReactDOM.render(<Appx />, app);