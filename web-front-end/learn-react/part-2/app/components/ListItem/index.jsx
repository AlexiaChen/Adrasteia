import React from 'react';
import PropTypes from 'prop-types';

const propTypes = {
    item: PropTypes.object.isRequired,
    onClick: PropTypes.func.isRequired
};


function ListItem(props){
    let formatTime = 'unknown time';
    if(props.item.time){
        formatTime = new Date(props.time).toISOString().match(/(\d{4}-\d{2}-\d{2})/)[1];
    }
    return (
        <a href="#" className="list-group-item item-component" onClick={props.onClick}> 
            <span className="label label-default label-pill pull-xs-right">{formatTime}</span>
            <span className="item-title">{props.item.title}</span>
        </a>
    );
}

ListItem.propTypes = propTypes;

export default ListItem;