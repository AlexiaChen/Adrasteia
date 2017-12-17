import React from 'react';
import uuid from 'uuid';

import CreateBar from '../CreateBar';
import List from '../List';
import './style.less';

class App extends React.Component {
    constructor(props){
        super(props);

        this.state = {
            items: [
                {
                    title: 'I Love yOU',
                    content: 'I love you forever, darling.  missing you'
                },

                {
                    title: 'My King',
                    content: 'Your princess is coming from Alasika'
                }
            ],
            selectedId: null,
            editing: false
        };


    }
    
    render() {
        const { items, selectId, editing } = this.state;
        
        return (
            <section className="deskmark-component">
                <nav className="navbar navbar-fixed-top navbar-dark bg-inverse">
                    <a href="#" className="navbar-brand">MarkDown Editor App</a>
                </nav>

                <div className="containor">
                    <div className="row">
                        <div className="col-md-4 list-group">
                            <CreateBar />
                            <List items={items}/>
                        </div>
                    </div>
                </div>
            </section>
        );

    }


}


export default App;

