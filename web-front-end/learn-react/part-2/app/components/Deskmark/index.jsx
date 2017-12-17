import React from 'react';
import uuid from 'uuid';

import CreateBar from '../CreateBar';
import List from '../List';
import ItemEditor from '../ItemEditor';
import ItemShowLayer from '../ItemShowLayer';
import './style.less';

class App extends React.Component {
    constructor(props){
        super(props);

        this.state = {
            items: [
                {
                    id: uuid.v4(),
                    title: 'I Love yOU',
                    content: 'I love you forever, darling.  missing you'
                },

                {
                    id: uuid.v4(),
                    title: 'My King',
                    content: 'Your princess is coming from Alasika'
                }
            ],
            selectedId: null,
            editing: false
        };

        this.selectItem = this.selectItem.bind(this);
        this.saveItem = this.saveItem.bind(this);
        this.deleteItem = this.deleteItem.bind(this);
        this.createItem = this.createItem.bind(this);
        this.editItem = this.editItem.bind(this);
        this.cancelEdit = this.cancelEdit.bind(this);
    }

    selectItem(id){
        if(id === this.state.selectedId){
            return;
        }

        this.setState({
            selectedId: id,
            editing: false
        });
    }

    saveItem(item){
        let items = this.state.items;
        if(!item.id){
           /* items = [...items, {
                ...item,
                id: uuid.v4(),
                time: new Date().getTime()
            }]*/

            items.push({
                id: uuid.v4(),
                time: new Date().getTime()
            });
        }else{
            items = items.map(
                exist => {
                    if(exist.id === item.id)
                    {
                        exist.title = item.title;
                        exist.content
                    }
                }
            );
        }

        this.setState({
            items,
            selectedId: item.id,
            editing: false
        });
    }

    deleteItem(id){
        if(!id){
            return;
        }

        const newItems = this.state.items.filter(result => result.id !== id);

        this.setState({
            items: newItems
        });
    }
    
    createItem() {
        this.setState({
            selectedId: null,
            editing: true
        });
    }

    editItem(id) {
        this.setState({
            editing: true,
            selectedId: id
        });
    }

    cancelEdit() {
        this.setState({
            editing: false
        });
    }

    render() {
        const { items, selectId, editing } = this.state;
        const selected = selectId && items.find( item => item.id === selectId);
        const mainPart = editing ? 
            (
                <ItemEditor 
                    item={selected}
                    onSave={this.saveItem}
                    onCancel={this.cancelEdit}
                />
            )
            :
            (
                <ItemShowLayer
                    item={selected}
                    onEdit={this.editItem}
                    onDelete={this.deleteItem} 
                />
            );
        
        return (
            <section className="deskmark-component">
                <nav className="navbar navbar-fixed-top navbar-dark bg-inverse">
                    <a href="#" className="navbar-brand">MarkDown Bloger App</a>
                </nav>

                <div className="containor">
                    <div className="row">
                        <div className="col-md-4 list-group">
                            <CreateBar onClick={this.createItem}/>
                            <List items={items} onSelect={this.selectItem}/>
                        </div>
                    </div>
                    {mainPart}
                </div>
            </section>
        );

    }


}


export default App;

