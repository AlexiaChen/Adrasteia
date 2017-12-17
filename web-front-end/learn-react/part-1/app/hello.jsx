import React from 'react';

export default class Hello extends React.Component{
    constructor(props){
        super(props);
    }

    render() {
        return (
            <div className="helloTitle">
                <h1>Hello React!</h1>
            </div>
        );
    }
};