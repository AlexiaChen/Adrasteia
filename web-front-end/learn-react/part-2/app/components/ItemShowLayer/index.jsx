import React from 'react';
import PropTypes from 'prop-types';
import './style.less';

import marked from 'marked';

const propTypes = {
    item: PropTypes.object,
    onEdit: PropTypes.func.isRequired,
    onDelete: PropTypes.func.isRequired
};

function ItemShowLayer(props){
    if(!props.item || !props.item.id)
    {
        return (
            <div className="col-md-8 item-show-layer-component">
                <div className="no-select">Please select the title on left list!</div>
            </div>
        );
    }

    // convert markdown string to html string
    const content = marked(props.item.content);

    return (
        <div className="col-md-8 item-show-layer-component">
            <div className="control-area">
                <button onClick={() => onEdit(props.item.id)} className="btn btn-primary">Edit</button>
                <button onClick={() => onDelete(props.item.id)} className="btn btn-danger">Delete</button>
            </div>
            <h2>{props.item.title}</h2>
            <div className="item-text">
                <div dangerouslySetInnerHTML={{ __html: content}}></div>
            </div>
        </div>
    );
}


ItemShowLayer.propTypes = propTypes;
export default ItemShowLayer;


