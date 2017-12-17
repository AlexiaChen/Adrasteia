import React from 'react';
import PropTypes from 'prop-types';
import './style.less';

const propTypes = {
    item: PropTypes.object,
    onSave: PropTypes.func.isRequired,
    onCancel: PropTypes.func.isRequired
};

class ItemEditor extends React.Component{
    render(){
        const {onSave, onCancel} = this.props;
        
        const item = this.props.item || {
            title: '',
            content: ''
        };

        let saveText = item.id ? 'Save' : 'Create';
        
        let save = () => {
            onSave({
                id: item.id,
                title: this.refs.title.value,
                content: this.refs.content.value,
                time: new Date().toISOString()
            });
        };
        
        
        return (
            <div className="col-md-8 item-editor-component">
                <div className="control-area">
                    <button className="btn btn-success" onClick={save}>{saveText}</button>
                    <button className="btn secondary" onClick={onCancel}>Cancel</button>
                </div>
                <div className="edit-area">
                    <input ref="title" type="text" placeholder="Please write the title" defaultValue={item.title}/>
                    <textarea ref="content" placeholder="Plesae write the content" defaultValue={item.content}></textarea>
                </div>
            </div>
        );
    }
}


ItemEditor.propTypes = propTypes;

export default ItemEditor;