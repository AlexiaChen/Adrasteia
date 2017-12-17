import React from 'react';
import PropTypes from 'prop-types';
import './style.less';

const propTypes = {
    item: PropTypes.object,
    onSave: PropTypes.func.isRequired,
    onCancel: PropTypes.func.isRequired
};

function ItemEditor(props){
    const {onSave, onCancel} = this.props;
    
    const item = this.item || {
        title: '',
        content: ''
    };
    
    let save = () => {
        onSave({
            title: this.refs.title.value,
            content: this.refs.content.value
        });
    };
    
    
    return (
        <div className="col-md-8 item-editor-component">
            <div className="control-area">
                <button className="btn btn-success" onClick={save}></button>
                <button className="btn secondary" onClick={onCancel}></button>
            </div>
            <div className="edit-area">
                <input ref="title" type="text" placeholder="Please write the title" defaultValue={item.title}/>
                <textarea ref="content" placeholder="Plesae write the content" defaultValue={item.content}></textarea>
            </div>
        </div>
    );
}


ItemEditor.propTypes = propTypes;

export default ItemEditor;